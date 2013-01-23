#include "base.h"

#include "config.h"
#include "io.h"
#include "lbfgs.h"
#include "hashtable/size.h"
#include "shared.h"
#include "lexicon.h"
#include "tagset.h"
#include "crf/features.h"
#include "crf/tagger.h"

template <typename T>
inline bool isinf(T value) {
  return std::numeric_limits<T>::has_infinity && value == std::numeric_limits<T>::infinity();
}

namespace NLP { namespace CRF {

Tagger::Tagger(Tagger::Config &cfg, const std::string &preface, Impl *impl)
  : _impl(impl), _cfg(cfg), _feature_types(impl->feature_types) { }

Tagger::Tagger(const Tagger &other)
  : _impl(share(other._impl)), _cfg(other._cfg), _feature_types(other._feature_types) { }

void Tagger::Impl::train(Reader &reader) {
  extract(reader, instances);
  inv_sigma_sq = 1.0 / (cfg.sigma() * cfg.sigma());
  const size_t n = attributes.nfeatures();

  lbfgsfloatval_t *x = lbfgs_malloc(n);
  lbfgs_parameter_t param;

  for(int i = 0; i < n; ++i)
    x[i] = 1.0;
  lbfgs_parameter_init(&param);

  int ret = lbfgs(n, x, NULL, evaluate, progress, (void *)this, &param);

  std::cerr << "L-BFGS optimization terminated with status code " << ret << std::endl;
  lbfgs_free(x);
  attributes.save_weights(cfg.weights(), preface);
}

double Tagger::Impl::log_likelihood(void) {
  double llhood = 0.0;
  uint64_t nsents = 0;
  for (Instances::iterator i = instances.begin(); i != instances.end(); ++i) {
    for (Contexts::iterator j = i->begin(); j != i->end(); ++j) {
      for (FeaturePtrs::iterator k = j->features.begin(); k != j->features.end(); ++k)
        llhood += (*k)->lambda;
    }
  }
  //std::cout << llhood << ' ' << log_z << ' ' << (attributes.sum_lambda_sq() * inv_sigma_sq * 0.5) << std::endl;
  return llhood - log_z - (attributes.sum_lambda_sq() * inv_sigma_sq * 0.5);
}

void Tagger::Impl::compute_psis(Contexts &contexts, PSIs &full_psis, PSIs &ind_psis) {
  for(int i = 0; i < contexts.size(); ++i) {
    double full_sum = 0.0;
    double state_sum = 0.0;
    double trans_sum = 0.0;
    Context &c = contexts[i];
    for (FeaturePtrs::iterator j = c.features.begin(); j != c.features.end(); ++j) {
      full_sum += (*j)->lambda;
      if ((*j)->klasses.prev == None::val)
        state_sum += (*j)->lambda;
      else
        trans_sum += (*j)->lambda;
    }
    trans_sum = exp(trans_sum);
    full_psis[i][c.klasses.prev][c.klasses.curr] = exp(full_sum);
    ind_psis[i][c.klasses.prev][c.klasses.curr] = trans_sum;
    ind_psis[i][None::val][c.klasses.curr] = exp(state_sum);
  }
}

void Tagger::Impl::print_psis(Contexts &contexts, PSIs &psis) {
  for(int i = 0; i < contexts.size(); ++i){
    std::cout << "Position " << i << std::endl;
    for (int j = 0; j < psis[i].size(); ++j){
      if(j == 0){
        std::cout << std::setw(16) << ' ';
        for (int k = 0; k < psis[i][j].size(); ++k)
          std::cout << std::setw(16) << tags.str(k);
        std::cout << std::endl;
      }
      std::cout << std::setw(16) << tags.str(j);
      for (int k = 0; k < psis[i][j].size(); ++k){
        std::cout << std::setw(16) << psis[i][j][k] << ' ';
      }
      std::cout << std::endl;
    }
    std::cout << '\n' << std::endl;
  }
}

void Tagger::Impl::forward(Contexts &contexts, PDFs &alphas, PSIs &psis, PDF &scale) {
  //std::cout << "forward" << std::endl;
  double sum = 0.0;

  for (Tag curr(2); curr < tags.size(); ++curr) {
    double val = psis[0][Sentinel::val][curr];
    alphas[0][curr] = val;
    sum += val;
    //std::cout << tags.str(curr) << ' ' << val << ' ' << sum << std::endl;
  }
  if (sum == 0)
    sum = 1.0;
  scale[0] = 1.0 / sum;
  vector_scale(alphas[0], scale[0]);
  //std::cout << "sum: " << sum << std::endl;

  //for (Tag curr(2); curr < tags.size(); ++curr) {
    //std::cout << alphas[1][curr] << std::endl;
    //assert(!isinf(alphas[1][curr]) && !std::isnan(alphas[1][curr]));
  //}
  //std::cout << std::endl;

  for (size_t i = 1; i < contexts.size(); ++i) {
    sum = 0.0;
    //std::cout << '\n' << "Position " << i << std::endl;
    for (Tag curr(2); curr < tags.size(); ++curr) {
      for (Tag prev(2); prev < tags.size(); ++prev) {
        double val = alphas[i-1][prev] * psis[i][prev][curr];
        //std::cout << tags.str(prev) << ' ' << tags.str(curr) << ' ' << alphas[i-1][prev] << ' ' << psis[i][prev][curr] << ' ' << val << std::endl;
        alphas[i][curr] += val;
        sum += val;
      }
    }
    if (sum == 0)
      sum = 1.0;
    scale[i] = 1.0 / sum;
    //std::cout << "sum: " << sum << std::endl;
    vector_scale(alphas[i], scale[i]);
    //for (Tag curr(0); curr < tags.size(); ++curr) {
      //std::cout << tags.str(curr) << ' ' << alphas[i][curr] << std::endl;
      //assert(!isinf(alphas[i][curr]) && !std::isnan(alphas[i][curr]));
    //}
    //std::cout << std::endl;
  }
  //std::cout << alphas[contexts.size()][0] << std::endl;
  //Z[index] = exp(alphas[contexts.size()][0]);
  log_z += -vector_sum_log(scale);
  //std::cout << Z[index] << std::endl;
  //assert(!isinf(Z[index]) && !std::isnan(Z[index]));
}

void Tagger::Impl::backward(Contexts &contexts, PDFs &betas, PSIs &psis, PDF &scale) {
  //std::cout << "backward" << std::endl;
  for (Tag curr(2); curr < tags.size(); ++curr) {
    //double val = psis[contexts.size() - 1][curr][Sentinel::val];
    double val = 1;
    betas[contexts.size() - 1][curr] = val;
  }
  vector_scale(betas[contexts.size() - 1], scale[contexts.size() - 1]);
  //std::cout << "scale: " << scale[contexts.size() - 2] << std::endl;
  //for (Tag curr(2); curr < tags.size(); ++curr) {
    //std::cout << betas[contexts.size() - 2][curr] << std::endl;
    //assert(!isinf(betas[contexts.size() - 2][curr]) && !std::isnan(betas[contexts.size() - 2][curr]));
  //}
  //std::cout << std::endl;

  for (int i = contexts.size() - 2; i >= 0; --i) {
    for (Tag curr(2); curr < tags.size(); ++curr) {
      for (Tag next(2); next < tags.size(); ++next) {
        betas[i][curr] += betas[i+1][next] * psis[i+1][curr][next];
        //std::cout << betas[i+1][next] << ' ' << psis[i+1][curr][next] << std::endl;
      }
    }
    //std::cout << "scale: " << scale[i] << std::endl;
    //for (Tag curr(2); curr < tags.size(); ++curr) {
      //std::cout << betas[i][curr] << std::endl;
      //assert(!isinf(betas[i][curr]) && !std::isnan(betas[i][curr]));
    //}
    //std::cout << std::endl;
    vector_scale(betas[i], scale[i]);
    //for (Tag curr(2); curr < tags.size(); ++curr) {
      //std::cout << betas[i][curr] << std::endl;
      //assert(!isinf(betas[i][curr]) && !std::isnan(betas[i][curr]));
    //}
    //std::cout << std::endl;
  }
}

int Tagger::Impl::progress(void *instance, const lbfgsfloatval_t *x,
    const lbfgsfloatval_t *g, const lbfgsfloatval_t fx,
    const lbfgsfloatval_t xnorm, const lbfgsfloatval_t gnorm,
    const lbfgsfloatval_t step, int n, int k, int ls) {
  std::cerr << "Iteration " << k << std::endl;
  std::cerr << "  fx = " << fx;
  std::cerr << "  xnorm = " << xnorm << " gnorm = " << gnorm << " step = " << step << std::endl;
  return 0;
}

lbfgsfloatval_t Tagger::Impl::_evaluate(const lbfgsfloatval_t *x,
    lbfgsfloatval_t *g, const int n, const lbfgsfloatval_t step) {
  attributes.copy_lambdas(x);
  attributes.reset_estimations();

  log_z = 0.0;
  for (Instances::iterator i = instances.begin(); i != instances.end(); ++i) {
    Contexts &contexts = *i;
    contexts.reset();

    PDFs &alphas(i->alphas);
    PDFs &betas(i->betas);
    PSIs &full_psis(i->psis);
    PSIs &ind_psis(i->ind_psis);
    PDFs &trans_probs(i->trans_probs);
    PDF &scale(i->scale);

    compute_psis(contexts, full_psis, ind_psis);
    forward(contexts, alphas, full_psis, scale);
    backward(contexts, betas, full_psis, scale);

    for (int j = 0; j < i->size(); ++j) {
      TagPair &klasses = contexts[j].klasses;
      for (FeaturePtrs::iterator k = contexts[j].features.begin(); k != contexts[j].features.end(); ++k) {
        double alpha = (j > 0) ? alphas[j-1][klasses.prev] : 1.0;
        double beta = betas[j][klasses.curr];
        (*k)->est += alpha * full_psis[j][klasses.prev][klasses.curr] * beta;

        //std::cout << j << ' ' << tags.str(klasses.prev) << ' ' << tags.str(klasses.curr) << ' ' << alpha << ' ' << beta << ' ' << full_psis[j][klasses.prev][klasses.curr] << std::endl;
        //assert(!std::isnan(est));
      }
      //std::cout << std::endl;
    }
  }
  //attributes.prep_finite_differences();
  //finite_differences(g, false);

  attributes.copy_gradients(g, inv_sigma_sq);
  //attributes.print(inv_sigma_sq);

  lbfgsfloatval_t llhood = log_likelihood();
  std::cout << "llhood: " << llhood << std::endl;
  return -llhood;
}

lbfgsfloatval_t Tagger::Impl::evaluate(void *_instance, const lbfgsfloatval_t *x,
    lbfgsfloatval_t *g, const int n, const lbfgsfloatval_t step) {
  return reinterpret_cast<Tagger::Impl *>(_instance)->_evaluate(x, g, n, step);
}

void Tagger::Impl::extract(Reader &reader, Instances &instances) {
  std::cerr << "beginning pass 1" << std::endl;
  _pass1(reader);
  reader.reset();
  std::cerr << "beginning pass 2" << std::endl;
  _pass2(reader);
  reader.reset();
  std::cerr << "beginning pass 3" << std::endl;
  _pass3(reader, instances);
}

void Tagger::Impl::finite_differences(lbfgsfloatval_t *g, bool overwrite) {
  double EPSILON = 1.0e-6;
  double step = 1.0;
  int i = 0;
  while (attributes.inc_next_gradient(step * EPSILON)) {
    double plus_llhood = -log_likelihood();
    attributes.dec_gradient(step * EPSILON);
    double minus_llhood = -log_likelihood();
    //std::cout << plus_llhood << ' ' << minus_llhood << ' ' << plus_llhood - minus_llhood << std::endl;
    double val = (plus_llhood - minus_llhood) / (2 * step * EPSILON);
    attributes.print_current_gradient(val, inv_sigma_sq);
    if(overwrite)
      g[i++] = val;
  }
}

} }
