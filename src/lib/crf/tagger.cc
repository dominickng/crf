#include "base.h"

#include "config.h"
#include "fastmath.h"
#include "hashtable/size.h"
#include "io.h"
#include "lexicon.h"
#include "prob.h"
#include "tagset.h"
#include "vector.h"
#include "crf/nodepool.h"
#include "crf/lattice.h"
#include "crf/state.h"
#include "crf/features.h"
#include "crf/tagger.h"

template <typename T>
inline bool isinf(T value) {
  return std::numeric_limits<T>::has_infinity && value == std::numeric_limits<T>::infinity();
}

namespace NLP { namespace CRF {

Tagger::Tagger(Tagger::Config &cfg, const std::string &preface, Impl *impl)
  : _impl(impl) { }

Tagger::Tagger(const Tagger &other)
  : _impl(share(other._impl)) { }

lbfgsfloatval_t Tagger::Impl::duration_s(void) {
  return (clock() - clock_begin) / (lbfgsfloatval_t) CLOCKS_PER_SEC;
}

lbfgsfloatval_t Tagger::Impl::duration_m(void) {
  return (clock() - clock_begin) / (60.0 * CLOCKS_PER_SEC);
}

void Tagger::Impl::reset(const size_t size) {
  std::fill(scale.begin(), scale.begin() + size, 1.0);

  for (size_t i = 0; i < ntags; ++i)
    std::fill(trans_marginals[i].begin(), trans_marginals[i].end(), 0.0);

  for (size_t i = 0; i < size; ++i) {
    std::fill(alphas[i].begin(), alphas[i].end(), 0.0);
    std::fill(betas[i].begin(), betas[i].end(), 0.0);
    std::fill(state_marginals[i].begin(), state_marginals[i].end(), 0.0);
    for (size_t j = 0; j < psis[i].size(); ++j)
      std::fill(psis[i][j].begin(), psis[i][j].end(), 0.0);
  }
}

/**
 * compute_psis.
 * Iterate through the features attached to a context, and add the lambdas
 * for each feature to a probability distribution. Exponentiate the final
 * summed distributions, scaling by a decay factor for SGD. The distribution is
 * indexed by a tuple of (previous_tag, current_tag)
 *
 * State features (previous_Tag = None::val) are uniformly added to every
 * (x, current_tag) for each tag x
 */
void Tagger::Impl::compute_psis(Context &context, PDFs &dist, lbfgsfloatval_t decay) {
  //TODO profiling shows that this is the bottleneck in training
  //(50% of training time!)
  FeaturePtrs &trans_features = attributes.trans_features();

  for (size_t j = 0; j != context.features.size(); ++j) {
    Feature &f = *context.features[j];
    dist[f.klasses.prev][f.klasses.curr] += *(f.lambda);
    if (f.klasses.prev == None::val)
      for (Tag prev = 1; prev < ntags; ++prev)
        dist[prev][f.klasses.curr] += *(f.lambda);
  }

  if (context.index > 0) {
    for (size_t j = 0; j != trans_features.size(); ++j) {
      Feature &f = *trans_features[j];
      dist[f.klasses.prev][f.klasses.curr] += *(f.lambda);
    }
  }

  for (Tag prev = 0; prev < ntags; ++prev)
    for (Tag curr = 0; curr < ntags; ++curr) {
      if (dist[prev][curr] == 0)
        dist[prev][curr] = 1;
      else
#ifdef FASTEXP
        dist[prev][curr] = fastexp(dist[prev][curr] * decay);
#else
        dist[prev][curr] = exp(dist[prev][curr] * decay);
#endif
    }
}

/**
 * compute_psis.
 * Iterate through contexts, computing the activation values for each one.
 */
void Tagger::Impl::compute_psis(Contexts &contexts, PSIs &psis, lbfgsfloatval_t decay) {
  for (size_t i = 0; i < contexts.size(); ++i)
    compute_psis(contexts[i], psis[i], decay);
}

/**
 * compute_expectations.
 * Iterate through contexts, computing the expected values of each feature
 *
 * The expected value of a state feature is the sum over each occurence of the
 * feature of alpha[i][tag] * beta[i][tag] * (1.0 / scale[i]), where i is the
 * current position and tag is the current gold tag
 *
 * The expected value of a transition feature is the sum over each occurence of
 * the feature of alpha[i-1][prev] * activation[i][prev][curr] * beta[i][curr],
 * where i is the current position, prev is the previous gold tag, and curr
 * is the current gold tag
 */
void Tagger::Impl::compute_expectations(Contexts &c) {
  FeaturePtrs &trans_features = attributes.trans_features();

  for (size_t i = 0; i < c.size(); ++i) {
    lbfgsfloatval_t inv_scale = (1.0 / scale[i]);
    for (size_t j = 0; j < c[i].features.size(); ++j) {
      Feature &f = *(c[i].features[j]);
      TagPair &klasses = f.klasses;
      if (klasses.prev == None::val) { //state feature
        lbfgsfloatval_t alpha = alphas[i][klasses.curr];
        lbfgsfloatval_t beta = betas[i][klasses.curr];
        f.exp += alpha * beta * inv_scale;
      }
      else {
        //trans feature
        //FIXME TODO WARNING for some reason, trans features that look
        //further than 1 word back don't work
        lbfgsfloatval_t alpha = (i > 0) ? alphas[i-1][klasses.prev] : 1.0;
        lbfgsfloatval_t beta = betas[i][klasses.curr];
        f.exp += alpha * psis[i][klasses.prev][klasses.curr] * beta;
      }
    }

    if (i > 0) {
      for (size_t j = 0; j < trans_features.size(); ++j) {
        Feature &f = *trans_features[j];
        TagPair &klasses = f.klasses;
        lbfgsfloatval_t alpha = alphas[i-1][klasses.prev];
        lbfgsfloatval_t beta = betas[i][klasses.curr];
        f.exp += alpha * psis[i][klasses.prev][klasses.curr] * beta;
      }
    }
  }
}

/**
 * forward.
 * The forward pass of the forward-backward algorithm. This pass calculates the
 * alpha scores, using scaling to avoid numerical overflow from exponentiation
 *
 * The scaling factor scale[i] at each position i is the inverse sum of each
 * alpha[i]. It can be shown from the recurrence relations for alpha, alpha',
 * beta, beta', and the definition of the scale factor that the log partition
 * function (aka the log of the normalizing constant Z) can be calculated as
 * the negative sum of the log of each scale value.
 *
 * At position 0, alpha[0][tag] is calculated for each tag. Then scale[0]
 * is calculated, and alpha'[0][tag] is calculated by multiplying each
 * alpha[0][tag] by scale[0]. Then the alpha[i-1] are used to calculate each
 * alpha[i] and scale[i] in turn.
 *
 * alpha[0][tag] = state_activation(tag)
 * scale[0] = 1.0 / (sum (over tags t) [alpha[0][t]])
 * alpha'[0][tag] alpha[0][tag] * scale[0]
 *
 * alpha[i][tag] = sum (over prev tags p) [alpha'[i-1][p] * activation[i][tag]]
 * scale[i] = 1.0 / (sum (over tags t) [alpha[i][t]])
 * alpha'[i][tag] = alpha[i][tag] * scale[i]
 *
 */
void Tagger::Impl::forward(Contexts &contexts, PDFs &alphas, PSIs &psis, PDF &scale) {
  lbfgsfloatval_t sum = 0.0;

  for (Tag curr(2); curr < ntags; ++curr) {
    lbfgsfloatval_t val = psis[0][Sentinel::val][curr];
    alphas[0][curr] = val;
    sum += val;
    //std::cout << tags.str(curr) << ' ' << val << ' ' << sum << std::endl;
  }
  if (sum == 0.0)
    sum = 1.0;
  scale[0] = 1.0 / sum;
  vector_scale(alphas[0], scale[0], ntags);
  //vector_print(alphas[0], ntags);
  //std::cout << "sum: " << sum << std::endl;
  //std::cout << "scale: " << scale[0] << std::endl;

  for (size_t i = 1; i < contexts.size(); ++i) {
    sum = 0.0;
    for (Tag curr(2); curr < ntags; ++curr) {
      for (Tag prev(2); prev < ntags; ++prev) {
        lbfgsfloatval_t val = alphas[i-1][prev] * psis[i][prev][curr];
        //std::cout << tags.str(prev) << ' ' << tags.str(curr) << ' ' << alphas[i-1][prev] << ' ' << psis[i][prev][curr] << ' ' << val << std::endl;
        alphas[i][curr] += val;
        sum += val;
      }
    }
    if (sum == 0.0)
      sum = 1.0;
    scale[i] = 1.0 / sum;
    //std::cout << "sum: " << sum << std::endl;
    //std::cout << "scale: " << scale[i] << std::endl;
    //vector_print(alphas[i], ntags);
    vector_scale(alphas[i], scale[i], ntags);
  }
  log_z += -vector_sum_log(scale, contexts.size());
  //std::cout << "scale Z: " << -vector_sum_log(scale, contexts.size()) << std::endl;
}

/**
 * forward_noscale.
 * A version of the forward pass that does not perform scaling. In this case,
 * the log partition function is calculated by taking the log of the sum of
 * the alpha values in the final column (i.e. at position N, where N is the
 * number of words in the sentence)
 */
void Tagger::Impl::forward_noscale(Contexts &contexts, PDFs &alphas, PSIs &psis) {
  for (Tag curr(2); curr < ntags; ++curr) {
    lbfgsfloatval_t val = psis[0][Sentinel::val][curr];
    alphas[0][curr] = val;
  }

  for (size_t i = 1; i < contexts.size(); ++i) {
    for (Tag curr(2); curr < ntags; ++curr) {
      for (Tag prev(2); prev < ntags; ++prev) {
        lbfgsfloatval_t val = alphas[i-1][prev] * psis[i][prev][curr];
        //std::cout << tags.str(prev) << ' ' << tags.str(curr) << ' ' << alphas[i-1][prev] << ' ' << psis[i][prev][curr] << ' ' << val << std::endl;
        alphas[i][curr] += val;
      }
    }
  }
  //std::cout << "noscale Z: " << log(vector_sum(alphas[contexts.size() - 1], ntags)) << std::endl;
}

/**
 * backward.
 * The backward pass of the forward-backward algorithm. This pass calculates
 * the beta scores, using scaling to avoid numerical overflow from
 * exponentiation
 *
 * The scale values calculated during the forward pass are reused for scaling
 * the beta values. This is necessary for the correct calculation of the
 * log partition function, and will scale the beta values to reasonable
 * ranges.
 *
 * At position N, beta[N][tag] is initialised to 1.0 for each tag, and
 * beta'[N][tag] is calculated by multiplying each beta[N][tag] by scale[N]
 * (previously computed). Then each beta[i][tag] is calculated from
 * beta[i+1] and scale[i] in turn.
 *
 * beta[N][tag] = 1.0
 * beta'[N][tag] = beta[N][tag] * scale[N]
 *
 * beta[i][tag] = sum (over next tags t) [beta'[i+1][t] * activation[i+1][t]]
 * beta'[i][tag] = beta[i][tag] * scale[i]
 *
 */
void Tagger::Impl::backward(Contexts &contexts, PDFs &betas, PSIs &psis, PDF &scale) {
  //std::cout << "backward" << std::endl;
  for (Tag curr(2); curr < ntags; ++curr)
    betas[contexts.size() - 1][curr] = 1.0;
  vector_scale(betas[contexts.size() - 1], scale[contexts.size() - 1], ntags);

  for (int i = contexts.size() - 2; i >= 0; --i) {
    for (Tag curr(2); curr < ntags; ++curr) {
      for (Tag next(2); next < ntags; ++next) {
        betas[i][curr] += betas[i+1][next] * psis[i+1][curr][next];
        //std::cout << betas[i+1][next] << ' ' << psis[i+1][curr][next] << std::endl;
      }
    }
    vector_scale(betas[i], scale[i], ntags);
      //assert(!isinf(betas[i][curr]) && !std::isnan(betas[i][curr]));
  }
}

/**
 * backward_noscale.
 * A version of the backward pass that does not perform scaling.
 */
void Tagger::Impl::backward_noscale(Contexts &contexts, PDFs &betas, PSIs &psis) {
  for (Tag curr(2); curr < ntags; ++curr)
    betas[contexts.size() - 1][curr] = 1.0;

  for (int i = contexts.size() - 2; i >= 0; --i) {
    for (Tag curr(2); curr < ntags; ++curr) {
      for (Tag next(2); next < ntags; ++next) {
        betas[i][curr] += betas[i+1][next] * psis[i+1][curr][next];
      }
    }
  }
  lbfgsfloatval_t z = 0.0;
  for (Tag next(2); next < ntags; ++next)
    z += betas[0][next] * psis[0][Sentinel::val][next];
  //std::cout << "noscale Z: " << log(z) << std::endl;
}

/**
 * sum_llhood.
 * Iterates through a given vector of contexts and computes the sum of all
 * active features on each context. Returns the sum.
 */
lbfgsfloatval_t Tagger::Impl::sum_llhood(Contexts &contexts, lbfgsfloatval_t decay) {
  lbfgsfloatval_t score = 0.0;
  FeaturePtrs &trans_features = attributes.trans_features();

  for (Contexts::iterator i = contexts.begin(); i != contexts.end(); ++i) {
    for (FeaturePtrs::iterator j = i->features.begin(); j != i->features.end(); ++j)
      if ((*j)->klasses == i->klasses || ((*j)->klasses.prev == None::val && (*j)->klasses.curr == i->klasses.curr))
        score += *((*j)->lambda) * decay;

    for (FeaturePtrs::iterator j = trans_features.begin(); j != trans_features.end(); ++j)
      if ((*j)->klasses == i->klasses) {
        score += *((*j)->lambda) * decay;
        continue;
      }
  }
  return score;
}

/**
 * regularised_llhood.
 * Computes the regularised log likelihood, i.e. the objective function for
 * L-BFGS optimization.
 *
 * The three components of the regularised log likelihood are:
 *  1. the summed log likelihood over each training instance
 *  2. the summed log partition function over each training instance
 *  3. the sum of squared lambdas over all features divided by (2 * sigma^2)
 *
 * libLBFGS minimizes a given function, thus this returns the negative
 * regularised log likelihood.
 */
lbfgsfloatval_t Tagger::Impl::regularised_llhood(void) {
  lbfgsfloatval_t llhood = 0.0;
  for (Instances::iterator i = instances.begin(); i != instances.end(); ++i)
    llhood += sum_llhood(*i);
  //std::cout << llhood << ' ' << log_z << ' ' << (attributes.sum_lambda_sq() * inv_sigma_sq * 0.5) << std::endl;
  return -(llhood - log_z - (attributes.sum_lambda_sq() * inv_sigma_sq * 0.5));
}

/**
 * _lbfgs_evaluate.
 * Gradient and objective evaluation function for libLBFGS optimization.
 * Updates the gradient for each feature lambda, and computes the new value
 * of the objective function (regularised_llhood)
 *
 * The update process is:
 *  1. reset previously computed feature expectations and log_z to 0
 *  2. for each training instance:
 *        a. reset the working vectors (alphas, betas, etc.)
 *        b. compute the activation scores (psis) for each position in the
 *           instance
 *        c. perform the forward-backward algorithm to compute marginals
 *        d. increment the feature expectations based on the instance
 *  3. calculate the gradient of each feature, and copy to the grad vector
 *  4. calculate the regularised log likelihood and return it
 */
lbfgsfloatval_t Tagger::Impl::_lbfgs_evaluate(const lbfgsfloatval_t *x,
    lbfgsfloatval_t *g, const int n, const lbfgsfloatval_t step) {
  attributes.reset_expectations();
  //vector_print(x, n);

  log_z = 0.0;
  for (Instances::iterator i = instances.begin(); i != instances.end(); ++i) {
    Contexts &contexts = *i;
    reset(i->size());
    compute_psis(contexts, psis);

    //print_psis(contexts, psis);
    //forward_noscale(contexts, alphas, psis);
    //backward_noscale(contexts, betas, psis);
    //for (size_t j = 0; j < alphas.size(); ++j) {
      //std::fill(alphas[j].begin(), alphas[j].end(), 0.0);
      //std::fill(betas[j].begin(), betas[j].end(), 0.0);
    //}

    forward(contexts, alphas, psis, scale);
    backward(contexts, betas, psis, scale);
    //print_fwd_bwd(contexts, alphas, scale);
    //print_fwd_bwd(contexts, betas, scale);

    compute_expectations(contexts);
  }
  //attributes.prep_finite_differences();
  //finite_differences(g, false);

  attributes.copy_gradients(g, inv_sigma_sq);
  //attributes.print(inv_sigma_sq);

  return regularised_llhood();
}

/**
 * print_psis.
 * Debugging function to print out the matrix of activation values (psis)
 * for a given instance.
 */
void Tagger::Impl::print_psis(Contexts &contexts, PSIs &psis) {
  for (size_t i = 0; i < contexts.size(); ++i) {
    std::cout << "Position " << i << std::endl;
    for (size_t j = 0; j < psis[i].size(); ++j) {
      if(j == 0){
        std::cout << std::setw(16) << ' ';
        for (size_t k = 0; k < psis[i][j].size(); ++k)
          std::cout << std::setw(16) << tags.str(k);
        std::cout << std::endl;
      }
      std::cout << std::setw(16) << tags.str(j);
      for (size_t k = 0; k < psis[i][j].size(); ++k){
        std::cout << std::setw(16) << psis[i][j][k] << ' ';
      }
      std::cout << std::endl;
    }
    std::cout << '\n' << std::endl;
  }
}

/**
 * print_fwd_bwd.
 * Debugging function to print out the matrix of alpha values or matrix of
 * beta values as well as the accompanying scale factors.
 */
void Tagger::Impl::print_fwd_bwd(Contexts &contexts, PDFs &pdfs, PDF &scale) {
  std::cout << std::setw(16) << ' ';
  for (size_t i = 0; i < contexts.size(); ++i)
    std::cout << std::setw(16) << tags.str(contexts[i].klasses.curr);
  std::cout << std::endl;
  for(Tag curr(0); curr < ntags; ++curr) {
    std::cout << std::setw(16) << tags.str(curr);
    for(size_t i = 0; i < contexts.size(); ++i)
      std::cout << std::setw(16) << pdfs[i][curr];
    std::cout << std::endl;
  }
  std::cout << std::setw(16) << "scale:";
  for(size_t i = 0; i < contexts.size(); ++i)
    std::cout << std::setw(16) << scale[i];

  std::cout << '\n' << std::endl;
}

/**
 * finite_differences.
 * Debugging function to empirically check the gradient of each feature.
 * Optionally uses the empirically calculated gradient in the L-BFGS update
 * if the overwrite parameter is true (default=false).
 *
 * Uses a first order finite difference approximation, i.e. the derivative of
 * a function is approximately equal to the difference between the function
 * value at (x+h) and at (x), divided by h. h is a small constant (here 1e-4):
 *
 * f'(x) ~= (f(x+h) - f(x)) / h
 *
 * In this instance, the function is the regularised log likelihood. The
 * empirical value of the gradient should be almost identical to the
 * correct value, i.e. on the order of 1% or less difference between the two.
 *
 * This function iterates through each feature lambda. For each one, it
 * increments the lambda value by h, and then recomputes the activation values
 * and log partition function given the increment, which requires a full pass
 * through all the training instances. Then the regularised
 * log likelihood is recomputed and the finite differences check performed.
 *
 * IT IS EXTREMELY IMPORTANT THAT THE LOG PARTITION FUNCTION AND ACTIVATION
 * VALUES ARE RECOMPUTED FOR EACH FEATURE. OTHERWISE, THE EMPIRICAL GRADIENT
 * CALCULATED WILL BE INCORRECT
 */
void Tagger::Impl::finite_differences(lbfgsfloatval_t *g, bool overwrite) {
  lbfgsfloatval_t old_log_z = log_z; //store to restore later
  lbfgsfloatval_t EPSILON = 1.0e-4;
  int index = 0;
  lbfgsfloatval_t llhood = regularised_llhood();

  while (attributes.inc_next_lambda(EPSILON)) {
    log_z = 0.0;
    // re-estimate log Z
    for (Instances::iterator i = instances.begin(); i != instances.end(); ++i) {
      Contexts &contexts = *i;
      reset(i->size());
      compute_psis(contexts, psis);
      forward(contexts, alphas, psis, scale);
    }

    lbfgsfloatval_t plus_llhood = regularised_llhood();
    lbfgsfloatval_t val = (plus_llhood - llhood) / EPSILON;
    if (overwrite)
      g[index++] = val;
    else
      attributes.print_current_gradient(val, inv_sigma_sq);
  }

  log_z = old_log_z;
}

/**
 * calibrate.
 * Calibrates the learning rate for stochastic gradient descent optimization.
 *
 * The calibration process first computes an initial loss based on max_samples
 * number of training instances. This initial loss uses uniform weights of 0,
 * so it is essentially just the summed log partition factor over each of
 * the training instances. Then, we consider up to ncandidates number of
 * candidates for the learning rate eta, with each new candidate being the
 * previous candidate divided by calibration_rate. We reset the search up to
 * once if we run out of candidates or the loss explodes, setting eta to half
 * its original value and looking again through ncandidates potential learning
 * rates.
 *
 * For each candidate learning rate, we perform one epoch of stochastic
 * gradient descent using that learning rate. We check the loss from that
 * epoch, choosing the learning rate that results in the lowest possible
 * loss.
 */
lbfgsfloatval_t Tagger::Impl::calibrate(InstancePtrs &instance_ptrs,
    lbfgsfloatval_t *weights, lbfgsfloatval_t lambda,
    lbfgsfloatval_t initial_eta, const int nfeatures) {
  size_t max_samples = fmin(1000, instances.size());
  const int max_trials = 20;
  int ntrials = 1;
  const int max_candidates = 10;
  int ncandidates = max_candidates;
  const lbfgsfloatval_t calibration_rate = 2.0;
  lbfgsfloatval_t loss, best_eta = initial_eta, eta = initial_eta;
  lbfgsfloatval_t initial_loss = 0.0;
  lbfgsfloatval_t best_loss = std::numeric_limits<lbfgsfloatval_t>::max();
  bool dec = false;

  std::random_shuffle(instance_ptrs.begin(), instance_ptrs.end());
  for (int i = 0; i < nfeatures; ++i)
    weights[i] = 0.0;

  for (size_t i = 0; i < max_samples; ++i)
    initial_loss += score(*(instance_ptrs[i]));

  initial_loss += (attributes.sum_lambda_sq() * inv_sigma_sq * 0.5);
  logger << "Initial loss: " << initial_loss << std::endl;

  while (ncandidates > 0 || !dec) {
    logger << "Trial " << ntrials << ", eta = " << eta << std::endl;
    loss = sgd_iterate_calibrate(instance_ptrs, weights, nfeatures, max_samples, 1.0 / (lambda * eta), lambda);

    bool check = !isinf(loss) && !std::isnan(loss) && loss < initial_loss;
    if (check) {
      --ncandidates;
      logger << "Loss: " << loss << std::endl;
    }
    else
      logger << "Loss: " << loss << " (worse)" << std::endl;

    if (!isinf(loss) && loss < best_loss) {
      best_loss = loss;
      best_eta = eta;
    }

    if (dec)
      eta /= calibration_rate;
    else {
      if (check && ncandidates >= 0)
        eta *= calibration_rate;
      else {
        dec = true;
        ncandidates = max_candidates;
        eta = initial_eta / calibration_rate;
      }
    }

    if (++ntrials >= max_trials)
      break;
  }

  eta = best_eta;
  logger << "Best learning rate: " << eta << std::endl;
  return 1.0 / (lambda * eta);
}

/**
 * sgd_epoch
 * Performs one epoch of stochastic gradient descent.
 *
 * Each epoch iterates through the first nsamples of the training data and
 * adjusts the learning parameters for each one based on the number of updates
 * so far. For each instance, the loss on the instance with the current
 * lambdas is calculated, and the lambdas are updated based on the
 * instance. Once nsamples have been considered, the weights are scaled by the
 * decay factor and the loss incremented by the L2 norm of the weights
 */
lbfgsfloatval_t Tagger::Impl::sgd_epoch(InstancePtrs &instance_ptrs,
    lbfgsfloatval_t *weights, const int nfeatures, const int nsamples,
    const lbfgsfloatval_t lambda, const int t0, int &t, const bool log) {
  lbfgsfloatval_t eta, norm, gain, decay = 1.0;
  lbfgsfloatval_t loss = 0.0;

  for (int i = 0; i < nsamples; ++i) {
    Contexts &contexts = *(instance_ptrs[i]);
    eta = 1 / (lambda * (t0 + t));
    decay *= (1.0 - eta * lambda);
    gain = eta / decay;
    loss += score_instance(contexts, decay, gain);
    ++t;
  }

  if (isinf(loss))
    std::cout << "oh dear" << std::endl;

  vector_scale(weights, decay, nfeatures);
  norm = attributes.sum_lambda_sq() * inv_sigma_sq * 0.5;
  loss += norm;

  if (log) {
    logger << "  Loss = " << loss << std::endl;
    logger << "  Feature L2 norm = " <<  sqrt(norm) << std::endl;
    logger << "  Learning rate (eta) = " <<  eta << std::endl;
    logger << "  Total feature updates = " << t << std::endl;
  }

  return loss;
}

/**
 * sgd_iterate_calibrate.
 * Performs one epoch of stochastic gradient descent. Used in the process
 * of calibrating the learning rate.
 */
lbfgsfloatval_t Tagger::Impl::sgd_iterate_calibrate(InstancePtrs &instance_ptrs,
    lbfgsfloatval_t *weights, const int nfeatures, const int nsamples,
    const lbfgsfloatval_t t0, const lbfgsfloatval_t lambda) {
  int t = 0;
  for (int i = 0; i < nfeatures; ++i)
    weights[i] = 0.0;

  return sgd_epoch(instance_ptrs, weights, nfeatures, nsamples, lambda, t0, t);
}

/**
 * sgd_iterate.
 * Performs up to nepochs iterations of stochastic gradient descent.
 *
 * Each epoch randomly shuffles the training data, and then iterates through
 * each training instance, performing the SGD update.
 *
 * The optimization terminates after nepochs, or if the percentage
 * improvement in the summed loss over all the training instance falls
 * below cfg.delta()
 */
lbfgsfloatval_t Tagger::Impl::sgd_iterate(InstancePtrs &instance_ptrs,
    lbfgsfloatval_t *weights, const int nfeatures, const int nsamples,
    const lbfgsfloatval_t t0, const lbfgsfloatval_t lambda, const int nepochs,
    const int period) {
  lbfgsfloatval_t loss = 0.0;
  lbfgsfloatval_t improvement = 0.0;
  lbfgsfloatval_t best_loss = std::numeric_limits<lbfgsfloatval_t>::max();
  lbfgsfloatval_t *best_weights = new lbfgsfloatval_t[nfeatures];
  lbfgsfloatval_t *previous = new lbfgsfloatval_t[period];
  int t = 0;

  for (int i = 0; i < nfeatures; ++i)
    weights[i] = 0.0;

  for (int epoch = 1; epoch <= nepochs; ++epoch) {
    clock_begin = clock();
    logger << "Epoch " << epoch << std::endl;
    std::random_shuffle(instance_ptrs.begin(), instance_ptrs.end());

    loss = sgd_epoch(instance_ptrs, weights, nfeatures, nsamples, lambda, t0, t, true);

    if (loss < best_loss) {
      best_loss = loss;
      memcpy(best_weights, weights, sizeof(lbfgsfloatval_t) * nfeatures);
    }

    if (period < epoch)
      improvement = (previous[(epoch-1) % period] - loss) / loss;
    else
      improvement = cfg.delta();

    previous[(epoch-1) % period] = loss;
    if (period < epoch)
      logger << "  Improvement ratio = " << improvement << std::endl;
    logger << "  Epoch time = " << duration_s() << "s\n" << std::endl;

    if (improvement < cfg.delta())
      break;
  }

  if (best_weights)
    memcpy(best_weights, weights, sizeof(lbfgsfloatval_t) * nfeatures);

  delete [] best_weights;
  delete [] previous;

  return loss;
}

/**
 * compute_marginals.
 * Computes the marginal probabilities based on the model expectations for
 * each tag at each position i in the given contexts.
 *
 * The model expectation of a state having tag t at position i is given by:
 *   p(t, i) = alpha[i][t] * beta[i][t] / Z
 *           = alpha'[i][t] * beta'[i][t] * (1.0 / scale[i])
 *
 * The model expectation of a transition from tag t to tag u at position
 * (i, i+1) is given by:
 *   p(t, u, i, i+1) = alpha[i-1][t] * psis[i][t][u] * beta[i][u] / Z
 *                   = alpha'[i-1][t] * psis[i][t][u] * beta'[i][u]
 *
 * The model expecation of a transition from tag t to tag u is the sum of
 * p(t, u, i, i+1) over each position i in a sentence.
 *
 * Fortuitously, the design of the scaling factor allows the Z term to cancel
 * out of the transition expectations.
 *
 */
void Tagger::Impl::compute_marginals(Contexts &c, lbfgsfloatval_t decay) {
  for (size_t i = 0; i < c.size(); ++i) {
    lbfgsfloatval_t inv_scale = (1.0 / scale[i]);
    for (Tag curr = 2; curr < ntags; ++curr) {
      lbfgsfloatval_t alpha = alphas[i][curr];
      lbfgsfloatval_t beta = betas[i][curr];
      state_marginals[i][curr] += alpha * beta * inv_scale;
    }
    if (i > 0) {
      for (Tag prev = 2; prev < ntags; ++prev) {
        for (Tag curr = 2; curr < ntags; ++curr) {
          lbfgsfloatval_t alpha = alphas[i-1][prev];
          lbfgsfloatval_t beta = betas[i][curr];
          trans_marginals[prev][curr] += alpha * psis[i][prev][curr] * beta;
        }
      }
    }
  }
}

/**
 * compute_weights.
 * Updates the feature lambdas for features active on a training instance.
 * Used for stochastic gradient descent optimization.
 */
void Tagger::Impl::compute_weights(Contexts &c, lbfgsfloatval_t gain) {
  FeaturePtrs &trans_features = attributes.trans_features();

  for (size_t i = 0; i < c.size(); ++i) {
    for (size_t j = 0; j < c[i].features.size(); ++j) {
      Feature &f = *(c[i].features[j]);
      TagPair &klasses = f.klasses;
      if (klasses.prev == None::val) {
        if (klasses.curr == c[i].klasses.curr)
          *f.lambda += gain;
        *f.lambda -= state_marginals[i][klasses.curr] * gain;
      }
      else if (klasses == c[i].klasses)
        *f.lambda += gain;
    }

    for (size_t j = 0; j < trans_features.size(); ++j) {
      Feature &f = *trans_features[j];
      TagPair &klasses = f.klasses;
      if (klasses == c[i].klasses)
        *f.lambda += gain;
    }
  }

  for (size_t j = 0; j < trans_features.size(); ++j) {
    Feature &f = *trans_features[j];
    TagPair &klasses = f.klasses;
    *f.lambda -= trans_marginals[klasses.prev][klasses.curr] * gain;
  }
}

/**
 * score.
 * Computes the unregularised log likelihood of a given training instance.
 * Used to compute the initial loss in the calibration process for stochastic
 * gradient descent optimization.
 */
lbfgsfloatval_t Tagger::Impl::score(Contexts &contexts, lbfgsfloatval_t decay) {
  lbfgsfloatval_t score = 0.0;
  log_z = 0.0;
  reset(contexts.size());
  compute_psis(contexts, psis, decay);
  forward(contexts, alphas, psis, scale);
  //backward(contexts, betas, psis, scale);
  score -= (sum_llhood(contexts, decay) - log_z);
  return score;
}

/**
 * score_instance.
 * Updates the weights for features active on a particular training instance,
 * and computes the unregularized loss for that instance. Used in stochastic
 * gradient descent optimization.
 */
lbfgsfloatval_t Tagger::Impl::score_instance(Contexts &contexts, lbfgsfloatval_t decay, lbfgsfloatval_t gain) {
  lbfgsfloatval_t score;
  log_z = 0.0;
  reset(contexts.size());
  score = (sum_llhood(contexts, decay));
  compute_psis(contexts, psis, decay);
  forward(contexts, alphas, psis, scale);
  backward(contexts, betas, psis, scale);
  compute_marginals(contexts, decay);
  compute_weights(contexts, gain);
  //std::cout << -score << ' ' << log_z << ' ' << (attributes.sum_lambda_sq() * inv_sigma_sq * 0.5) <<  std::endl;
  return -score + log_z;
}

/**
 * train_lbfgs.
 * Perform L-BFGS optimization given a labelled training dataset. Uses the
 * libLBFGS library for the optimization, which requires the calculation of
 * the function objective value (negative regularised log likelihood) and
 * the gradient of each feature lambda at each iteration.
 */
void Tagger::Impl::train_lbfgs(Reader &reader, lbfgsfloatval_t *weights) {
  logger << "beginning L-BFGS optimization" << std::endl;
  const size_t n = model.nfeatures();
  lbfgs_parameter_t param;

  for (size_t i = 0; i < n; ++i)
    weights[i] = 0.0;

  lbfgs_parameter_init(&param);
  param.max_iterations = cfg.niterations();
  param.linesearch = LBFGS_LINESEARCH_MORETHUENTE;
  param.epsilon = 1e-5;
  param.delta = 1e-5;
  param.past = 10;

  attributes.assign_lambdas(weights);
  clock_begin = clock();

  int ret = lbfgs(n, weights, NULL, lbfgs_evaluate, lbfgs_progress, (void *)this, &param);

  logger << "L-BFGS optimization terminated with status code " << ret << std::endl;
}

/**
 * train_sgd.
 * Perform stochastic gradient descent optimization given a labelled training
 * dataset.
 */
void Tagger::Impl::train_sgd(Reader &reader, lbfgsfloatval_t *weights) {
  logger << "beginning SGD optimization" << std::endl;
  const size_t n = model.nfeatures();
  lbfgsfloatval_t lambda = 1.0 / (instances.size() * cfg.sigma() * cfg.sigma());
  InstancePtrs instance_ptrs; // randomly shuffling pointers is faster

  for (Instances::iterator i = instances.begin(); i != instances.end(); ++i)
    instance_ptrs.push_back(&(*i));

  for (size_t i = 0; i < n; ++i)
    weights[i] = 0.0;

  attributes.assign_lambdas(weights);

  clock_begin = clock();
  lbfgsfloatval_t t0 = calibrate(instance_ptrs, weights, lambda, cfg.eta(), n);
  logger << "Calibration time: " << duration_s() << "s\n" << std::endl;

  clock_begin = clock();
  sgd_iterate(instance_ptrs, weights, n, instance_ptrs.size(), t0, lambda, cfg.niterations(), cfg.period());
}

/**
 * reg.
 * Registers active feature generating functions with their associated
 * feature type constants and feature dictionaries in the registry.
 *
 * The registry object is responsible for mapping feature type constants to
 * the appropriate feature generator and feature dictionary.
 *
 * Feature generators are responsible for generating feature values given a
 * sentence and a position. This is done in multiple passes: an initial pass
 * that extracts all active features, and a second pass which builds the
 * instances vector used in training.
 *
 * Feature dictionaries load the weights associated with each feature for use
 * in tagging. Each dictionary is a member of the Tagger (sub)class
 */
void Tagger::Impl::reg(void) {
  registry.reg(Types::w, new WordGen(w_dict, true, false), types.use_words());
  registry.reg(Types::pw, new OffsetWordGen(w_dict, -1, true, false), types.use_prev_words());
  registry.reg(Types::ppw, new OffsetWordGen(w_dict, -2, true, false), types.use_prev_words());
  registry.reg(Types::nw, new OffsetWordGen(w_dict, 1, true, false), types.use_next_words());
  registry.reg(Types::nnw, new OffsetWordGen(w_dict, 2, true, false), types.use_next_words());

  //registry.reg(Types::prefix, new PrefixGen(a_dict, true, false), true, types.use_prefix());
  //registry.reg(Types::suffix, new SuffixGen(a_dict, true, false), true, types.use_suffix());

  //registry.reg(Types::ppw_pw, new BigramWordGen(ww_dict, -2, true, true), types.use_word_bigrams());
  registry.reg(Types::pw_w, new BigramWordGen(ww_dict, -1, true, false), types.use_word_bigrams());
  registry.reg(Types::w_nw, new BigramWordGen(ww_dict, 0, true, false), types.use_word_bigrams());
  //registry.reg(Types::nw_nnw, new BigramWordGen(ww_dict, 1, true, true), types.use_word_bigrams());

  registry.reg(Types::trans, new TransGen(t_dict, false, true), types.use_trans());
}

/**
 * load.
 * Loads the lexicon and tag hash tables, registers the active features, and
 * loads the trained model. This function must be called before tagging
 * sentences.
 */
void Tagger::Impl::load(void) {
  lexicon.load();
  tags.load();
  reg();
  _load_model(model);
}

/**
 * _load_model.
 * Reads the model statistics, and loads the feature lambdas and attributes.
 */
void Tagger::Impl::_load_model(Model &model) {
  model.read_config();
  _read_weights(model);
  _read_attributes(model);
}

/**
 * _read_weights.
 * Loads the attribute-sorted feature lambdas into a reference vector of
 * weights.
 *
 * An attribute is some string generated from the input sequence (e.g. the
 * current word, the previous word, etc.) and a feature type.
 * A feature is a lambda value associated with a pair of tags and an
 * attribute.
 *
 * As each lambda is loaded, the attribute that it is associated with is
 * checked. The weights file is guaranteed to be sorted by attribute values,
 * so each time the attribute value changes, the next lambda corresponds to
 * the next attribute.
 *
 * At test time, each attribute is simply represented as a pair of pointers to
 * Weight objects in their reference vector; one to the start, and one to
 * one past the end.
 */
void Tagger::Impl::_read_weights(Model &model) {
  uint64_t prev_klass, curr_klass, freq, attrib = 0, nlines = 0;
  uint64_t previous = static_cast<uint64_t>(-1);
  lbfgsfloatval_t lambda;
  std::string preface;
  const std::string &filename = cfg.features();

  std::ifstream in(filename.c_str());
  if (!in)
    throw IOException("could not open file", filename);

  read_preface(filename, in, preface, nlines);
  weights.reserve(model.nfeatures());

  while (in >> attrib >> prev_klass >> curr_klass >> freq >> lambda) {
    ++nlines;
    weights.push_back(Weight(prev_klass, curr_klass, lambda));
    if (attrib != previous) {
      attribs2weights.push_back(&weights.back());
      previous = attrib;
    }
  }

  if (!in.eof())
    throw IOException("could not parse weight tuple", cfg.features(), nlines);
  if (weights.size() != model.nfeatures())
    throw IOException("number of weights read is not equal to configuration value", cfg.features(), nlines);
  attribs2weights.push_back(&weights.back() + 1);
  if (attribs2weights.size() != model.nattributes() + 1)
    throw IOException("number of attributes read is not equal to configuration value", cfg.features(), nlines);
}

/**
 * _read_attributes.
 * Loads the attributes extracted during training into the feature
 * dictionaries. This process maps an attribute to all the feature lambdas
 * associated with it in the feature dictionary for the appropriate feature
 * type.
 */
void Tagger::Impl::_read_attributes(Model &model) {
  uint64_t freq = 0, nlines = 0, id = 0;
  std::string preface, type;
  const std::string &filename = cfg.attributes();

  std::ifstream in(filename.c_str());
  if (!in)
    throw IOException("could not open file", filename);

  read_preface(filename, in, preface, nlines);

  while (in >> type) {
    ++nlines;
    if (id >= model.nattributes())
      throw IOException("attribute id >= nattributes", filename, nlines);
    Attribute &attrib = registry.load(type, in);

    if (!(in >> freq))
      throw IOException("could not read freq", filename, nlines);
    if (in.get() != '\n')
      throw IOException("expected newline after attribute freq", filename, nlines);

    attrib.begin = attribs2weights[id];
    attrib.end = attribs2weights[id + 1];
    ++id;
  }

  if (!in.eof())
    throw IOException("could not parse weight tuple", cfg.features(), nlines);
  if (id != model.nattributes())
    throw IOException("number of attributes read is not equal to configuration value", cfg.attributes(), nlines);
}

/**
 * extract.
 * Runs the feature extraction process by calling the pure virtual functions
 * _pass1, _pass2, and _pass3. These methods are to be implemented by the
 * Tagger::Impl subclasses.
 *
 * In general:
 *
 *  _pass1: extracts the word lexicon, the tags present in the
 *          training corpus, and any other provided data (e.g. POS tags).
 *          These will be saved to disk.
 *
 *  _pass2: extracts and counts all occurences of active features, and stores
 *          them in the attributes dictionary. The attributes dictionary maps
 *          features to the attributes that they occur with. The attributes
 *          are sorted by frequency and saved to disk.
 *
 *    any relevant cutoffs for eliminating rare attributes and features is
 *    applied
 *
 *  _pass3: constructs the instances vector used in training. For each
 *          training instance, build a vector of contexts, one for each word
 *          in the sentence. For each context, compute a feature vector that
 *          consists of pointers to the appropriate feature object in the
 *          attributes dictionary
 */
void Tagger::Impl::extract(Reader &reader, Instances &instances) {
  logger << "beginning pass 1" << std::endl;
  _pass1(reader);
  reader.reset();
  logger << "beginning pass 2" << std::endl;
  _pass2(reader);

  attributes.apply_cutoff(Types::w, cfg.cutoff_words(), cfg.cutoff_default());
  if (cfg.cutoff_attribs() > 1)
    attributes.apply_attrib_cutoff(cfg.cutoff_attribs());

  reader.reset();
  logger << "beginning pass 3" << std::endl;
  _pass3(reader, instances);
}

/**
 * train.
 * Given a labelled training dataset and a training algorithm, train a model
 * for CRF tagging.
 */
void Tagger::Impl::train(Reader &reader, const std::string &trainer) {
  clock_t begin = clock();
  clock_begin = begin;
  logger << "beginning feature extraction" << std::endl;
  reg();
  extract(reader, instances);
  logger << "completed feature extraction in " << duration_s() << "s\n" << std::endl;

  ntags = tags.size();
  inv_sigma_sq = 1.0 / (cfg.sigma() * cfg.sigma());

  // initialize the working vectors. each one is prepopulated to the size
  // of the longest sentence seen in the training data to avoid the overhead
  // of extra memory allocations during training
  for (size_t i = 0; i < ntags; ++i)
    trans_marginals.push_back(PDF(ntags, 0.0));

  for (size_t i = 0; i < model.max_size(); ++i) {
    alphas.push_back(PDF(ntags, 0.0));
    betas.push_back(PDF(ntags, 0.0));
    state_marginals.push_back(PDF(ntags, 0.0));
    scale.push_back(1.0);
    psis.push_back(PDFs(0));
    for (size_t j = 0; j < ntags; ++j)
      psis[i].push_back(PDF(ntags, 0.0));
  }

  model.nattributes(attributes.size());
  model.nfeatures(attributes.nfeatures());
  lbfgsfloatval_t *weights = new lbfgsfloatval_t[model.nfeatures()];

  if (trainer == "lbfgs")
    train_lbfgs(reader, weights);
  else if (trainer == "sgd")
    train_sgd(reader, weights);
  else
    throw ValueException("Unknown training algorithm", trainer);

  model.save(preface);
  attributes.save_features(cfg.features(), preface);
  attributes.zero_lambdas();
  delete [] weights;

  logger << "Total training time: " << (clock() - begin) / (60.0 * CLOCKS_PER_SEC) << " minutes" << std::endl;
}

/**
 * lbfgs_evaluate.
 * Static function required for libLBFGS. Casts the instance parameter to
 * its true type of a Tagger::Impl pointer, and then calls the
 * _lbfgs_evaluate method on it.
 */
lbfgsfloatval_t Tagger::Impl::lbfgs_evaluate(void *_instance,
    const lbfgsfloatval_t *x, lbfgsfloatval_t *g, const int n,
    const lbfgsfloatval_t step) {
  return reinterpret_cast<Tagger::Impl *>(_instance)->_lbfgs_evaluate(x, g, n, step);
}

/**
 * lbfgs_progress.
 * Static function required for libLBFGS. Prints out the progress of training
 * at each iteration
 *
 * instance: pointer to a Tagger::Impl subclass
 * x: array of floatvals of size n; holds the current lambda values
 * g: array of floatvals of size n; holds the current gradients
 * fx: current negative log-likelihood value
 * xnorm: the Euclidean norm of the current lambdas
 * gnorm: the Euclidean norm of the current gradients
 * step: the size of the line search step used in this iteration
 * n: the number of feature lambdas
 * k: the iteration count
 * ls: the number of line search evaluations called in this iteration
 *
 */
int Tagger::Impl::lbfgs_progress(void *instance, const lbfgsfloatval_t *x,
    const lbfgsfloatval_t *g, const lbfgsfloatval_t fx,
    const lbfgsfloatval_t xnorm, const lbfgsfloatval_t gnorm,
    const lbfgsfloatval_t step, int n, int k, int ls) {

  uint64_t nactives = 0;
  for (int i = 0; i < n; ++i)
    if (x[i] != 0.0)
      ++nactives;

  Tagger::Impl *impl = reinterpret_cast<Tagger::Impl *>(instance);
  Log &logger = impl->logger;
  logger << "Iteration " << k << '\n' << "  llhood = " << fx;
  logger << ", xnorm = " << xnorm << ", gnorm = " << gnorm;
  logger << ", step = " << step << ", trials = " << ls;
  logger << ", nactives = " << nactives << '/' << n;
  logger << ", time = " << impl->duration_s() << "s" << std::endl;

  impl->clock_begin = clock();
  return 0;
}

} }
