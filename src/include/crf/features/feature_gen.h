namespace NLP {
  namespace CRF {
    class FeatureGen {
      protected:
        const Type &type;

      public:
        FeatureGen(const Type &type) : type(type) { }
        virtual ~FeatureGen(void) { }
        virtual void operator()(Attributes &attributes, Sentence &sent, TagPair &tp, int j) = 0;
        virtual void operator()(Attributes &attributes, Sentence &sent, Context &c, int j) = 0;
    };

    class WordGen : public FeatureGen {
      public:
        WordGen(const Type &type);
        virtual ~WordGen(void) { }

        virtual void operator()(Attributes &attributes, Sentence &sent, TagPair &tp, int j);
        virtual void operator()(Attributes &attributes, Sentence &sent, Context &c, int j);

    };
  }
}
