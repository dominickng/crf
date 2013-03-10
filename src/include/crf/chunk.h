namespace NLP {
  namespace CRF {
    class Chunk : public Tagger {
      public:
        const static std::string name;
        const static std::string desc;
        const static std::string reader;
        const static std::string chain;

        class Config : public Tagger::Config {
          public:
            config::OpPath pos;

            Config(const std::string &name=Chunk::name,
                const std::string &desc=Chunk::desc)
              : Tagger::Config(name, desc, 0.707, 500),
                pos(*this, "pos", "location to save the pos tag file", "//postags", true, &model) { }
        };

        Chunk(Chunk::Config &cfg, Types &types, const std::string &preface);

        void train(Reader &reader, const std::string &trainer);
        void run_tag(Reader &reader, Writer &writer);
        void tag(State &state, Sentence &sent);
        void extract(Reader &reader, Instances &instances);

      private:
        class Impl;
    };
  }
}
