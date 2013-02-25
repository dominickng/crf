/**
 * tagger.h: common functionality for CRF training and tagging
 *
 */

namespace config = Util::config;

namespace NLP {
  namespace CRF {
    /**
     * Tagger class. Base class for conditional random field taggers.
     * Implements the basic training routines, including L-BFGS and
     * SGD optimization, for CRFs. All of the functionality is contained in
     * the private Tagger::Impl class
     *
     */
    class Tagger {
      public:
        /**
         * Tagger config class. Base class for commmon parameters to be read
         * from the command line when instantiating a Tagger object
         *
         */
        class Config : public config::OpGroup {
          public:
            config::OpPath model;
            config::OpPath lexicon;
            config::OpPath tags;
            config::OpPath attributes;
            config::OpPath features;
            config::OpPath weights;
            config::Op<double> sigma;
            config::Op<double> eta;
            config::Op<double> delta;

            config::Op<uint64_t> batch;
            config::Op<uint64_t> period;
            config::Op<uint64_t> niterations;

            config::Op<uint64_t> cutoff_default;
            config::Op<uint64_t> cutoff_words;
            config::Op<uint64_t> cutoff_attribs;
            config::Op<uint64_t> rare_cutoff;

            Config(const std::string &name, const std::string &desc,
                double sigma, uint64_t niterations)
              : config::OpGroup(name, desc, true),
            model(*this, "model", "location of the model directory", true),
            lexicon(*this, "lexicon", "location to save the lexicon file", "//lexicon", true, &model),
            tags(*this, "tags", "location to save the tag file", "//tags", true, &model),
            attributes(*this, "attributes", "location to save the attributes file", "//attributes", true, &model),
            features(*this, "features", "location to save the features file", "//features", true, &model),
            weights(*this, "weights", "location to save the weights file", "//weights", true, &model),
            sigma(*this, "sigma", "sigma value for L2 regularization", sigma, true),
            eta(*this, "eta", "eta calibration value for SGD (ignored for L-BFGS)", 0.1, true, true),
            delta(*this, "delta", "SGD optimization converges when log-likelihood improvement over the last (period) iterations is no larger than this value (ignored for L-BFGS)", 1e-6, true, true),
            batch(*this, "batch", "batch size for SGD optimization (ignored for L-BFGS)", 1, true, true),
            period(*this, "period", "period size for checking SGD convergence (ignored for L-BFGS)", 10, true, true),
            niterations(*this, "niterations", "number of training iterations", niterations, true),
            cutoff_default(*this, "cutoff_default", "minimum frequency cutoff for features", 1, true, true),
            cutoff_words(*this, "cutoff_words", "minimum frequency cutoff for word features", 1, true, true),
            cutoff_attribs(*this, "cutoff_attribs", "minimum frequency cutoff for attributes", 1, true, true),
            rare_cutoff(*this, "rare_cutoff", "cutoff to apply rare word features", 5, true, true)
          { }

            virtual ~Config(void) { /* nothing */ }
        };

        /**
         * Model config class. Stores information about a trained CRF model
         *
         */
        class Model : public config::Info {
          public:
            config::Op<uint64_t> nattributes;
            config::Op<uint64_t> nfeatures;
            config::Op<uint64_t> max_size;
            Model(const std::string &name, const std::string &desc, const config::OpPath &base)
              : config::Info(name, desc, base),
              nattributes(*this, "nattributes", "the number of attributes", 0),
              nfeatures(*this, "nfeatures", "the number of features", 0),
              max_size(*this, "max_size", "the size of the largest sentence", 0)
          { }

            virtual ~Model(void) { }
        };

      protected:
        class Impl;
        Impl *_impl;

        Tagger(Tagger::Config &cfg, const std::string &preface, Impl *impl);
        Tagger(const Tagger &other);
        virtual ~Tagger(void) { release(_impl); }
    };

    /**
     * Private implementation for the Tagger class.
     */
    class Tagger::Impl : public Util::Shared {
      protected:
        typedef std::vector<Contexts *> InstancePtrs; //for SGD

        virtual void reset(const size_t size);

