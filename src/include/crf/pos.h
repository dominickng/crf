namespace NLP {
  namespace CRF {
    class POS : public Tagger {
      public:
        const static std::string name;
        const static std::string desc;
        const static std::string reader;
        const static std::string chain;

        class Config : public Tagger::Config {
          public:
            Config(const std::string &name="pos",
                const std::string &desc="pos CRF tagger config")
              : Tagger::Config(name, desc, 1.414, 500) { }
        };

        POS(POS::Config &cfg, Types &types, const std::string &preface);

        void train(Reader &reader, const std::string &trainer);
        void run_tag(Reader &reader, Writer &writer);
        void tag(State &state, Sentence &sent);
        void extract(Reader &reader, Instances &instances);

      private:
        class Impl;
    };
  }
}
