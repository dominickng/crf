namespace NLP {
  namespace CRF {
    class Chunk : public Tagger {
      public:
        const static std::string name;
        const static std::string desc;
        const static std::string reader;

        class Config : public Tagger::Config {
          public:
            config::OpPath pos;

            Config(const std::string &name="chunk",
                const std::string &desc="chunk CRF tagger config")
              : Tagger::Config(name, desc, 0.707, 400),
                pos(*this, "pos", "location to save the pos tag file", "//postags", &model) { }
        };

        Chunk(Chunk::Config &cfg, Types &types, const std::string &preface);

        void train(Reader &reader);
        void run_tag(Reader &reader, Writer &writer);
        void tag(State &state, Sentence &sent);
        void extract(Reader &reader, Instances &instances);

      private:
        class Impl;
    };
  }
}
