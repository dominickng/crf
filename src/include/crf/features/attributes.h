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

        void load(const std::string &filename);
        void load(const std::string &filename, std::istream &input);
        void save_attributes(const std::string &filename, const std::string &preface);
        void save_attributes(std::ostream &out, const std::string &preface);
        void save_features(const std::string &filename, const std::string &preface);
        void save_features(std::ostream &out, const std::string &preface);

        void operator()(const char *type, const std::string &str, TagPair &tp);
        void operator()(const char *type, const std::string &str, uint64_t &id);
        void sort_by_freq(void);

        size_t size(void) const;
    };
  }

}
