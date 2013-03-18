/**
 * tagger.h: common functionality for conditional random field (CRF) training
 * and tagging
 *
 * A CRF is a kind of undirected graphical model commonly used for sequence
 * tagging. The unobserved variables in the model (tags) depend only upon the
 * entire observation sequence (e.g. a sentence), and in a linear-chain CRF,
 * some fixed Markov window of previous tags. In this implementation, we use a
 * Markov window of 1 previous tag.
 *
 * We can describe the conditional probability of a sequence of tags Y given
 * an observation sequence X of length T as follows:
 *
 * p(Y|X,theta) = ( (1 / Z(X)) * product [t from 1 to T] (psi(y(t-1), y(t), theta, X)))
 *
 *  Z(X) is the log partition function (normalising constant)
 *    Z(X) = sum [over all possible tag sequences Y] (
 *             product [from 1 to T] (
 *              psi(y(t), y(t-1), theta, X)
 *             )
 *           )
 *  y(t) is the tag at position t in the sequence
 *  y(t-1) is the tag preceding y(t)
 *  psi(y(t), y(t-1), theta, X)
 *        = exp(sum [k from 1 to K] (theta(k) * f(y(t-1), y(t), X)))
 *    for k feature functions that extract values from the observation sequence.
 *    Each feature has a corresponding real-valued weight that is represented
 *    here by the K-vector theta.
 *    Features may depend on the current tag y(t), the previous tag y(t-1), both
 *    of them, or neither of them. State features depend only on y(t), and
 *    transition features depend on y(t) and y(t-1).
 *
 *    The result of the psi function is also referred to as the
 *    activation value at position t between tags y(t-1) and y(t). This is the
 *    typically log-linear format common in many machine learning techniques
 *
 * The training process estimates the optimal theta values. Given fully
 * observed training data of N (X, Y) pairs, the process is supervised, and
 * like many machine learning models, CRFs are trained by optimizing the log
 * likelihood given training data (which is a convex objective). The log
 * likelihood is the sum of log probabilities of the gold sequences X given the
 * corresponding observation X and parameters theta:
 *
 * llhood(theta) = sum [i from 1 to N] (log P(Y(i) | X(i), theta))
 *
 * Substituting in the definition of the conditional probability from above,
 * and adding an L2 regularization term to penalise weights that become too
 * large, we have:
 *
 * llhood(theta) = sum [i from 1 to N] (
 *                    sum [t from 1 to T] (
 *                      sum [k from 1 to K] (
 *                        theta(k) * f(y(i)(t-1), y(i)(t), X(i))
 *                      )
 *                    )
 *                 )
 *                 -
 *                 sum [i from 1 to N] (log Z(X(i)))
 *                 -
 *                 sum [k from 1 to K] (theta(k)^2 / (2 * sigma^2))
 *
 *              where sigma is a parameter that indicates the strength of the
 *              regularization.
 *
 * The first component of the log likelihood is simply computed by iterating
 * over the training data and summing the appropriate activation values. The
 * third component is also trivially calculated by summing the square of the
 * current weights. Unfortunately, the second term, which corresponds to the sum
 * of log partition factors for each instance, is exponential in the
 * number of possible tag sequences. Thankfully, in linear-chain CRFs, we can
 * use a variant of the forward-backward (or Baum-Welch) algorithm used for
 * training Hidden Markov Models (HMMs) to efficiently calculate the log Z term.
 *
 * Forward-backward performs two passes over each training instance. The
 * forward pass computes the score of reaching tag t at position i given
 * the scores of every tag p at position i-1. This is done by summing over all
 * previous tags p, the score of a tag p at position i-1 multipled by the
 * transition score to move from tag p to tag t at the current position.
 * The backward pass analagously computes the score of reaching tag t at
 * position i given the scores of every tag n at position i+1. Then the forward
 * and backward scores can be combined to efficiently estimate marginal
 * probabilities of tag sequences and the log partition factor.
 *
 * The llhood objective typically does not have a closed form solution, so
 * iterative methods must be employed for optimization. These methods compute
 * the derivative (gradient) of each component of the theta weight vector, and
 * adjust the values of the components so to make the gradients go to 0 (a
 * stationary point, and since the objective is convex, this is a unique optima
 * that maximises log likelihood). Two popular techinques are L-BFGS and
 * stochastic gradient descent (SGD).
 *
 * BFGS is a second-order Newton method. It uses the Hessian matrix of second
 * derivatives to compute the curvature of the likelihood, making it faster
 * than vanilla steepest descent along the negative gradient direction. The size
 * of the Hessian is quadratic in the size of the theta vector, so it is
 * impractical to directly compute it. Instead, BFGS approximates the Hessian
 * using only the first derivatives of the objectvive function. This full
 * approximation still requires quadratic space, so limited-memory BFGS
 * (L-BFGS) that keeps only a fixed window of derivative histories to estimate
 * the Hessian is typically used. At each iteration, L-BFGS requires the value
 * of the objective function and the gradient of each component of the theta
 * vector.
 *
 * Computing the objective value and gradients for L-BFGS requires a full pass
 * through the training data before any updates are made - i.e. batch
 * optimization. However, if the training examples are drawn iid from the same
 * distribution, it seems wasteful to require seeing all of the training
 * examples before updating the feature weights. Stochastic gradient descent
 * uses this insight: the basic idea is pick training instances at random, and
 * take little steps in the direction indicated by the gradient of the chosen
 * instance only. This can be computed much faster than an L-BFGS update, and
 * while each individual update may not be globally optimal, the randomness
 * helps to achieve convergence at much faster rates than L-BFGS. The rate of
 * descent is controlled by a learning rate that may require tuning based on
 * the size of the problem. The learning rate decreases as optimization
 * continues to help ensure that the objective is not overshot.
 *
 * A variant of the Viterbi algorithm used in HMMs is used at tagging time.
 * Viterbi is very much like the forward pass of the forward-backward algorithm,
 * except that the sum operation is replaced by a maximization, and backpointers
 * to each previous maximising state are stored. The end state with the highest
 * score is then traced backwards to yield the most probable sequence given
 * the model parameters.
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
            config::OpPath tagdict;
            config::OpPath attributes;
            config::OpPath features;
            config::OpPath weights;
            config::OpPath log;
            config::Op<lbfgsfloatval_t> sigma;
            config::Op<lbfgsfloatval_t> eta;
            config::Op<lbfgsfloatval_t> delta;

            config::Op<uint64_t> batch;
            config::Op<uint64_t> period;
            config::Op<uint64_t> niterations;

            config::Op<uint64_t> cutoff_default;
            config::Op<uint64_t> cutoff_words;
            config::Op<uint64_t> cutoff_attribs;
            config::Op<uint64_t> rare_cutoff;

            Config(const std::string &name, const std::string &desc,
                lbfgsfloatval_t sigma, uint64_t niterations)
              : config::OpGroup(name, desc, true),
            model(*this, "model", "location of the model directory", true),
            lexicon(*this, "lexicon", "location to save the lexicon file", "//lexicon", true, &model),
            tags(*this, "tags", "location to save the tag file", "//tags", true, &model),
            tagdict(*this, "tagdict", "location to save the tag dictionary file", "//tagdict", true, &model),
            attributes(*this, "attributes", "location to save the attributes file", "//attributes", true, &model),
            features(*this, "features", "location to save the features file", "//features", true, &model),
            weights(*this, "weights", "location to save the weights file", "//weights", true, &model),
            log(*this, "log", "location to save the training log file", "//log", true, &model),
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
        typedef std::string Chains;

        lbfgsfloatval_t duration_s(void);
        lbfgsfloatval_t duration_m(void);

        virtual void reset(const size_t size);

        void compute_psis(Context &context, PDFs &dist, lbfgsfloatval_t decay=1.0);
        void compute_psis(Contexts &contexts, PSIs &psis, lbfgsfloatval_t decay=1.0);
        void compute_expectations(Contexts &c);
        void forward(Contexts &contexts, PDFs &alphas, PSIs &psis, PDF &scale);
        void forward_noscale(Contexts &contexts, PDFs &alphas, PSIs &psis);
        void backward(Contexts &contexts, PDFs &betas, PSIs &psis, PDF &scale);
        void backward_noscale(Contexts &contexts, PDFs &betas, PSIs &psis);
        lbfgsfloatval_t sum_llhood(Contexts &contexts, lbfgsfloatval_t decay=1.0);
        lbfgsfloatval_t regularised_llhood(void);
        lbfgsfloatval_t _lbfgs_evaluate(const lbfgsfloatval_t *x,
            lbfgsfloatval_t *g, const int n, const lbfgsfloatval_t step);

        void print_psis(Contexts &contexts, PSIs &psis);
        void print_fwd_bwd(Contexts &contexts, PDFs &pdfs, PDF &scale);

        void finite_differences(lbfgsfloatval_t *g, bool overwrite=false);

        lbfgsfloatval_t calibrate(InstancePtrs &instance_ptrs, lbfgsfloatval_t *weights,
            lbfgsfloatval_t lambda, lbfgsfloatval_t initial_eta, const int nfeatures);
        lbfgsfloatval_t sgd_epoch(InstancePtrs &instance_ptrs, lbfgsfloatval_t *weights,
            const int nfeatures, const int nsamples, const lbfgsfloatval_t lambda,
            const int t0, int &t, const bool log=false);
        lbfgsfloatval_t sgd_iterate_calibrate(InstancePtrs &instance_ptrs,
            lbfgsfloatval_t *weights, const int nfeatures, const int nsamples,
            const lbfgsfloatval_t t0, const lbfgsfloatval_t lambda);
        lbfgsfloatval_t sgd_iterate(InstancePtrs &instance_ptrs, lbfgsfloatval_t *weights,
            const int nfeatures, const int nsamples, const lbfgsfloatval_t t0,
            const lbfgsfloatval_t lambda, const int nepochs, const int period);
        void compute_marginals(Contexts &c, lbfgsfloatval_t decay=1.0);
        void compute_weights(Contexts &c, lbfgsfloatval_t gain);
        lbfgsfloatval_t score(Contexts &contexts, lbfgsfloatval_t decay=1.0);
        lbfgsfloatval_t score_instance(Contexts &contexts, lbfgsfloatval_t decay=1.0,
            lbfgsfloatval_t gain=1.0);

        virtual void _pass1(Reader &reader) = 0;
        virtual void _pass2(Reader &reader) = 0;
        virtual void _pass3(Reader &reader, Instances &instances) = 0;

        void train_lbfgs(Reader &reader, lbfgsfloatval_t *weights);
        void train_sgd(Reader &reader, lbfgsfloatval_t *weights);

        virtual void reg(void);
        virtual void load(void);
        virtual void _load_model(Model &model);
        void _read_weights(Model &model);
        void _read_attributes(Model &model);

      public:
        Config &cfg;
        Types &types;
        Model model;
        Registry registry;
        Log logger;
        Chains chains;

        Lexicon lexicon;
        TagSet tags;
        TagLimits limits;
        Lexicon words2tags;
        Attributes attributes;
        Instances instances;
        Weights weights;
        Attribs2Weights attribs2weights;

        WordDict w_dict;
        BiWordDict ww_dict;
        AffixDict a_dict;
        TransDict t_dict;

        const std::string preface;
        lbfgsfloatval_t inv_sigma_sq;
        lbfgsfloatval_t log_z;
        uint64_t ntags;
        clock_t clock_begin;

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

        Impl(Config &cfg, Types &types, const std::string &chains,
            const std::string &preface)
          : Util::Shared(), cfg(cfg), types(types),
            model("info", "Tagger model info file", cfg.model),
            registry(cfg.rare_cutoff()), logger(cfg.log(), std::cout),
            chains(Format(chains, true).fields), lexicon(cfg.lexicon()),
            tags(cfg.tags()), limits(tags), words2tags(cfg.tagdict()),
            attributes(), instances(), weights(), attribs2weights(),
            w_dict(lexicon), ww_dict(lexicon), a_dict(), t_dict(),
            preface(preface), inv_sigma_sq(), log_z(0.0), ntags(),
            clock_begin(), alphas(), betas(), state_marginals(),
            trans_marginals(), psis(), scale() { }

        virtual ~Impl(void) { /* nothing */ }

        void extract(Reader &reader, Instances &instances);
        void train(Reader &reader, const std::string &trainer);

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
