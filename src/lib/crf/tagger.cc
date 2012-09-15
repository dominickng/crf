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

namespace NLP { namespace CRF {

Tagger::Tagger(Tagger::Config &cfg, const std::string &preface, Impl *impl)
  : _impl(impl), _cfg(cfg), _feature_types(impl->feature_types) { }

Tagger::Tagger(const Tagger &other)
  : _impl(share(other._impl)), _cfg(other._cfg), _feature_types(other._feature_types) { }

void Tagger::Impl::reset_estimations(void) {
  attributes.reset_estimations();
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


lbfgsfloatval_t Tagger::Impl::evaluate(void *_instance, const lbfgsfloatval_t *x,
    lbfgsfloatval_t *g, const int n, const lbfgsfloatval_t step) {

  Tagger::Impl *instance = reinterpret_cast<Tagger::Impl *>(_instance);
  instance->attributes.copy_lambdas(x);
  instance->reset_estimations();
  uint64_t index = 0;
  for (Instances::iterator i = instance->instances.begin(); i != instance->instances.end(); ++i) {
    PDF alphas(instance->tags.size() * (instance->longest_sent + 1), 0.0);
    PDF betas(instance->tags.size() * (instance->longest_sent + 1), 0.0);
    PDF psis(instance->npairs * (instance->longest_sent + 1), 0.0);
    instance->compute_psis(*i, psis);
    instance->forward(*i, alphas, psis);
    instance->backward(*i, betas, psis, index);
    uint64_t sent_index = 1;
    for (Contexts::iterator j = i->begin(); j != i->end(); ++j) {
      for (FeaturePtrs::iterator k = j->features.begin(); k != j->features.end(); ++k) {
        double est = (1.0 / instance->Z[index]) * alphas[instance->tag_index((*k)->klasses.prev, sent_index)] * psis[instance->psi_index((*k)->klasses, sent_index - 1)] * betas[instance->tag_index((*k)->klasses.curr, sent_index + 1)];
        (*k)->est += est;
      }
      ++sent_index;
    }
    ++index;
  }
  instance->attributes.copy_gradient(g);

  std::cout << instance->log_likelihood() << std::endl;
  return instance->log_likelihood();
}

void Tagger::Impl::train(Reader &reader) {
  extract(reader, instances);
  inv_sigma_sq = 1.0 / (2 * cfg.sigma() * cfg.sigma());
  npairs = TagPair::npairs(tags.size());

  Z.reserve(instances.size());

  lbfgsfloatval_t fx;
  lbfgsfloatval_t *x = lbfgs_malloc(attributes.nfeatures());
  lbfgs_parameter_t param;
  param.linesearch = LBFGS_LINESEARCH_BACKTRACKING;

  for(int i = 0; i < attributes.nfeatures(); ++i)
    x[i] = 1.0;
  lbfgs_parameter_init(&param);

  int ret = lbfgs(attributes.nfeatures(), x, &fx, evaluate, progress, (void *)this, &param);

  printf("L-BFGS optimization terminated with status code = %d\n", ret);
  lbfgs_free(x);
}

size_t Tagger::Impl::tag_index(const Tag &t, const size_t j) const {
  return tags.size() * j + t.id();
}

size_t Tagger::Impl::psi_index(const TagPair &tp, const uint64_t index) const {
  return tp.index(tags.size()) * npairs + index;
}

double Tagger::Impl::log_likelihood(void) {
  double llhood = 0.0;
  double lambdas = 0.0;
  uint64_t nsents = 0;
  for (Instances::iterator i = instances.begin(); i != instances.end(); ++i) {
    llhood -= log(Z[nsents++]);
    for (Contexts::iterator j = i->begin(); j != i->end(); ++j) {
      for (FeaturePtrs::iterator k = j->features.begin(); k != j->features.end(); ++k) {
        double lambda = (*k)->lambda;
        llhood += lambda;
        lambdas += lambda * lambda;
      }
    }
  }
  return llhood - lambdas * inv_sigma_sq;
}

double Tagger::Impl::psi(Context &context, TagPair &tp) {
  double psi = 0.0;
  for (FeaturePtrs::iterator j = context.features.begin(); j != context.features.end(); ++j)
    if ((*j)->klasses == tp)
      psi += (*j)->lambda;
  return exp(psi);
}

void Tagger::Impl::compute_psis(Contexts &contexts, PDF &psis) {
  uint16_t sent_index = 0;
  for(Contexts::iterator i = contexts.begin(); i != contexts.end(); ++i) {
    for (Tag prev((uint16_t)0); prev < tags.size(); ++prev) {
      for (Tag curr((uint16_t)0); curr < tags.size(); ++curr) {
        TagPair tp(prev, curr);
        size_t j = psi_index(tp, sent_index);
        psis[j] = psi(*i, tp);
      }
    }
    ++sent_index;
  }
}

void Tagger::Impl::forward(Contexts &contexts, PDF &alphas, PDF &psis) {
  size_t sent_index = 1;

  for (Contexts::iterator i = contexts.begin(); i != contexts.end(); ++i) {
    for (Tag curr((uint16_t)0); curr < tags.size(); ++curr) {
      size_t j = tag_index(curr, sent_index);
      if (sent_index == 1) {
        TagPair tp(0, curr);
        alphas[j] = psis[psi_index(tp, sent_index)];
        continue;
      }
      for (Tag prev((uint16_t)1); prev < tags.size(); ++prev) {
        size_t k = tag_index(prev, sent_index - 1);
        TagPair tp(prev, curr);
        alphas[j] += alphas[k] * psis[psi_index(tp, sent_index)];
      }
    }
    ++sent_index;
  }
}

void Tagger::Impl::backward(Contexts &contexts, PDF &betas, PDF &psis, const uint64_t index) {
  size_t sent_index = contexts.size() - 1;

  for (Contexts::reverse_iterator i = contexts.rbegin(); i != contexts.rend(); ++i) {
    for (int _curr = tags.size() - 1; _curr >= 0; --_curr) {
      Tag curr((uint16_t)_curr);
      size_t j = tag_index(curr, sent_index);
      if (sent_index == contexts.size() - 1) {
        TagPair tp(curr, 0);
        betas[j] = psis[psi_index(tp, sent_index)];
        continue;
      }
      for (int _next = tags.size() - 1; _next >= 0; --_next)  {
        Tag next((uint16_t)_next);
        size_t k = tag_index(next, sent_index + 1);
        TagPair tp(curr, next);
        betas[j] += betas[k] * psis[psi_index(tp, sent_index)];
      }
    }
    --sent_index;
  }
  Z[index] = betas[tag_index(Tag((uint16_t)0), 0)];
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
