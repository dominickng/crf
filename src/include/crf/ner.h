namespace NLP {
  namespace CRF {
    class NER : public Tagger {
      public:
        const static std::string name;
        const static std::string desc;
        const static std::string reader;
        const static std::string chain;

        class Config : public Tagger::Config {
          public:
            config::OpPath data;
            config::OpPath gazetteers;
            config::OpPath pos;

            Config(const std::string &name=NER::name,
                const std::string &desc=NER::desc)
              : Tagger::Config(name, desc, 0.707, 400),
                data(*this, "data", "location of the data directory for gazeteers", "//data", true, &model),
                gazetteers(*this, "gazetteers", "location of the gazetteers config file", "//gazetteers", true, &data),
                pos(*this, "pos", "location to save the pos tag file", "//postags", true, &model) { }
        };

        NER(NER::Config &cfg, Types &types, const std::string &preface);

        void train(Reader &reader, const std::string &trainer);
        void run_tag(Reader &reader, Writer &writer);
        void tag(State &state, Sentence &sent);
        void extract(Reader &reader, Instances &instances);

      private:
        class Impl;
    };
  }
}
