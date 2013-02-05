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

        class FeatureTypes : public config::OpGroup {
          public:
            typedef std::vector<OpType *> Actives;
            typedef std::map<std::string, OpType *> Registry;
            OpType use_words;
            OpType use_prev_words;
            OpType use_prev_prev_words;
            OpType use_next_words;
            OpType use_next_next_words;
            Actives actives;
            Registry registry;

            FeatureTypes(void);

            void get_tagpair(TagSet &tags, Raws &raws, TagPair &tp, int i);
            void generate(Attributes &attributes, TagSet &tags, Sentence &sent, Contexts &contexts, Raws &raws, const bool extract);
            void generate(PDFs &dist);
            Attribute &load(const std::string &type, std::istream &in);
            virtual void reg(const Type &type, FeatureDict &dict);
            virtual void validate(void);
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

        virtual void reg(void);
        virtual void _add_features(Attribute attrib, PDFs &dist);
        virtual void add_features(Sentence &sent, PDFs &dist, size_t i);

        virtual void compute_psis(Contexts &contexts, PSIs &psis);
        virtual void print_psis(Contexts &contexts, PSIs &psis);
        virtual void forward(Contexts &contexts, PDFs &alphas, PSIs &psis, PDF &scale);
        virtual void backward(Contexts &contexts, PDFs &betas, PSIs &psis, PDF &scale);
        virtual void print_fwd_bwd(Contexts &contexts, PDFs &pdfs, PDF &scale);
        virtual void reset(PDFs &alphas, PDFs &betas, PSIs &psis, PDF &scale);

        virtual void _pass1(Reader &reader) = 0;
        virtual void _pass2(Reader &reader) = 0;
        virtual void _pass3(Reader &reader, Instances &instances) = 0;
      public:
        Config &cfg;
        Lexicon lexicon;
        TagSet tags;
        Attributes attributes;
        FeatureTypes &feature_types;
        Instances instances;
        Weights weights;
        Attribs2Weights attribs2weights;
        Model model;

        WordDict w_dict;

        const std::string preface;
        double inv_sigma_sq;
        double log_z;

        Impl(Config &cfg, FeatureTypes &types, const std::string &preface)
          : Util::Shared(), cfg(cfg), lexicon(cfg.lexicon()), tags(cfg.tags()),
            attributes(), feature_types(types), instances(), weights(),
            attribs2weights(),
            model("info", "Tagger model info file", cfg.model),
            w_dict(lexicon), preface(preface), inv_sigma_sq(), log_z(0.0) { }

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
          Model model("info", "model info", cfg.model);
          reg();
          _load_model(model);
        }

        virtual void tag(Reader &reader, Writer &writer) = 0;

        virtual void train(Reader &reader);
        virtual void extract(Reader &reader, Instances &instances);

        virtual void finite_differences(lbfgsfloatval_t *g, bool overwrite=false);

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
