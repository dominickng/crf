namespace NLP {
  namespace CRF {
    class FeatureGen {
      protected:
        void _add_features(Attribute attrib, PDFs &dist) {
          for (Weight *w = attrib.begin; w != attrib.end; ++w) {
            //std::cout << "adding weight " << w->lambda << " for " << tags.str(w->prev) << " -> " << tags.str(w->curr) << std::endl;
            if (w->prev == None::val)
              for (Tag t = 0; t < dist.size(); ++t)
                dist[t][w->curr] += w->lambda;
            else
              dist[w->prev][w->curr] += w->lambda;
          }
        }

      public:
        FeatureGen(void) { }
        virtual ~FeatureGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in) = 0;

        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) = 0;
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) = 0;
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i) = 0;
    };

    class OffsetGen : public FeatureGen {
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

    class BigramWordGen : public OffsetGen {
      public:
        BigramWordGen(BiWordDict &dict, const int offset);
        virtual ~BigramWordGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        BiWordDict &dict;
      private:
        virtual void _get_raw(Sentence &sent, Raw &raw, int i);
    };

    class BigramPosGen : public OffsetGen {
      public:
        BigramPosGen(BiTagDict &dict, const int offset);
        virtual ~BigramPosGen(void) { }

        virtual Attribute &load(const Type &type, std::istream &in);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i);
        virtual void operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i);
        virtual void operator()(const Type &type, Sentence &sent, PDFs &dist, int i);

        BiTagDict &dict;
      private:
        virtual void _get_raw(Sentence &sent, Raw &raw, int i);
    };

  }
}
