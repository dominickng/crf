namespace NLP {
  namespace CRF {
      namespace config = Util::config;

      class OpType : public config::OpFlag {
        protected:
          FeatureGen *_gen;

        public:
          OpType(config::OpGroup &group, const std::string &name,
              const std::string &desc, FeatureGen *gen,
              const bool default_value=true)
            : config::OpFlag(group, name, desc, default_value), _gen(gen) { }

          ~OpType(void) { delete _gen; }

          template <typename TPC>
          void generate(Attributes &attributes, Sentence &sent, TPC &tpc, int index) { (*_gen)(attributes, sent, tpc, index); }
      };

      class FeatureTypes : public config::OpGroup {
        public:
          TagSet tags;
          OpType use_words;
          const static Type words;

          OpType use_pos;
          const static Type pos;

          FeatureTypes(const TagSet &tags);

          void get_tagpair(Sentence &sent, TagPair &tp, int i);
          void generate(Attributes &attributes, Sentence &sent, Contexts &contexts, const bool extract);
      };
  }
}
