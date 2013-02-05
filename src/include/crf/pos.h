namespace NLP {
  namespace CRF {
    class POS : public Tagger {
      public:
        const static std::string name;
        const static std::string desc;
        const static std::string reader;

        class Config : public Tagger::Config {
          public:
            config::OpPath pos;
            config::Op<std::string> ifmt;
            config::Op<std::string> ofmt;

            Config(const std::string &name="ner",
                const std::string &desc="ner CRF tagger config")
              : Tagger::Config(name, desc),
                pos(*this, "pos", "location to save the pos tag file", "//postags", &model),
                ifmt(*this, "ifmt", "input file format", "%w|%p \n", false),
                ofmt(*this, "ofmt", "output file format", "%w|%p \n", false)
                { }
        };

        class FeatureTypes : public Tagger::FeatureTypes {
          public:
            FeatureTypes(void);
        };

        POS(POS::Config &cfg, POS::FeatureTypes &types, const std::string &preface);

        void train(Reader &reader);
        void tag(Reader &reader, Writer &writer);
        void extract(Reader &reader, Instances &instances);

      private:
        class Impl;
    };
  }
}
