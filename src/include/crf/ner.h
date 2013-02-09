namespace NLP {
  namespace CRF {
    class NER : public Tagger {
      public:
        const static std::string name;
        const static std::string desc;
        const static std::string reader;

        class Config : public Tagger::Config {
          public:
            config::OpPath pos;

            Config(const std::string &name="ner",
                const std::string &desc="ner CRF tagger config")
              : Tagger::Config(name, desc),
                pos(*this, "pos", "location to save the pos tag file", "//postags", &model) { }
        };

        NER(NER::Config &cfg, Types &types, const std::string &preface);

        void train(Reader &reader);
        void run_tag(Reader &reader, Writer &writer);
        void tag(State &state, Sentence &sent);
        void extract(Reader &reader, Instances &instances);

      private:
        class Impl;
    };
  }
}
