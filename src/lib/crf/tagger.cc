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

template<typename T>
inline bool isinf(T value)
{
return std::numeric_limits<T>::has_infinity &&
value == std::numeric_limits<T>::infinity();
}

namespace NLP { namespace CRF {

Tagger::Tagger(Tagger::Config &cfg, const std::string &preface, Impl *impl)
  : _impl(impl), _cfg(cfg), _feature_types(impl->feature_types) { }

Tagger::Tagger(const Tagger &other)
  : _impl(share(other._impl)), _cfg(other._cfg), _feature_types(other._feature_types) { }

void Tagger::Impl::train(Reader &reader) {
  extract(reader, instances);
  inv_sigma_sq = 1.0 / (cfg.sigma() * cfg.sigma());
  npairs = TagPair::npairs(tags.size());
  const size_t n = attributes.nfeatures();

  Z.reserve(instances.size());

  lbfgsfloatval_t *x = lbfgs_malloc(n);
  lbfgs_parameter_t param;

  for(int i = 0; i < n; ++i)
    x[i] = 1.0;
  lbfgs_parameter_init(&param);
  param.linesearch = LBFGS_LINESEARCH_BACKTRACKING;

  int ret = lbfgs(n, x, NULL, evaluate, progress, (void *)this, &param);

  std::cerr << "L-BFGS optimization terminated with status code " << ret << std::endl;
  lbfgs_free(x);
  attributes.save_weights(cfg.weights(), preface);
}

size_t Tagger::Impl::tag_index(const Tag &t, const size_t j) const {
  return tags.size() * j + t.id();
}

size_t Tagger::Impl::psi_index(const TagPair &tp, const uint64_t index) const {
  return tp.index(tags.size()) + npairs * index;
}

double Tagger::Impl::log_likelihood(void) {
  double llhood = 0.0;
  double log_z = 0.0;
  uint64_t nsents = 0;
  for (Instances::iterator i = instances.begin(); i != instances.end(); ++i) {
    log_z += log(Z[nsents++]);
    for (Contexts::iterator j = i->begin(); j != i->end(); ++j) {
      for (FeaturePtrs::iterator k = j->features.begin(); k != j->features.end(); ++k)
        llhood += (*k)->lambda;
    }
  }
  std::cout << llhood << ' ' << log_z << ' ' << (attributes.sum_lambda_sq() * inv_sigma_sq * 0.5) << std::endl;
  return log_z - llhood + (attributes.sum_lambda_sq() * inv_sigma_sq * 0.5);
}

double Tagger::Impl::psi(Context &context, TagPair &tp) {
  double psi = 0.0;
  for (FeaturePtrs::iterator j = context.features.begin(); j != context.features.end(); ++j)
    if ((*j)->klasses == tp)
      psi += (*j)->lambda;
  return psi;
}

void Tagger::Impl::compute_psis(Contexts &contexts, PDFs &psis) {
  for(int i = 0; i < contexts.size(); ++i) {
    size_t j = contexts[i].klasses.index(tags.size());
    psis[i][j] = psi(contexts[i], contexts[i].klasses);
  }
}

void Tagger::Impl::forward(Contexts &contexts, PDFs &psis, PDF &scale, const uint64_t index) {
  PDFs &alphas(contexts.alphas);

  for (Tag curr((uint16_t)2); curr < tags.size(); ++curr) {
    TagPair tp(0, curr);
    double val = 1.0 + psis[0][tp.index(tags.size())];
    alphas[0][curr.id()] = val;
    //std::cout << "0 alpha after log " << tags.str(curr) << ' ' << val << ' ' << std::endl;
  }

  //for(PDFs::iterator it = psis.begin(); it != psis.end(); ++it) {
    //for(PDF::iterator jt = it->begin(); jt != it->end(); ++jt)
      //std::cout << *jt << ' ';
    //std::cout << std::endl;
  //}

  for (size_t i = 1; i <= contexts.size(); ++i) {
    for (Tag curr((uint16_t)2); curr < tags.size(); ++curr) {
      if (i == contexts.size()) {
        TagPair tp(curr, 0);
        double val = alphas[i-1][curr.id()] + psis[i][tp.index(tags.size())];
        alphas[i][0] += exp(val);
        continue;
      }
      for (Tag prev((uint16_t)2); prev < tags.size(); ++prev) {
        TagPair tp(prev, curr);
        double val = alphas[i-1][prev.id()] + psis[i][tp.index(tags.size())];
        //std::cout << tags.str(prev) << ' ' << tags.str(curr) << ' ' << alphas[i-1][prev.id()] << ' ' << psis[i][tp.index(tags.size())] << std::endl;
        alphas[i][curr.id()] += exp(val);
      }
    }
    for (Tag curr((uint16_t)0); curr < tags.size(); ++curr) {
      //std::cout << i << " alpha before log " << tags.str(curr) << ' ' << alphas[i][curr.id()] << ' ' << std::endl;
      alphas[i][curr.id()] = log(alphas[i][curr.id()]);
      //std::cout << i << " alpha after log " << tags.str(curr) << ' ' << alphas[i][curr.id()] << ' ' << std::endl;
      assert(!isinf(alphas[i][curr.id()]) && !std::isnan(alphas[i][curr.id()]));
    }
    //std::cout << std::endl;
  }
  //std::cout << alphas[contexts.size()][0] << std::endl;
  Z[index] = exp(alphas[contexts.size()][0]);
  //std::cout << Z[index] << std::endl;
  assert(!isinf(Z[index]) && !std::isnan(Z[index]));
}

void Tagger::Impl::backward(Contexts &contexts, PDFs &psis, PDF &scale, const uint64_t index) {

  PDFs &betas(contexts.betas);
  for (Tag curr((uint16_t)2); curr < tags.size(); ++curr) {
    TagPair tp(curr, 0);
    double val = 1.0 + psis[contexts.size()][tp.index(tags.size())];
    betas[contexts.size()][curr.id()] = val;
  }

  for (int i = contexts.size() - 1; i >= 0; --i) {
    for (Tag curr((uint16_t)2); curr < tags.size(); ++curr) {
      if (i == 0) {
        TagPair tp(0, curr);
        betas[0][0] += exp(betas[1][curr.id()] + psis[0][tp.index(tags.size())]);
        continue;
      }
      for (Tag next((uint16_t)2); next < tags.size(); ++next) {
        TagPair tp(curr, next);
        betas[i][curr.id()] += exp(betas[i+1][next.id()] + psis[i][tp.index(tags.size())]);
      }
    }
    for (Tag curr((uint16_t)0); curr < tags.size(); ++curr) {
      //std::cout << "betas " << i << ' ' << tags.str(curr) << ' ' << betas[tag_index(curr, i)] << ' ' << scale_factor << std::endl;
      betas[i][curr.id()] = log(betas[i][curr.id()]);
      //std::cout << i << " beta after log " << tags.str(curr) << ' ' << betas[i][curr.id()] << ' ' << std::endl;
      assert(!isinf(betas[i][curr.id()]) && !std::isnan(betas[i][curr.id()]));
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

  uint64_t index = 0;
  log_norm = 0.0;
  for (Instances::iterator i = instances.begin(); i != instances.end(); ++i) {
    Contexts &contexts = *i;
    PDFs &alphas(i->alphas);
    PDFs &betas(i->betas);
    PDFs &psis(i->psis);
    PDF scale(i->size(), 1.0);
    contexts.reset();
    compute_psis(contexts, psis);
    forward(contexts, psis, scale, index);
    backward(contexts, psis, scale, index);
    double inv_z = 1.0 / Z[index];
    for (int j = 0; j < i->size(); ++j) {
      for (FeaturePtrs::iterator k = contexts[j].features.begin(); k != contexts[j].features.end(); ++k) {
        double alpha = alphas[j][(*k)->klasses.prev.id()];
        double beta = betas[j+1][(*k)->klasses.curr.id()];
        double est = exp(alpha + psis[j][(*k)->klasses.index(tags.size())] + beta) * inv_z;
        //std::cout << j << ' ' << tags.str((*k)->klasses.prev) << ' ' << tags.str((*k)->klasses.curr) << ' ' << alpha << ' ' << beta << psis[j][(*k)->klasses.index(tags.size())] << std::endl;
        (*k)->est += est;
        assert(!std::isnan(est));
      }
      //std::cout << std::endl;
    }
    ++index;
  }
  attributes.copy_gradients(g, inv_sigma_sq);

  lbfgsfloatval_t llhood = log_likelihood();
  std::cout << "end of evaluate: " << llhood << std::endl;
  return llhood;
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

} }
