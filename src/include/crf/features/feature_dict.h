namespace NLP {
  namespace CRF {

    class FeatureDict {
      public:
        FeatureDict(void) { }
        virtual ~FeatureDict(void) { };

        virtual Attribute &load(const Type &type, std::istream &in) = 0;
    };

    class WordDict : public FeatureDict {
      public:
        WordDict(const Lexicon lexicon, const size_t nbuckets=HT::MEDIUM,
        const size_t pool_size=HT::LARGE);
        WordDict(const WordDict &other);
        virtual ~WordDict(void);

        virtual Attribute &load(const Type &type, std::istream &in);
        Attribute get(const Type &type, Raw &raw);
        Attribute &insert(const Type &type, Raw &raw);

      private:
        class Impl;
        Impl *_impl;
    };

    class TagDict : public FeatureDict {
      public:
        TagDict(const TagSet tags)
          : tags(tags), attributes() { }
        virtual ~TagDict(void) { };

        virtual Attribute &load(const Type &type, std::istream &in) {
          if (!attributes.size())
            attributes.resize(tags.size());
          Raw value;
          in >> value;
          return insert(type.name, value);
        }

        Attribute get(const Type &type, Raw &raw) {
          return attributes[tags[raw]];
        }

        Attribute &insert(const std::string &type, Raw &raw) {
          return attributes[tags[raw]];
        }

      private:
        const TagSet tags;
        std::vector<Attribute> attributes;
    };
  }
}
