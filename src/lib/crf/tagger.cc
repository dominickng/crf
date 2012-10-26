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

double Tagger::Impl::psi(Context &c, TagPair &tp) {
  double psi = 0.0;
  for (FeaturePtrs::iterator j = c.features.begin(); j != c.features.end(); ++j)
    if ((*j)->klasses == tp)
      psi += (*j)->lambda;
  return exp(psi);
}

void Tagger::Impl::compute_psis(Contexts &contexts, PSIs &psis) {
  for(int i = 0; i < contexts.size(); ++i) {
    TagPair klasses = contexts[i].klasses;
    psis[i][klasses.prev][klasses.curr] = psi(contexts[i], klasses);
    klasses.prev = None::val;
    psis[i][klasses.prev][klasses.curr] = psi(contexts[i], klasses);
  }
}

void Tagger::Impl::forward(Contexts &contexts, PDFs &alphas, PSIs &psis, PDF &scale) {
  double sum = 0.0;

  for (Tag curr((uint16_t)2); curr < tags.size(); ++curr) {
    TagPair tp(0, curr);
    double val = psis[1][0][curr];
    alphas[1][curr] = val;
    sum += val;
    //std::cout << "0 alpha after log " << tags.str(curr) << ' ' << val << ' ' << std::endl;
  }
  if (sum == 0)
    sum = 1.0;
  scale[1] = 1.0 / sum;
  vector_scale(alphas[1], scale[1]);

  //for(PDFs::iterator it = psis.begin(); it != psis.end(); ++it) {
    //for(PDF::iterator jt = it->begin(); jt != it->end(); ++jt)
      //std::cout << *jt << ' ';
    //std::cout << std::endl;
  //}

  sum = 0.0;
  for (size_t i = 2; i < contexts.size() - 1; ++i) {
    for (Tag curr((uint16_t)2); curr < tags.size(); ++curr) {
      for (Tag prev((uint16_t)2); prev < tags.size(); ++prev) {
        TagPair tp(prev, curr);
        double val = alphas[i-1][prev] * psis[i][prev][curr];
        //std::cout << tags.str(prev) << ' ' << tags.str(curr) << ' ' << alphas[i-1][prev.id()] << ' ' << psis[i][tp.index(tags.size())] << std::endl;
        alphas[i][curr] += val;
        sum += val;

        //if (isinf(alphas[i][curr.id()]) || std::isnan(alphas[i][curr.id()])) {
          //for(PDFs::iterator it = psis.begin(); it != psis.end(); ++it) {
            //for(PDF::iterator jt = it->begin(); jt != it->end(); ++jt)
              //std::cout << *jt << ' ';
            //std::cout << std::endl;
          //}
          //std::cout << val << ' ' << alphas[i-1][prev.id()] << ' ' << psis[i][tp.index(tags.size())] << std::endl;
        //}
      }
    }
    if (sum == 0)
      sum = 1.0;
    scale[i] = 1.0 / sum;
    vector_scale(alphas[i], scale[i]);
    for (Tag curr((uint16_t)0); curr < tags.size(); ++curr) {
      //std::cout << i << " alpha before log " << tags.str(curr) << ' ' << alphas[i][curr.id()] << ' ' << std::endl;
    //alphas[i][curr.id()] = log(alphas[i][curr.id()]);
      //std::cout << i << " alpha after log " << tags.str(curr) << ' ' << alphas[i][curr.id()] << ' ' << std::endl;

      assert(!isinf(alphas[i][curr]) && !std::isnan(alphas[i][curr]));
    }
    //std::cout << std::endl;
  }
  //std::cout << alphas[contexts.size()][0] << std::endl;
  //Z[index] = exp(alphas[contexts.size()][0]);
  log_z += -vector_sum_log(scale);
  //std::cout << Z[index] << std::endl;
  //assert(!isinf(Z[index]) && !std::isnan(Z[index]));
}

