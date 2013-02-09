namespace NLP {
  namespace CRF {
    namespace HT = Util::hashtable;

    class Attributes {
      private:
        class Impl;
        Impl *_impl;

      public:
        Attributes(const size_t nbuckets=HT::MEDIUM,
            const size_t pool_size=HT::LARGE);
        Attributes(const std::string &filename, const size_t nbuckets=HT::MEDIUM,
            const size_t pool_size=HT::LARGE);
        Attributes(const std::string &filename, std::istream &input,
            const size_t nbuckets=HT::MEDIUM, const size_t pool_size=HT::LARGE);
        Attributes(const Attributes &other);

        ~Attributes(void);

        void load(const std::string &filename);
        void load(const std::string &filename, std::istream &input);
        void save_attributes(const std::string &filename, const std::string &preface);
        void save_attributes(std::ostream &out, const std::string &preface);
        void save_features(const std::string &filename, const std::string &preface);
        void save_features(std::ostream &out, const std::string &preface);
        void save_weights(const std::string &filename, const std::string &preface);
        void save_weights(std::ostream &out, const std::string &preface);

        void operator()(const char *type, const std::string &str, TagPair &tp);
        void operator()(const char *type, const std::string &str, uint64_t &id);
        void operator()(const char *type, const std::string &str, Context &c);
        void sort_by_freq(void);
        void reset_estimations(void);

        uint64_t nfeatures(void) const;

        double sum_lambda_sq(void);
        void copy_lambdas(const lbfgsfloatval_t *x);
        void copy_gradients(lbfgsfloatval_t *x, double inv_sigma_sq);
        bool inc_next_gradient(double val);
        bool dec_gradient(double val);
        void print_current_gradient(double val, double inv_sigma_sq);
        void print(double inv_sigma_sq);
        void prep_finite_differences(void);

        size_t size(void) const;
    };
  }

}
