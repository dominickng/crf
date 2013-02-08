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
#include "crf/features.h"
#include "crf/tagger.h"

template <typename T>
inline bool isinf(T value) {
  return std::numeric_limits<T>::has_infinity && value == std::numeric_limits<T>::infinity();
}

namespace NLP { namespace CRF {

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

void Tagger::Impl::train(Reader &reader) {
  reg();
  inv_sigma_sq = 1.0 / (cfg.sigma() * cfg.sigma());
  extract(reader, instances);
  const size_t n = attributes.nfeatures();

  lbfgsfloatval_t *x = lbfgs_malloc(n);
  lbfgs_parameter_t param;

  for(int i = 0; i < n; ++i)
    x[i] = 1.0;
  lbfgs_parameter_init(&param);
  param.linesearch = LBFGS_LINESEARCH_BACKTRACKING;
  //param.delta = 1e-8;
  //param.past = 2;

  int ret = lbfgs(n, x, NULL, evaluate, progress, (void *)this, &param);

  std::cerr << "L-BFGS optimization terminated with status code " << ret << std::endl;
  lbfgs_free(x);
  model.nattributes(attributes.size());
  model.nfeatures(attributes.nfeatures());
  model.save(preface);
  attributes.save_features(cfg.features(), preface);
}

double Tagger::Impl::log_likelihood(void) {
  double llhood = 0.0;
  for (Instances::iterator i = instances.begin(); i != instances.end(); ++i) {
    for (Contexts::iterator j = i->begin(); j != i->end(); ++j) {
      for (FeaturePtrs::iterator k = j->features.begin(); k != j->features.end(); ++k)
        llhood += (*k)->lambda;
    }
  }
  //std::cout << llhood << ' ' << log_z << ' ' << (attributes.sum_lambda_sq() * inv_sigma_sq * 0.5) << std::endl;
  return -(llhood - log_z - (attributes.sum_lambda_sq() * inv_sigma_sq * 0.5));
}

void Tagger::Impl::compute_psis(Contexts &contexts, PSIs &psis) {
  for(int i = 0; i < contexts.size(); ++i) {
    double sum = 0.0;
    Context &c = contexts[i];
    for (FeaturePtrs::iterator j = c.features.begin(); j != c.features.end(); ++j)
      sum += (*j)->lambda;
    psis[i][c.klasses.prev][c.klasses.curr] = exp(sum);
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

void Tagger::Impl::print_fwd_bwd(Contexts &contexts, PDFs &pdfs, PDF &scale) {
  std::cout << std::setw(16) << ' ';
  for(int i = 0; i < contexts.size(); ++i)
    std::cout << std::setw(16) << tags.str(contexts[i].klasses.curr);
  std::cout << std::endl;
  for(Tag curr(0); curr < tags.size(); ++curr) {
    std::cout << std::setw(16) << tags.str(curr);
    for(int i = 0; i < contexts.size(); ++i)
      std::cout << std::setw(16) << pdfs[i][curr];
    std::cout << std::endl;
  }
  std::cout << std::setw(16) << "scale:";
  for(int i = 0; i < contexts.size(); ++i)
    std::cout << std::setw(16) << scale[i];

  std::cout << '\n' << std::endl;
}

void Tagger::Impl::forward(Contexts &contexts, PDFs &alphas, PSIs &psis, PDF &scale) {
  double sum = 0.0;

  for (Tag curr(2); curr < tags.size(); ++curr) {
    double val = psis[0][Sentinel::val][curr];
    alphas[0][curr] = val;
    sum += val;
    //std::cout << tags.str(curr) << ' ' << val << ' ' << sum << std::endl;
  }
  if (sum == 0.0)
    sum = 1.0;
  scale[0] = 1.0 / sum;
  vector_scale(alphas[0], scale[0], tags.size());
  //std::cout << "sum: " << sum << std::endl;

  for (size_t i = 1; i < contexts.size(); ++i) {
    sum = 0.0;
    for (Tag curr(2); curr < tags.size(); ++curr) {
      for (Tag prev(2); prev < tags.size(); ++prev) {
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
    vector_scale(alphas[i], scale[i], tags.size());
  }
  log_z += -vector_sum_log(scale, contexts.size());
  //std::cout << "scale Z: " << -vector_sum_log(scale, contexts.size()) << std::endl;
}

void Tagger::Impl::forward_noscale(Contexts &contexts, PDFs &alphas, PSIs &psis) {
  for (Tag curr(2); curr < tags.size(); ++curr) {
    double val = psis[0][Sentinel::val][curr];
    alphas[0][curr] = val;
  }

  for (size_t i = 1; i < contexts.size(); ++i) {
    for (Tag curr(2); curr < tags.size(); ++curr) {
      for (Tag prev(2); prev < tags.size(); ++prev) {
        double val = alphas[i-1][prev] * psis[i][prev][curr];
        //std::cout << tags.str(prev) << ' ' << tags.str(curr) << ' ' << alphas[i-1][prev] << ' ' << psis[i][prev][curr] << ' ' << val << std::endl;
        alphas[i][curr] += val;
      }
    }
  }
  std::cout << "noscale Z: " << log(vector_sum(alphas[contexts.size() - 1], tags.size())) << std::endl;
}


void Tagger::Impl::backward(Contexts &contexts, PDFs &betas, PSIs &psis, PDF &scale) {
  //std::cout << "backward" << std::endl;
  for (Tag curr(2); curr < tags.size(); ++curr)
    betas[contexts.size() - 1][curr] = 1.0;
  vector_scale(betas[contexts.size() - 1], scale[contexts.size() - 1], tags.size());

  for (int i = contexts.size() - 2; i >= 0; --i) {
    for (Tag curr(2); curr < tags.size(); ++curr) {
      for (Tag next(2); next < tags.size(); ++next) {
        betas[i][curr] += betas[i+1][next] * psis[i+1][curr][next];
        //std::cout << betas[i+1][next] << ' ' << psis[i+1][curr][next] << std::endl;
      }
    }
    vector_scale(betas[i], scale[i], tags.size());
      //assert(!isinf(betas[i][curr]) && !std::isnan(betas[i][curr]));
  }
}

void Tagger::Impl::backward_noscale(Contexts &contexts, PDFs &betas, PSIs &psis) {
  for (Tag curr(2); curr < tags.size(); ++curr)
    betas[contexts.size() - 1][curr] = 1.0;

  for (int i = contexts.size() - 2; i >= 0; --i) {
    for (Tag curr(2); curr < tags.size(); ++curr) {
      for (Tag next(2); next < tags.size(); ++next) {
        betas[i][curr] += betas[i+1][next] * psis[i+1][curr][next];
      }
    }
  }
  double z = 0.0;
  for (Tag next(2); next < tags.size(); ++next)
    z += betas[0][next] * psis[0][Sentinel::val][next];
  std::cout << "noscale Z: " << log(z) << std::endl;
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

void Tagger::Impl::reset(PDFs &alphas, PDFs &betas, PSIs &psis, PDF &scale, const size_t size) {
  std::fill(scale.begin(), scale.begin() + size, 1.0);

  for (int i = 0; i < size; ++i) {
    std::fill(alphas[i].begin(), alphas[i].end(), 0.0);
    std::fill(betas[i].begin(), betas[i].end(), 0.0);
    for (int j = 0; j < psis[i].size(); ++j)
      std::fill(psis[i][j].begin(), psis[i][j].end(), 1.0);
  }
}

lbfgsfloatval_t Tagger::Impl::_evaluate(const lbfgsfloatval_t *x,
    lbfgsfloatval_t *g, const int n, const lbfgsfloatval_t step) {
  attributes.copy_lambdas(x);
  attributes.reset_estimations();

  log_z = 0.0;
  PDFs alphas;
  PDFs betas;
  PSIs psis;
  PDF scale;

  for (int i = 0; i < model.max_size(); ++i) {
    alphas.push_back(PDF(tags.size(), 0.0));
    betas.push_back(PDF(tags.size(), 0.0));
    scale.push_back(1.0);
    psis.push_back(PDFs(0));
    for (int j = 0; j < tags.size(); ++j)
      psis[i].push_back(PDF(tags.size(), 1.0));
  }

  for (Instances::iterator i = instances.begin(); i != instances.end(); ++i) {
    Contexts &contexts = *i;
    reset(alphas, betas, psis, scale, i->size());

    compute_psis(contexts, psis);

    //print_psis(contexts, psis);
    //forward_noscale(contexts, alphas, psis);
    //backward_noscale(contexts, betas, psis);
    //for (int j = 0; j < alphas.size(); ++j) {
      //std::fill(alphas[j].begin(), alphas[j].end(), 0.0);
      //std::fill(betas[j].begin(), betas[j].end(), 0.0);
    //}

    forward(contexts, alphas, psis, scale);
    backward(contexts, betas, psis, scale);
    //print_fwd_bwd(contexts, alphas, scale);
    //print_fwd_bwd(contexts, betas, scale);

    for (int j = 0; j < i->size(); ++j) {
      TagPair &klasses = contexts[j].klasses;
      for (FeaturePtrs::iterator k = contexts[j].features.begin(); k != contexts[j].features.end(); ++k) {
        double alpha = (j > 0) ? alphas[j-1][klasses.prev] : 1.0;
        double beta = betas[j][klasses.curr];
        (*k)->est += alpha * psis[j][klasses.prev][klasses.curr] * beta;

        //std::cout << j << ' ' << tags.str(klasses.prev) << ' ' << tags.str(klasses.curr) << ' ' << alpha << ' ' << beta << ' ' << psis[j][klasses.prev][klasses.curr] << std::endl;
        //assert(!std::isnan(est));
      }
      //std::cout << std::endl;
    }
  }
  //attributes.prep_finite_differences();
  //finite_differences(instances, alphas, betas, psis, scale, g, false);

  attributes.copy_gradients(g, inv_sigma_sq);
  //attributes.print(inv_sigma_sq);

  lbfgsfloatval_t llhood = log_likelihood();
  std::cout << "llhood: " << llhood << std::endl;
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

void Tagger::Impl::finite_differences(Instances &instances, PDFs &alphas, PDFs &betas, PSIs &psis, PDF &scale, lbfgsfloatval_t *g, bool overwrite) {
  double old_log_z = log_z;
  double EPSILON = 1.0e-4;
  int index = 0;
  double llhood = log_likelihood();

  while (attributes.inc_next_gradient(EPSILON)) {
    log_z = 0.0;
    // re-estimate log Z
    for (Instances::iterator i = instances.begin(); i != instances.end(); ++i) {
      Contexts &contexts = *i;
      reset(alphas, betas, psis, scale, i->size());
      compute_psis(contexts, psis);
      forward(contexts, alphas, psis, scale);
    }

    double plus_llhood = log_likelihood();
    double val = (plus_llhood - llhood) / EPSILON;
    if(overwrite)
      g[index++] = val;
    else
      attributes.print_current_gradient(val, inv_sigma_sq);
  }

  log_z = old_log_z;
}

void Tagger::Impl::reg(void) {
  registry.reg(Types::w, types.use_words, new WordGen(w_dict));
  registry.reg(Types::pw, types.use_prev_words, new OffsetWordGen(w_dict, -1));
  registry.reg(Types::ppw, types.use_prev_words, new OffsetWordGen(w_dict, -2));
  registry.reg(Types::nw, types.use_next_words, new OffsetWordGen(w_dict, 1));
  registry.reg(Types::nnw, types.use_next_words, new OffsetWordGen(w_dict, 2));
}

Tagger::Tagger(Tagger::Config &cfg, const std::string &preface, Impl *impl)
  : _impl(impl), cfg(cfg), types(_impl->types) { }

Tagger::Tagger(const Tagger &other)
  : _impl(share(other._impl)), cfg(other.cfg), types(other.types) { }

} }
