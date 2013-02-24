#include "base.h"

#include "config.h"
#include "io.h"
#include "lbfgs.h"
#include "hashtable/size.h"
#include "shared.h"
#include "lexicon.h"
#include "tagset.h"
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

void Tagger::Impl::compute_psis(Context &context, PDFs &dist, double decay) {
  //TODO profiling shows that this is the bottleneck in training
  //(50% of training time!)
  for (size_t j = 0; j != context.features.size(); ++j) {
    Feature &f = *context.features[j];
    dist[f.klasses.prev][f.klasses.curr] += *(f.lambda);
    if (f.klasses.prev == None::val)
      for (Tag prev = 1; prev < ntags; ++prev)
        dist[prev][f.klasses.curr] += *(f.lambda);
  }

  for (Tag prev = 0; prev < ntags; ++prev)
    for (Tag curr = 0; curr < ntags; ++curr)
      dist[prev][curr] = exp(dist[prev][curr] * decay);
}

void Tagger::Impl::compute_psis(Contexts &contexts, PSIs &psis, double decay) {
  for (size_t i = 0; i < contexts.size(); ++i)
    compute_psis(contexts[i], psis[i], decay);
}

void Tagger::Impl::compute_expectations(Contexts &c) {
  for (size_t i = 0; i < c.size(); ++i) {
    double inv_scale = (1.0 / scale[i]);
    for (size_t j = 0; j < c[i].features.size(); ++j) {
      Feature &f = *(c[i].features[j]);
      TagPair &klasses = f.klasses;
      if (klasses.prev == None::val) { //state feature
        double alpha = alphas[i][klasses.curr];
        double beta = betas[i][klasses.curr];
        f.exp += alpha * beta * inv_scale;
      }
      else {
        //trans feature
        //FIXME TODO WARNING for some reason, trans features that look
        //further than 1 word back don't work
        double alpha = (i > 0) ? alphas[i-1][klasses.prev] : 1.0;
        double beta = betas[i][klasses.curr];
        f.exp += alpha * psis[i][klasses.prev][klasses.curr] * beta;
      }

      //std::cout << j << ' ' << tags.str(klasses.prev) << ' ' << tags.str(klasses.curr) << ' ' << alpha << ' ' << beta << ' ' << psis[j][klasses.prev][klasses.curr] << std::endl;
    }
  }
}