        virtual void compute_psis(Context &context, PDFs &dist, double decay=1.0);
        virtual void compute_psis(Contexts &contexts, PSIs &psis, double decay=1.0);
        void compute_expectations(Contexts &c);
        virtual void forward(Contexts &contexts, PDFs &alphas, PSIs &psis, PDF &scale);
        virtual void forward_noscale(Contexts &contexts, PDFs &alphas, PSIs &psis);
        virtual void backward(Contexts &contexts, PDFs &betas, PSIs &psis, PDF &scale);
        virtual void backward_noscale(Contexts &contexts, PDFs &betas, PSIs &psis);
        virtual double sum_llhood(Contexts &contexts, double decay=1.0);
        virtual lbfgsfloatval_t regularised_llhood(void);
        lbfgsfloatval_t _lbfgs_evaluate(const lbfgsfloatval_t *x,
            lbfgsfloatval_t *g, const int n, const lbfgsfloatval_t step);

        virtual void print_psis(Contexts &contexts, PSIs &psis);
        virtual void print_fwd_bwd(Contexts &contexts, PDFs &pdfs, PDF &scale);

        virtual void finite_differences(lbfgsfloatval_t *g, bool overwrite=false);

        virtual double calibrate(InstancePtrs &instance_ptrs, double *weights, double lambda, double initial_eta, const size_t n);
        virtual double sgd_iterate(InstancePtrs &instance_ptrs, double *weights, const int n, const int nsamples, const double t0, const double lambda, const int nepochs, const int period, bool calibration);
        void compute_marginals(Contexts &c, double decay=1.0);
        void compute_weights(Contexts &c, double gain);
        virtual double score(Contexts &contexts, double decay=1.0);
        virtual double score_instance(Contexts &contexts, double decay=1.0, double gain=1.0);

        virtual void _pass1(Reader &reader) = 0;
        virtual void _pass2(Reader &reader) = 0;
        virtual void _pass3(Reader &reader, Instances &instances) = 0;

        virtual void train_lbfgs(Reader &reader, double *weights);
        virtual void train_sgd(Reader &reader, double *weights);

        virtual void reg(void);
        virtual void load(void);
        virtual void _load_model(Model &model);
        virtual void _read_weights(Model &model);
        virtual void _read_attributes(Model &model);

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
        TransDict t_dict;

        const std::string preface;
        double inv_sigma_sq;
        double log_z;
        uint64_t ntags;

        /**
         * working vectors for training
         * alphas: an (nwords * ntags) matrix that stores the forward scores for
         *         one pass of the forward-backward algorithm
         * betas: an (nwords * ntags) matrix that stores the backward scores for
         *        one pass of the forward-backward algorithm
         *
         * state_marginals: an (nwords * ntags) matrix that stores the model
         *                  expectation of a tag t at position i
         *
         * trans_marginals: an (ntags * ntags) matrix that stores the model
         *                  expectation of a transition between tag t and tag u
         *
         * psis: an (nwords * ntags * ntags) matrix that stores the activation
         *       score (psi) for a transition from tag t to tag u at position i.
         *       State activations for features that do not rely on the
         *       previous tag are stored with a previous tag of None
         *
         * scale: an (nwords) vector that stores the scale factor for each
         *        position i.
         */
        PDFs alphas;
        PDFs betas;
        PDFs state_marginals;
        PDFs trans_marginals;
        PSIs psis;
        PDF scale;

        Impl(Config &cfg, Types &types, const std::string &preface)
          : Util::Shared(), cfg(cfg), types(types),
            model("info", "Tagger model info file", cfg.model),
            registry(cfg.rare_cutoff()),
            lexicon(cfg.lexicon()), tags(cfg.tags()),
            attributes(), instances(), weights(), attribs2weights(),
            w_dict(lexicon), ww_dict(lexicon), a_dict(), t_dict(),
            preface(preface), inv_sigma_sq(), log_z(0.0),
            ntags(), alphas(), betas(), state_marginals(), trans_marginals(),
            psis(), scale() { }

        virtual ~Impl(void) { /* nothing */ }

        virtual void extract(Reader &reader, Instances &instances);
        virtual void train(Reader &reader, const std::string &trainer);

        static lbfgsfloatval_t lbfgs_evaluate(void *instance,
            const lbfgsfloatval_t *x, lbfgsfloatval_t *g, const int n,
            const lbfgsfloatval_t step);

        static int lbfgs_progress(void *instance, const lbfgsfloatval_t *x,
            const lbfgsfloatval_t *g, const lbfgsfloatval_t fx,
            const lbfgsfloatval_t xnorm, const lbfgsfloatval_t gnorm,
            const lbfgsfloatval_t step, int n, int k, int ls);

        virtual void run_tag(Reader &reader, Writer &writer) = 0;
        virtual void tag(State &state, Sentence &sent) = 0;

    };

  }
}
