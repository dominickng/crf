namespace NLP {
  namespace CRF {
    class NER : public Tagger {
      public:
        const static std::string name;
        const static std::string desc;

        class Config : public Tagger::Config {
          public:
            Config(const std::string &name="ner",
                const std::string &desc="ner CRF tagger config")
              : Tagger::Config(name, desc) { }
        };
        NER(NER::Config &cfg, const std::string &preface);

        void extract(Reader &reader);

      private:
        class Impl;
    };
  }
}
