/**
 * feature_gen.h.
 *
 * Defines the feature generators used in the CRF. Feature generators are
 * responsible for extracting features from each position in a sentence,
 * accumulating pointers to the feature objects active at each context object,
 * and computing the probability distribution of features at tagging time.
 *
 */
namespace NLP {
  namespace CRF {
    /**
     * FeatureGen. Abstract base class for feature generators.
     */
    class FeatureGen {
      protected:
        void _add_features(Attribute attrib, PDFs &dist);

        // whether to add state and trans versions of features features
        const bool _add_state;
        const bool _add_trans;

      public:
        FeatureGen(const bool add_state, const bool add_trans)
          : _add_state(add_state), _add_trans(add_trans) { }
        virtual ~FeatureGen(void) { }

        /**
         * load.
         * Given an input stream, loads a feature of a given type and its
         * lambda value. Used to load the feature dictionaries prior to tagging
         */
        virtual Attribute &load(const Type &type, std::istream &in) = 0;

        /**
         * operator().
         * This version extracts the attribute value from the sentence at
         * position i, and inserts it into the attributes dictionary with
         * a feature for the observed tagpair. Called in the second pass
         * of feature extraction.
         */
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) = 0;

        /**
         * operator().
         * This version looks up the extracted attribute value in the populated
         * attributes dictionary, adding all matching features to the context
         * object that represents all active features at position i. This
         * prepares the data structures used for optimization. Called in
         * the third pass of feature extraction.
         */
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) = 0;

        /**
         * operator().
         * This version looks up all features matching the extracted attribute
         * value for the sentence at position i, and adds the lambdas of those
         * features to the probability distribution. The distribution is indexed
         * by prev_klass, curr_klass.
         */
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i) = 0;
    };

    class TransGen : public FeatureGen {
      public:
        TransGen(TransDict &dict, const bool add_state, const bool add_trans);
        virtual ~TransGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        TransDict &dict;
        const static std::string name;
        bool trans_loaded;
    };

    class OffsetGen : public FeatureGen {
      protected:
        const Raw *_get_raw(Raws &raws, int i);

      public:
        const int offset;
        OffsetGen(const int offset, const bool add_state=true, const bool add_trans=true) :
          FeatureGen(add_state, add_trans), offset(offset) { }

    };

    class WordGen : public FeatureGen {
      public:
        WordGen(WordDict &dict, const bool add_state, const bool add_trans);
        virtual ~WordGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        WordDict &dict;
    };

    class PrefixGen : public FeatureGen {
      public:
        PrefixGen(AffixDict &dict, const bool add_state, const bool add_trans);
        virtual ~PrefixGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        AffixDict &dict;
    };

    class SuffixGen : public FeatureGen {
      public:
        SuffixGen(AffixDict &dict, const bool add_state, const bool add_trans);
        virtual ~SuffixGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        AffixDict &dict;
    };

    class ShapeGen : public FeatureGen {
      public:
        ShapeGen(AffixDict &dict, const bool add_state, const bool add_trans);
        virtual ~ShapeGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        AffixDict &dict;
        Shape shape;
    };

    class OffsetShapeGen : public OffsetGen {
      public:
        OffsetShapeGen(AffixDict &dict, const int offset, const bool add_state, const bool add_trans);
        virtual ~OffsetShapeGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        AffixDict &dict;
        Shape shape;
    };

    class PosGen : public FeatureGen {
      public:
        PosGen(TagDict &dict, const bool add_state, const bool add_trans);
        virtual ~PosGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        TagDict &dict;

    };

    class OffsetWordGen : public OffsetGen {
      public:
        OffsetWordGen(WordDict &dict, const int offset, const bool add_state, const bool add_trans);
        virtual ~OffsetWordGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        WordDict &dict;
    };

    class OffsetPosGen : public OffsetGen {
      public:
        OffsetPosGen(TagDict &dict, const int offset, const bool add_state, const bool add_trans);
        virtual ~OffsetPosGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        TagDict &dict;
    };

    class BigramGen : public OffsetGen {
      protected:
        void _get_raw(Raws &raws, Raw &raw, int i);

      public:
        BigramGen(const int offset, const bool add_state, const bool add_trans) : OffsetGen(offset, add_state, add_trans) { }
        virtual ~BigramGen(void) { }
    };

    class BigramWordGen : public BigramGen {
      public:
        BigramWordGen(BiWordDict &dict, const int offset, const bool add_state, const bool add_trans);
        virtual ~BigramWordGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        BiWordDict &dict;
    };

    class BigramPosGen : public BigramGen {
      public:
        BigramPosGen(BiTagDict &dict, const int offset, const bool add_state, const bool add_trans);
        virtual ~BigramPosGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        BiTagDict &dict;
    };

    class MorphGen : public FeatureGen {
      public:
        MorphGen(BinDict &dict, const bool add_state, const bool add_trans);
        virtual ~MorphGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        BinDict &dict;
    };


  }
}
