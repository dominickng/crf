namespace NLP {
  namespace CRF {
    class Registry {
      public:
        Registry(const uint64_t rare_cutoff, const size_t nbuckets=HT::TINY, const size_t pool_size=HT::TINY);
        Registry(const Registry &other);
       ~Registry(void);

        void reg(const Type &type, config::Op<bool> &op, FeatureGen *gen, const bool rare=false);
        Attribute &load(const std::string &type, std::istream &in);

        void generate(Attributes &attributes, Lexicon lexicon, TagSet tags, Sentence &sent, Raws &rawtags, Contexts &contexts, const bool extract);

        void add_features(Lexicon lexicon, Sentence &sent, PDFs &dist, int i);

      private:
        class Impl;
        Impl *_impl;
    };
  }
}
