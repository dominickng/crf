namespace config = Util::config;

namespace NLP {
  namespace CRF {
    class Tagger {
      public:
        class Config : public config::OpGroup {
          public:
            config::OpPath model;
            config::OpPath lexicon;
            config::OpPath tags;
            config::OpPath attributes;
            config::OpPath features;
            config::OpPath weights;
            config::Op<double> sigma;
            config::Op<uint64_t> niterations;
            Config(const std::string &name, const std::string &desc)
              : config::OpGroup(name, desc),
            model(*this, "model", "location to save the model"),
            lexicon(*this, "lexicon", "location to save the lexicon file", "//lexicon", &model),
            tags(*this, "tags", "location to save the tag file", "//tags", &model),
            attributes(*this, "attributes", "location to save the attributes file", "//attributes", &model),
            features(*this, "features", "location to save the features file", "//features", &model),
            weights(*this, "weights", "location to save the weights file", "//weights", &model),
            sigma(*this, "sigma", "sigma value for regularization", 0.707, true),
            niterations(*this, "niterations", "number of training iterations", 1, false)
          { }

            virtual ~Config(void) { /* nothing */ }

        };

        FeatureTypes &feature_types(void) { return _feature_types; }

      protected:
        class Impl;

        Impl *_impl;
        Config &_cfg;
        FeatureTypes &_feature_types;

        Tagger(Tagger::Config &cfg, const std::string &preface, Impl *impl);
        Tagger(const Tagger &other);
        virtual ~Tagger(void) { release(_impl); }

    };

    class Tagger::Impl : public Util::Shared {
      protected:
        virtual lbfgsfloatval_t log_likelihood(void);

        lbfgsfloatval_t _evaluate(const lbfgsfloatval_t *x,
            lbfgsfloatval_t *g, const int n, const lbfgsfloatval_t step);

        virtual double psi(Context &c, TagPair &tp);
        virtual void compute_psis(Contexts &contexts, PSIs &psis);
        virtual void forward(Contexts &contexts, PDFs &alphas, PSIs &psis, PDF &scale);
        virtual void backward(Contexts &contexts, PDFs &betas, PSIs &psis, PDF &scale);

        virtual void _pass1(Reader &reader) = 0;
        virtual void _pass2(Reader &reader) = 0;
        virtual void _pass3(Reader &reader, Instances &instances) = 0;
      public:
        Config &cfg;
        Lexicon lexicon;
        TagSet tags;
        Attributes attributes;
        FeatureTypes feature_types;
        Instances instances;
        const std::string preface;
        double inv_sigma_sq;
        double log_z;

        Impl(Config &cfg, const std::string &preface)
          : Util::Shared(), cfg(cfg), lexicon(), tags(), attributes(),
            feature_types(tags), instances(), preface(preface),
            inv_sigma_sq(), log_z(0.0) { }

        virtual ~Impl(void) { /* nothing */ }

        virtual void train(Reader &reader);
        virtual void extract(Reader &reader, Instances &instances);

        virtual void finite_differences(void);

        static lbfgsfloatval_t evaluate(void *instance,
            const lbfgsfloatval_t *x, lbfgsfloatval_t *g, const int n,
            const lbfgsfloatval_t step);

        static int progress(void *instance, const lbfgsfloatval_t *x,
            const lbfgsfloatval_t *g, const lbfgsfloatval_t fx,
            const lbfgsfloatval_t xnorm, const lbfgsfloatval_t gnorm,
            const lbfgsfloatval_t step, int n, int k, int ls);

    };

  }
}
