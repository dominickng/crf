namespace NLP {
  namespace CRF {
    class NER : public Tagger {
      public:
        const static std::string name;
        const static std::string desc;

        class Config : public Tagger::Config {
          public:
            config::OpPath pos;

            Config(const std::string &name="ner",
                const std::string &desc="ner CRF tagger config")
              : Tagger::Config(name, desc),
                pos(*this, "pos", "location to save the pos tag file", "//postags", &model)
                { }
        };

        class FeatureTypes : public Tagger::FeatureTypes {
          public:
            OpType use_pos;
            OpType use_prev_pos;
            OpType use_next_pos;

            FeatureTypes(void);

        };

        NER(NER::Config &cfg, NER::FeatureTypes &types, const std::string &preface);

        void train(Reader &reader);
        void extract(Reader &reader, Instances &instances);

      private:
        class Impl;
    };
  }
}