void Tagger::Impl::backward(Contexts &contexts, PDFs &betas, PSIs &psis, PDF &scale) {
  for (Tag curr((uint16_t)2); curr < tags.size(); ++curr) {
    TagPair tp(curr, 0);
    double val = psis[contexts.size() - 2][tp.curr][0];
    betas[contexts.size() - 2][curr] = val;
  }
  vector_scale(betas[contexts.size() - 2], scale[contexts.size() - 2]);

  for (int i = contexts.size() - 3; i >= 1; --i) {
    for (Tag curr((uint16_t)2); curr < tags.size(); ++curr) {
      for (Tag next((uint16_t)2); next < tags.size(); ++next) {
        TagPair tp(curr, next);
        betas[i][curr] += betas[i+1][next] * psis[i][curr][next];
      }
    }
    vector_scale(betas[i], scale[i]);
    for (Tag curr((uint16_t)0); curr < tags.size(); ++curr) {
      //betas[i][curr.id()] = log(betas[i][curr.id()]);
      //std::cout << i << " beta after log " << tags.str(curr) << ' ' << betas[i][curr.id()] << ' ' << std::endl;
      assert(!isinf(betas[i][curr]) && !std::isnan(betas[i][curr]));
    }
  }
  //std::cout << index << ' ' << Z[index] << ' ' << exp(betas[0][0]) << ' ' << fabs(Z[index] - exp(betas[0][0])) << std::endl;
  //if (fabs(Z[index] - exp(betas[0][0])) > 1.0) {
    //std::cout << index << ' ' << Z[index] << ' ' << exp(betas[0][0]) << ' ' << fabs(Z[index] - exp(betas[0][0])) << std::endl;
  //}
}

int Tagger::Impl::progress(void *instance, const lbfgsfloatval_t *x,
    const lbfgsfloatval_t *g, const lbfgsfloatval_t fx,
    const lbfgsfloatval_t xnorm, const lbfgsfloatval_t gnorm,
    const lbfgsfloatval_t step, int n, int k, int ls) {
  std::cout << "Iteration " << k << std::endl;
  std::cout << "  fx = " << fx;
  std::cout << "  xnorm = " << xnorm << " gnorm = " << gnorm << " step = " << step << std::endl;
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
    PSIs &psis(i->psis);
    PDF &scale(i->scale);

    compute_psis(contexts, psis);
    forward(contexts, alphas, psis, scale);
    backward(contexts, betas, psis, scale);

    for (int j = 1; j < i->size() - 1; ++j) {
      TagPair &klasses = (*i)[j].klasses;
      for (FeaturePtrs::iterator k = contexts[j].features.begin(); k != contexts[j].features.end(); ++k) {
        double alpha = alphas[j-1][klasses.prev];
        double beta = betas[j][klasses.curr];
        double est = alpha * psis[j][klasses.prev][klasses.curr] * beta;
        //std::cout << j << ' ' << tags.str(klasses.prev) << ' ' << tags.str(klasses.curr) << ' ' << alpha << ' ' << beta << ' ' << psis[j][klasses.prev][klasses.curr] << std::endl;
        (*k)->est += est;
        assert(!std::isnan(est));
      }
      //std::cout << std::endl;
    }
  }
  attributes.prep_finite_differences();
  finite_differences();

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

void Tagger::Impl::finite_differences(void) {
  double EPSILON = 1.0e-6;
  double step = 1.0;
  while (attributes.inc_next_gradient(step * EPSILON)) {
    double plus_llhood = -log_likelihood();
    attributes.dec_gradient(step * EPSILON);
    double minus_llhood = -log_likelihood();
    //std::cout << plus_llhood << ' ' << minus_llhood << ' ' << plus_llhood - minus_llhood << std::endl;
    attributes.print_current_gradient((plus_llhood - minus_llhood) / (2 * step * EPSILON), inv_sigma_sq);
  }
}

} }
