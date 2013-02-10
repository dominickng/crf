namespace NLP {
  namespace CRF {
    class FeatureGen {
      protected:
        void _add_features(Attribute attrib, PDFs &dist);

      public:
        FeatureGen(void) { }
        virtual ~FeatureGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in) = 0;

        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) = 0;
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) = 0;
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i) = 0;
    };

    class OffsetGen : public FeatureGen {
      protected:
        const Raw *_get_raw(Raws &raws, int i);

      public:
        const int offset;
        OffsetGen(const int offset) :
          FeatureGen(), offset(offset) { }

    };

    class WordGen : public FeatureGen {
      public:
        WordGen(WordDict &dict);
        virtual ~WordGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        WordDict &dict;

    };

    class PosGen : public FeatureGen {
      public:
        PosGen(TagDict &dict);
        virtual ~PosGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        TagDict &dict;

    };

    class OffsetWordGen : public OffsetGen {
      public:
        OffsetWordGen(WordDict &dict, const int offset);
        virtual ~OffsetWordGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        WordDict &dict;
    };

    class OffsetPosGen : public OffsetGen {
      public:
        OffsetPosGen(TagDict &dict, const int offset);
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
        BigramGen(const int offset) : OffsetGen(offset) { }
        virtual ~BigramGen(void) { }
    };

    class BigramWordGen : public BigramGen {
      public:
        BigramWordGen(BiWordDict &dict, const int offset);
        virtual ~BigramWordGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        BiWordDict &dict;
    };

    class BigramPosGen : public BigramGen {
      public:
        BigramPosGen(BiTagDict &dict, const int offset);
        virtual ~BigramPosGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        BiTagDict &dict;
    };

  }
}
