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
            config::Op<double> sigma;
            config::Op<uint64_t> niterations;
            Config(const std::string &name, const std::string &desc)
              : config::OpGroup(name, desc),
            model(*this, "model", "location to save the model"),
            lexicon(*this, "lexicon", "location to save the lexicon file", "//lexicon", &model),
            tags(*this, "tags", "location to save the tag file", "//tags", &model),
            attributes(*this, "attributes", "location to save the attributes file", "//attributes", &model),
            features(*this, "features", "location to save the features file", "//features", &model),
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
        void reset_estimations(void);
        size_t tag_index(const Tag &t, const size_t j) const;
        size_t psi_index(const TagPair &t, const uint64_t index) const;
        virtual lbfgsfloatval_t log_likelihood(void);
        virtual double psi(Context &context, TagPair &tp);
        virtual void compute_psis(Contexts &contexts, PDF &psis);
        virtual void forward(Contexts &contexts, PDF &alphas, PDF &psis);
        virtual void backward(Contexts &contexts, PDF &betas, PDF &psis, const uint64_t index);

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
        PDF Z;
        size_t npairs;
        size_t longest_sent;
        const std::string preface;
        double inv_sigma_sq;

        Impl(Config &cfg, const std::string &preface)
          : Util::Shared(), cfg(cfg), lexicon(), tags(), attributes(),
            feature_types(tags), instances(), Z(), npairs(),
            preface(preface), inv_sigma_sq() { }

        virtual ~Impl(void) { /* nothing */ }

        virtual void train(Reader &reader);
        virtual void extract(Reader &reader, Instances &instances);

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
