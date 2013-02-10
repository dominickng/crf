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

        class Model : public config::Info {
          public:
            config::Op<uint64_t> nattributes;
            config::Op<uint64_t> nfeatures;
            config::Op<uint64_t> max_size;
            Model(const std::string &name, const std::string &desc, const config::OpPath &base)
              : config::Info(name, desc, base),
              nattributes(*this, "nattributes", "the number of attributes", 0, true),
              nfeatures(*this, "nfeatures", "the number of features", 0, true),
              max_size(*this, "max_size", "the size of the largest sentence", 0, true)
          { }

            virtual ~Model(void) { }
        };

      protected:
        class Impl;

        Impl *_impl;
        Config &cfg;
        Types &types;

        Tagger(Tagger::Config &cfg, const std::string &preface, Impl *impl);
        Tagger(const Tagger &other);
        virtual ~Tagger(void) { release(_impl); }
    };

    class Tagger::Impl : public Util::Shared {
      protected:
        virtual lbfgsfloatval_t log_likelihood(void);

        lbfgsfloatval_t _evaluate(const lbfgsfloatval_t *x,
            lbfgsfloatval_t *g, const int n, const lbfgsfloatval_t step);

        virtual void reg(void);

        virtual void compute_psis(Contexts &contexts, PSIs &psis);
        virtual void print_psis(Contexts &contexts, PSIs &psis);
        virtual void forward(Contexts &contexts, PDFs &alphas, PSIs &psis, PDF &scale);
        virtual void forward_noscale(Contexts &contexts, PDFs &alphas, PSIs &psis);
        virtual void backward(Contexts &contexts, PDFs &betas, PSIs &psis, PDF &scale);
        virtual void backward_noscale(Contexts &contexts, PDFs &betas, PSIs &psis);
        virtual void print_fwd_bwd(Contexts &contexts, PDFs &pdfs, PDF &scale);
        virtual void reset(PDFs &alphas, PDFs &betas, PSIs &psis, PDF &scale, const size_t size);

        virtual void _pass1(Reader &reader) = 0;
        virtual void _pass2(Reader &reader) = 0;
        virtual void _pass3(Reader &reader, Instances &instances) = 0;
      public:
        Config &cfg;
        Types &types;
        Model model;
        Registry registry;

        Lexicon lexicon;
        TagSet tags;
        Attributes attributes;
        Instances instances;
        Weights weights;
        Attribs2Weights attribs2weights;

        WordDict w_dict;
        BiWordDict ww_dict;
        AffixDict a_dict;

        const std::string preface;
        double inv_sigma_sq;
        double log_z;
        uint64_t ntags;

        Impl(Config &cfg, Types &types, const std::string &preface)
          : Util::Shared(), cfg(cfg), types(types),
            model("info", "Tagger model info file", cfg.model), registry(),
            lexicon(cfg.lexicon()), tags(cfg.tags()),
            attributes(), instances(), weights(), attribs2weights(),
            w_dict(lexicon), ww_dict(lexicon), a_dict(), preface(preface),
            inv_sigma_sq(), log_z(0.0), ntags() { }

        virtual ~Impl(void) { /* nothing */ }

        virtual void _read_weights(Model &model);
        virtual void _read_attributes(Model &model);
        virtual void _load_model(Model &model) {
          model.read_config();
          _read_weights(model);
          _read_attributes(model);
        }

        virtual void load(void) {
          lexicon.load();
          tags.load();
          reg();
          _load_model(model);
        }

        virtual void run_tag(Reader &reader, Writer &writer) = 0;
        virtual void tag(State &state, Sentence &sent) = 0;

        virtual void train(Reader &reader);
        virtual void extract(Reader &reader, Instances &instances);

        virtual void finite_differences(Instances &instances, PDFs &alphas, PDFs &betas, PSIs &psis, PDF &scale, lbfgsfloatval_t *g, bool overwrite=false);

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
