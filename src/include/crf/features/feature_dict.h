/**
 * feature_dict.h.
 * Defines the feature dictionary classes, which are used to store the mapping
 * between attributes and feature lambdas at tagging time.
 */
namespace NLP {
  namespace CRF {

    /**
     * FeatureDict.
     * The abstract base class for feature dictionaries. All FeatureDicts are
     * able to load attribute values from an istream. Subclasses of this
     * base class provide methods to get Attribute objects given a feature
     * value
     */
    class FeatureDict {
      public:
        FeatureDict(void) { }
        virtual ~FeatureDict(void) { };

        virtual Attribute &load(const Type &type, std::istream &in) = 0;
    };

    /**
     * AffixDict.
     * Class to support lookup of affix (prefix and suffix) features.
     */
    class AffixDict : public FeatureDict {
      public:
        AffixDict(const size_t nbuckets=HT::SMALL, const size_t pool_size=HT::MEDIUM);
        AffixDict(const AffixDict &other);
        virtual ~AffixDict(void);

        virtual Attribute &load(const Type &type, std::istream &in);
        Attribute get(const Type &type, const Raw &raw);
        Attribute &insert(const Type &type, const Raw &raw);

        void print_stats(std::ostream &out);

      private:
        class Impl;
        Impl *_impl;
    };

    /**
     * WordDict.
     * Class to support lookup of lexicon (word) features.
     */
    class WordDict : public FeatureDict {
      public:
        WordDict(const Lexicon lexicon, const size_t nbuckets=HT::MEDIUM,
        const size_t pool_size=HT::LARGE);
        WordDict(const WordDict &other);
        virtual ~WordDict(void);

        virtual Attribute &load(const Type &type, std::istream &in);
        Attribute get(const Type &type, const Raw &raw);
        Attribute &insert(const Type &type, const Raw &raw);

        void print_stats(std::ostream &out);

      private:
        class Impl;
        Impl *_impl;
    };

    /**
     * BiWordDict.
     * Class to support lookup of bigram lexicon (word) features.
     */
    class BiWordDict : public FeatureDict {
      public:
        BiWordDict(const Lexicon lexicon, const size_t nbuckets=HT::MEDIUM,
        const size_t pool_size=HT::LARGE);
        BiWordDict(const BiWordDict &other);
        virtual ~BiWordDict(void);

        virtual Attribute &load(const Type &type, std::istream &in);
        Attribute get(const Type &type, const Raw &raw1, const Raw &raw2);
        Attribute &insert(const Type &type, const Raw &raw1, const Raw &raw2);

      private:
        class Impl;
        Impl *_impl;
    };

    /**
     * TransDict.
     * Class to support lookup of HMM-style transition features from a
     * previous tag to a current tag.
     *
     * All transition features are independent of state, so they are simply
     * stored with a constant feature value. Thus, all transition features
     * map to a single Attribute, which is stored in this class.
     */
    class TransDict : public FeatureDict {
      public:
        TransDict(void) : _attrib() { }
        virtual ~TransDict(void) { }

        virtual Attribute &load(const Type &type, std::istream &in) {
          Raw val;
          in >> val;
          return _attrib;
        }

        Attribute get(const Type &type) {
          return _attrib;
        }

        Attribute &insert(const std::string &type) {
          return _attrib;
        }

      private:
        Attribute _attrib;
    };

    /**
     * TagDict.
     * Class to support lookup of tagset features. Implemented as a vector,
     * as tag objects are already canonized into an integer based
     * representation
     */
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

        Attribute get(const Type &type, const Raw &raw) {
          return attributes[tags[raw]];
        }

        Attribute &insert(const std::string &type, const Raw &raw) {
          return attributes[tags[raw]];
        }

      private:
        const TagSet tags;
        std::vector<Attribute> attributes;
    };

    /**
     * BiTagDict.
     * Class to support lookup of bigram tagset features. This is implemented
     * as a 2D vector, as each tag is canonized to an integer-based
     * representation.
     */
    class BiTagDict : public FeatureDict {
      public:
        BiTagDict(const TagSet tags)
          : tags(tags), attributes() { }
        virtual ~BiTagDict(void) { };

        virtual Attribute &load(const Type &type, std::istream &in) {
          if (!attributes.size())
            attributes.resize(tags.size() * tags.size());
          Raw val1, val2;
          in >> val1 >> val2;
          return insert(type.name, val1, val2);
        }

        Attribute get(const Type &type, const Raw &raw1, const Raw &raw2) {
          return attributes[tags[raw1] * tags.size() + tags[raw2]];
        }

        Attribute &insert(const std::string &type, const Raw &raw1, const Raw &raw2) {
          return attributes[tags[raw1] * tags.size() + tags[raw2]];
        }

      private:
        const TagSet tags;
        std::vector<Attribute> attributes;
    };
  }
}
