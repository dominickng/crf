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
          void generate(Attributes &attributes, Sentence &sent, TagPair &tp, size_t index);
          void generate(Context &context, Attributes &attributes, Sentence &sent, size_t index);
      };

      class FeatureTypes : public config::Config {
        public:
          OpType use_words;
          const static Type words;

          FeatureTypes(const std::string &name, const std::string &desc);

          void generate(Attributes &attributes, Sentence &sent);
          void generate(Context &context, Attributes &attributes, Sentence &sent);
      };
  }
}
