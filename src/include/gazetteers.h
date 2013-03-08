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
  typedef uint64_t GazFlags;

  class Gazetteers {
    private:
      class Impl;
      Impl *_impl;

    public:
      typedef std::vector<std::string> GazNames;

      // for common words
      const static GazFlags COMMON = 1 << 0;
      // first names
      const static GazFlags FIRST = 1 << 1;
      //last names
      const static GazFlags LAST = 1 << 2;

      //conll gazetteers
      const static GazFlags CONLL_LOC = 1 << 3;
      const static GazFlags CONLL_MISC = 1 << 4;
      const static GazFlags CONLL_ORG = 1 << 5;
      const static GazFlags CONLL_PER = 1 << 6;
      const static GazFlags CONLL_ILOC = 1 << 7;
      const static GazFlags CONLL_IMISC = 1 << 8;
      const static GazFlags CONLL_IORG = 1 << 9;
      const static GazFlags CONLL_IPER = 1 << 10;

      Gazetteers(const size_t nbuckets=HT::LARGE, const size_t pool_size=HT::LARGE);
      Gazetteers(const std::string &dir, const std::string &config,
          const size_t nbuckets=HT::LARGE, const size_t pool_size=HT::LARGE);
      Gazetteers(const Gazetteers &other);

      ~Gazetteers(void);

      void add(const std::string &entry, const GazFlags flags);

      void load(const std::string &dir, const std::string &config);
      void load(const std::string &filename, const GazFlags flag);

      GazFlags exists(const std::string &str) const;
      GazFlags lower(const std::string &str) const;

      int gaz_index(const std::string &name) const;

      const std::string &gaz_name(const GazFlags flag) const;
      const GazNames &gaz_names(void) const;

      size_t size(void) const;
      void clear(void);
  };

}
