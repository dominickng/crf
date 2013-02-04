namespace NLP {
  namespace HT = Util::hashtable;

  class TagSet {
    private:
      class Impl;
      Impl *_impl;

    public:
      TagSet(void);
      TagSet(const std::string &filename);
      TagSet(const std::string &filename, std::istream &input);
      TagSet(const TagSet &other);

      TagSet &operator=(const TagSet &other);

      void add(const std::string &raw, const uint64_t freq=1);
      void insert(const std::string &raw, const uint64_t freq=1);

      void load(void);
      void load(const std::string &filename, std::istream &input);
      void save(const std::string &preface);
      void save(const std::string &filename, const std::string &preface);
      void save(std::ostream &out, const std::string &preface);

      const Tag canonize(const std::string &raw) const;
      const Tag canonize(const char *raw) const;
      void canonize(const Raws &raws, Tags &tags) const;

      const char *str(const Tag &tag) const;
      void str(const Tags &tags, Raws &raws) const;

      const Tag operator[](const std::string &raw) const { return canonize(raw); }
      const Tag operator[](const char *raw) const { return canonize(raw); }
      const char *operator[](const Tag &tag) const { return str(tag); }

      size_t size(void) const;
      size_t index(TagPair &tp) const;

      size_t npairs(void) const { return size() * size(); };
  };

}