void Tagger::Impl::forward(Contexts &contexts, PDFs &alphas, PSIs &psis, PDF &scale) {
  double sum = 0.0;

  for (Tag curr(2); curr < ntags; ++curr) {
    double val = psis[0][Sentinel::val][curr];
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
        double val = alphas[i-1][prev] * psis[i][prev][curr];
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

void Tagger::Impl::forward_noscale(Contexts &contexts, PDFs &alphas, PSIs &psis) {
  for (Tag curr(2); curr < ntags; ++curr) {
    double val = psis[0][Sentinel::val][curr];
    alphas[0][curr] = val;
  }

  for (size_t i = 1; i < contexts.size(); ++i) {
    for (Tag curr(2); curr < ntags; ++curr) {
      for (Tag prev(2); prev < ntags; ++prev) {
        double val = alphas[i-1][prev] * psis[i][prev][curr];
        //std::cout << tags.str(prev) << ' ' << tags.str(curr) << ' ' << alphas[i-1][prev] << ' ' << psis[i][prev][curr] << ' ' << val << std::endl;
        alphas[i][curr] += val;
      }
    }
  }
  std::cout << "noscale Z: " << log(vector_sum(alphas[contexts.size() - 1], ntags)) << std::endl;
}


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
  double z = 0.0;
  for (Tag next(2); next < ntags; ++next)
    z += betas[0][next] * psis[0][Sentinel::val][next];
  std::cout << "noscale Z: " << log(z) << std::endl;
}

double Tagger::Impl::sum_llhood(Contexts &contexts, double decay) {
  double score = 0.0;
  for (Contexts::iterator i = contexts.begin(); i != contexts.end(); ++i) {
    for (FeaturePtrs::iterator j = i->features.begin(); j != i->features.end(); ++j)
      if ((*j)->klasses == i->klasses || ((*j)->klasses.prev == None::val && (*j)->klasses.curr == i->klasses.curr))
        score += *((*j)->lambda) * decay;
  }
  return score;
}

double Tagger::Impl::regularised_llhood(void) {
  double llhood = 0.0;
  for (Instances::iterator i = instances.begin(); i != instances.end(); ++i)
    llhood += sum_llhood(*i);
  //std::cout << llhood << ' ' << log_z << ' ' << (attributes.sum_lambda_sq() * inv_sigma_sq * 0.5) << std::endl;
  return -(llhood - log_z - (attributes.sum_lambda_sq() * inv_sigma_sq * 0.5));
}

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

void Tagger::Impl::finite_differences(lbfgsfloatval_t *g, bool overwrite) {
  double old_log_z = log_z; //store to restore later
  double EPSILON = 1.0e-4;
  int index = 0;
  double llhood = regularised_llhood();

  while (attributes.inc_next_lambda(EPSILON)) {
    log_z = 0.0;
    // re-estimate log Z
    for (Instances::iterator i = instances.begin(); i != instances.end(); ++i) {
      Contexts &contexts = *i;
      reset(i->size());
      compute_psis(contexts, psis);
      forward(contexts, alphas, psis, scale);
    }

    double plus_llhood = regularised_llhood();
    double val = (plus_llhood - llhood) / EPSILON;
    if (overwrite)
      g[index++] = val;
    else
      attributes.print_current_gradient(val, inv_sigma_sq);
  }

  log_z = old_log_z;
}

double Tagger::Impl::calibrate(InstancePtrs &instance_ptrs, double *weights, double lambda, double initial_eta, const size_t n) {
  size_t max_samples = fmin(1000, instances.size());
  size_t nsamples = max_samples;
  const int max_trials = 20;
  int ntrials = 1;
  const int max_candidates = 10;
  int ncandidates = max_candidates;
  const double calibration_rate = 2.0;
  double loss, best_eta = initial_eta, eta = initial_eta;
  double initial_loss = 0.0;
  double best_loss = std::numeric_limits<double>::max();
  bool dec = false;

  std::random_shuffle(instance_ptrs.begin(), instance_ptrs.end());

  for (size_t i = 0; i < n; ++i)
    weights[i] = 0.0;

  for (size_t i = 0; i < nsamples; ++i)
    initial_loss += score(*(instance_ptrs[i]));

  initial_loss += (attributes.sum_lambda_sq() * inv_sigma_sq * 0.5);
  std::cout << "Initial loss: " << initial_loss << std::endl;

  while (ncandidates > 0 || !dec) {
    std::cout << "Trial " << ntrials << ", eta = " << eta << std::endl;
    loss = sgd_iterate(instance_ptrs, weights, n, nsamples, 1.0 / (lambda * eta), lambda, 1, 1, true);

    bool check = !isinf(loss) && loss < initial_loss;
    if (check) {
      --ncandidates;
      std::cout << "Loss: " << loss << std::endl;
    }
    else
      std::cout << "Loss: " << loss << " (worse)" << std::endl;

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
  std::cout << "Best learning rate: " << eta << std::endl;
  return 1.0 / (lambda * eta);
}

double Tagger::Impl::sgd_iterate(InstancePtrs &instance_ptrs, double *weights,
    const int n, const int nsamples, const double t0, const double lambda,
    const int nepochs, const int period, bool calibration) {
  double *previous, gain, eta;
  double decay = 1;
  double loss = 0.0;
  double improvement = 0.0;
  double best_loss = std::numeric_limits<double>::max();
  int t = 0;
  double *best_weights = 0;

  if (!calibration) {
    best_weights = new double[n];
    previous = new double[period];
  }

  for (size_t i = 0; i < n; ++i)
    weights[i] = 0.0;

  for (size_t epoch = 1; epoch <= nepochs; ++epoch) {
    if (!calibration) {
      std::cout << "Epoch " << epoch << std::endl;
      std::random_shuffle(instance_ptrs.begin(), instance_ptrs.end());
    }

    loss = 0.0;
    for (int i = 0; i < nsamples; ++i) {
      Contexts &contexts = *(instance_ptrs[i]);
      eta = 1 / (lambda * (t0 + t));
      decay *= (1.0 - eta * lambda);
      gain = eta / decay;

      double l = score_instance(contexts, decay, gain);
      //std::cout << "  lambda: " << lambda << ", eta: " << eta <<", gain: " << gain << ", decay: " << decay << ", l: " << l << std::endl;
      //vector_print(weights, n);

      loss += l;
      ++t;
    }

    if (isinf(loss))
      std::cout << "oh dear" << std::endl;

    vector_scale(weights, decay, n);
    decay = 1.0;
    double norm = attributes.sum_lambda_sq() * inv_sigma_sq * 0.5;
    loss += norm;
    //vector_print(weights, n);
    //std::cout << norm << std::endl;

    if (!calibration) {
      if (loss < best_loss) {
        best_loss = loss;
        memcpy(best_weights, weights, sizeof(double) * n);
      }

      if (period < epoch)
        improvement = (previous[(epoch-1) % period] - loss) / loss;
      else
        improvement = cfg.delta();

      previous[(epoch-1) % period] = loss;
      std::cout << "  Loss: " << loss << std::endl;
      if (period < epoch)
        std::cout << "  Improvement ratio " << improvement << std::endl;
      std::cout << "  Feature L2 norm " <<  sqrt(norm) << std::endl;
      std::cout << "  Learning rate (eta) " <<  eta << std::endl;
      std::cout << "  Total feature updates " << t << std::endl;

      if (improvement < cfg.delta())
        break;
    }
  }

  if (best_weights)
    memcpy(best_weights, weights, sizeof(double) * n);

  if (!calibration) {
    delete [] best_weights;
    delete [] previous;
  }

  return loss;
}

void Tagger::Impl::compute_marginals(Contexts &c, double decay) {
  for (size_t i = 0; i < c.size(); ++i) {
    double inv_scale = (1.0 / scale[i]);
    //std::cout << "computing expectation for state " << klasses.curr << " at position " << i << std::endl;
    for (Tag curr = 2; curr < ntags; ++curr) {
      double alpha = alphas[i][curr];
      double beta = betas[i][curr];
      state_marginals[i][curr] += alpha * beta * inv_scale;
    }
    if (i > 0) {
      for (Tag prev = 2; prev < ntags; ++prev) {
        for (Tag curr = 2; curr < ntags; ++curr) {
          double alpha = alphas[i-1][prev];
          double beta = betas[i][curr];
          trans_marginals[prev][curr] += alpha * psis[i][prev][curr] * beta;
        }
      }
    }
  }
}

void Tagger::Impl::compute_weights(Contexts &c, double gain) {
  for (size_t i = 0; i < c.size(); ++i) {
    for (size_t j = 0; j < c[i].features.size(); ++j) {
      Feature &f = *(c[i].features[j]);
      TagPair &klasses = f.klasses;
      if (klasses.prev == None::val) {
        if (klasses.curr == c[i].klasses.curr) {
          *f.lambda += gain;
          //std::cout << "incrementing state lambda " << klasses.curr << " by " << gain << " at position " << i << " to " << *f.lambda << std::endl;
        }
        *f.lambda -= state_marginals[i][klasses.curr] * gain;
        //std::cout << "decrementing state lambda " << klasses.curr << " by " << gain * state_marginals[i][klasses.curr] << " at position " << i << " to " << *f.lambda << " (" << gain << " * " << state_marginals[i][klasses.curr] << ")" << std::endl;
        //std::cout << alpha << ' ' << beta << ' ' << inv_scale << std::endl;
        //std::cout << (*f.lambda) << std::endl;
      }
      else if (klasses == c[i].klasses) {
        *f.lambda += gain;
        //std::cout << "incrementing lambda by " << gain << std::endl;
      }
    }
  }

  Features& trans_features = attributes.trans_features();
  for (size_t j = 0; j < trans_features.size(); ++j) {
    Feature &f = trans_features[j];
    TagPair &klasses = f.klasses;
    *f.lambda -= trans_marginals[klasses.prev][klasses.curr] * gain;
    //std::cout << "decrementing lambda by " << gain * trans_marginals[klasses.prev][klasses.curr] << " (" << gain << " * " << trans_marginals[klasses.prev][klasses.curr] << ")" << std::endl;
  }
}

double Tagger::Impl::score(Contexts &contexts, double decay) {
  double score = 0.0;
  log_z = 0.0;
  reset(contexts.size());
  compute_psis(contexts, psis, decay);
  forward(contexts, alphas, psis, scale);
  //backward(contexts, betas, psis, scale);
  score -= (sum_llhood(contexts, decay) - log_z);
  return score;
}

double Tagger::Impl::score_instance(Contexts &contexts, double decay, double gain) {
  double score;
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

void Tagger::Impl::train_lbfgs(Reader &reader, double *weights) {
  const size_t n = attributes.nfeatures();
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

  int ret = lbfgs(n, weights, NULL, lbfgs_evaluate, lbfgs_progress, (void *)this, &param);

  std::cerr << "L-BFGS optimization terminated with status code " << ret << std::endl;
}

void Tagger::Impl::train_sgd(Reader &reader, double *weights) {
  const size_t n = attributes.nfeatures();
  double lambda = 1.0 / (instances.size() * cfg.sigma() * cfg.sigma());
  InstancePtrs instance_ptrs; // randomly shuffling pointers is faster

  for (Instances::iterator i = instances.begin(); i != instances.end(); ++i)
    instance_ptrs.push_back(&(*i));

  for (size_t i = 0; i < n; ++i)
    weights[i] = 0.0;

  attributes.assign_lambdas(weights);

  double t0 = calibrate(instance_ptrs, weights, lambda, cfg.eta(), n);
  sgd_iterate(instance_ptrs, weights, n, instance_ptrs.size(), t0, lambda, cfg.niterations(), cfg.period(), false);
}

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

void Tagger::Impl::load(void) {
  lexicon.load();
  tags.load();
  reg();
  _load_model(model);
}

void Tagger::Impl::_load_model(Model &model) {
  model.read_config();
  _read_weights(model);
  _read_attributes(model);
}

void Tagger::Impl::_read_weights(Model &model) {
  uint64_t prev_klass, curr_klass, freq, attrib = 0, nlines = 0;
  uint64_t previous = static_cast<uint64_t>(-1);
  double lambda;
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

void Tagger::Impl::extract(Reader &reader, Instances &instances) {
  std::cout << "beginning pass 1" << std::endl;
  _pass1(reader);
  reader.reset();
  std::cout << "beginning pass 2" << std::endl;
  _pass2(reader);

  attributes.apply_cutoff(Types::w, cfg.cutoff_words(), cfg.cutoff_default());
  if (cfg.cutoff_attribs() > 1)
    attributes.apply_attrib_cutoff(cfg.cutoff_attribs());

  reader.reset();
  std::cout << "beginning pass 3" << std::endl;
  _pass3(reader, instances);

}

void Tagger::Impl::train(Reader &reader, const std::string &trainer) {
  reg();
  inv_sigma_sq = 1.0 / (cfg.sigma() * cfg.sigma());
  extract(reader, instances);
  ntags = tags.size();

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

  double *weights = new double[attributes.nfeatures()];
  if (trainer == "lbfgs")
    train_lbfgs(reader, weights);
  else if (trainer == "sgd")
    train_sgd(reader, weights);
  else
    throw Exception("Unknown training algorithm");

  model.nattributes(attributes.size());
  model.nfeatures(attributes.nfeatures());
  model.save(preface);
  attributes.save_features(cfg.features(), preface);
  delete [] weights;
}

lbfgsfloatval_t Tagger::Impl::lbfgs_evaluate(void *_instance, const lbfgsfloatval_t *x,
    lbfgsfloatval_t *g, const int n, const lbfgsfloatval_t step) {
  return reinterpret_cast<Tagger::Impl *>(_instance)->_lbfgs_evaluate(x, g, n, step);
}

int Tagger::Impl::lbfgs_progress(void *instance, const lbfgsfloatval_t *x,
    const lbfgsfloatval_t *g, const lbfgsfloatval_t fx,
    const lbfgsfloatval_t xnorm, const lbfgsfloatval_t gnorm,
    const lbfgsfloatval_t step, int n, int k, int ls) {

  uint64_t nactives = 0;
  for (int i = 0; i < n; ++i)
    if (x[i] != 0.0)
      ++nactives;

  std::cout << "Iteration " << k << '\n' << "  llhood = " << fx;
  std::cout << ", xnorm = " << xnorm << ", gnorm = " << gnorm;
  std::cout << ", step = " << step << ", trials = " << ls;
  std::cout << ", nactives = " << nactives << '/' << n << std::endl;
  return 0;
}

} }
