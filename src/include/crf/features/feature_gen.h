namespace NLP {
  namespace CRF {
    class FeatureGen {
      protected:
        const Type &type;

      public:
        FeatureGen(const Type &type) : type(type) { }
        virtual ~FeatureGen(void) { }
        virtual void operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j) = 0;
        virtual void operator()(Attributes &attributes, Sentence &sent, Context &c, int j) = 0;
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

    class PrevWordGen : public FeatureGen {
      public:
        PrevWordGen(const Type &type);
        virtual ~PrevWordGen(void) { }

        virtual void operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j);
        virtual void operator()(Attributes &attributes, Sentence &sent, Context &c, int j);

    };

    class NextWordGen : public FeatureGen {
      public:
        NextWordGen(const Type &type);
        virtual ~NextWordGen(void) { }

        virtual void operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j);
        virtual void operator()(Attributes &attributes, Sentence &sent, Context &c, int j);

    };

    class PrevPosGen : public FeatureGen {
      public:
        PrevPosGen(const Type &type);
        virtual ~PrevPosGen(void) { }

        virtual void operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j);
        virtual void operator()(Attributes &attributes, Sentence &sent, Context &c, int j);

    };

    class NextPosGen : public FeatureGen {
      public:
        NextPosGen(const Type &type);
        virtual ~NextPosGen(void) { }

        virtual void operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j);
        virtual void operator()(Attributes &attributes, Sentence &sent, Context &c, int j);

    };

  }
}
