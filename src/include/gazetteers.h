/**
 * gazetteers.h
 * Defines the gazetteer interface.
 *
 * This class maps words to a 64 bit integer that represents the gazetteer
 * source id. In this way, information from up to 64 different gazetteers can
 * be efficiently represented.
 */
namespace NLP {
  namespace HT = Util::hashtable;

  class Gazetteers {
    private:
      class Impl;
      Impl *_impl;

    public:
      // for common words
      const static uint64_t COMMON = 1 << 0;
      // first names
      const static uint64_t FIRST = 1 << 1;
      //last names
      const static uint64_t LAST = 1 << 2;

      //conll gazetteers
      const static uint64_t CONLL_LOC = 1 << 3;
      const static uint64_t CONLL_MISC = 1 << 4;
      const static uint64_t CONLL_ORG = 1 << 5;
      const static uint64_t CONLL_PER = 1 << 6;
      const static uint64_t CONLL_ILOC = 1 << 7;
      const static uint64_t CONLL_IMISC = 1 << 8;
      const static uint64_t CONLL_IORG = 1 << 9;
      const static uint64_t CONLL_IPER = 1 << 10;

      typedef std::vector<std::string> GazNames;

      Gazetteers(const size_t nbuckets=HT::LARGE, const size_t pool_size=HT::LARGE);
      Gazetteers(const std::string &dir, const std::string &config,
          const size_t nbuckets=HT::LARGE, const size_t pool_size=HT::LARGE);
      Gazetteers(const Gazetteers &other);

      ~Gazetteers(void);

      void add(const std::string &entry, const uint64_t flags);

      void load(const std::string &dir, const std::string &config);
      void load(const std::string &filename, const uint64_t flag);

      uint64_t exists(const std::string &str) const;
      uint64_t lower(const std::string &str) const;

      int gaz_index(const std::string &name) const;

      const std::string &gaz_name(const uint64_t flag) const;
      const GazNames &gaz_names(void) const;

      size_t size(void) const;
      void clear(void);
  };

}
