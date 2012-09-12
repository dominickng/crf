namespace NLP {
  namespace HT = Util::hashtable;

  class Lexicon {
    private:
      class Impl;
      Impl *_impl;

    public:
      Lexicon(const size_t nbuckets=HT::MEDIUM, const size_t pool_size=HT::LARGE);
      Lexicon(const std::string &filename, const size_t nbuckets=HT::MEDIUM,
          const size_t pool_size=HT::LARGE);
      Lexicon(const std::string &filename, std::istream &input,
          const size_t nbuckets=HT::MEDIUM, const size_t pool_size=HT::LARGE);
      Lexicon(const Lexicon &other);

      void add(const std::string &raw, const uint64_t freq=1);
      void insert(const std::string &raw, const uint64_t freq=1);

      void load(const std::string &filename);
      void load(const std::string &filename, std::istream &input);
      void save(const std::string &filename, const std::string &preface);
      void save(std::ostream &out, const std::string &preface);

      const Word canonize(const std::string &raw) const;
      const Word canonize(const char *raw) const;
      void canonize(const Raws &raws, Words &words) const;

      const char *str(const Word &word) const;
      void str(const Words &words, Raws &raws) const;

      const Word operator[](const std::string &raw) const { return canonize(raw); }
      const Word operator[](const char *raw) const { return canonize(raw); }
      const char *operator[](const Word &word) const { return str(word); }

      void sort_by_freq(void);

      size_t size(void) const;
  };

}
