namespace NLP {
  namespace CRF {
    class NER : public Tagger {
      public:
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
