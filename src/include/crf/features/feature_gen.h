namespace NLP {
  namespace CRF {
    class FeatureGen {
      public:
        FeatureGen(const Type &type) : type(type) { }
        virtual ~FeatureGen(void) { }
        virtual void operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j) = 0;
        virtual void operator()(Attributes &attributes, Sentence &sent, Context &c, int j) = 0;

        const Type &type;
    };

    class OffsetGen : public FeatureGen {
      public:
        const int offset;
        OffsetGen(const Type &type, const int offset) :
          FeatureGen(type), offset(offset) { }
    };

    class WordGen : public FeatureGen {
      public:
        WordGen(const Type &type);
        virtual ~WordGen(void) { }

        virtual void operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j);
        virtual void operator()(Attributes &attributes, Sentence &sent, Context &c, int j);

    };

    class PosGen : public FeatureGen {
      public:
        PosGen(const Type &type);
        virtual ~PosGen(void) { }

        virtual void operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j);
        virtual void operator()(Attributes &attributes, Sentence &sent, Context &c, int j);

    };

    class OffsetWordGen : public OffsetGen {
      public:
        OffsetWordGen(const Type &type, const int offset);
        virtual ~OffsetWordGen(void) { }

        virtual void operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j);
        virtual void operator()(Attributes &attributes, Sentence &sent, Context &c, int j);

    };

    class OffsetPosGen : public OffsetGen {
      public:
        OffsetPosGen(const Type &type, const int offset);
        virtual ~OffsetPosGen(void) { }

        virtual void operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j);
        virtual void operator()(Attributes &attributes, Sentence &sent, Context &c, int j);

    };

  }
}
