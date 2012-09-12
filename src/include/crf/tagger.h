namespace config = Util::config;

namespace NLP {
  namespace CRF {
    class Tagger {
      public:
        class Config : public config::Config {
          public:
            config::OpPath model;
            config::OpPath lexicon;
            config::OpPath tags;
            Config(const std::string &name, const std::string &desc)
              : config::Config(name, desc),
            model(*this, "model", "default location to save the model"),
            lexicon(*this, "lexicon", "default location to save the lexicon file", "//lexicon", &model),
            tags(*this, "tags", "default location to save the tag file", "//tags", &model) { }

            virtual ~Config(void) { /* nothing */ }
        };

      protected:
        class Impl;

        Impl *_impl;
        Config &cfg;

        Tagger(Tagger::Config &cfg, const std::string &preface, Impl *impl);
        Tagger(const Tagger &other);
        virtual ~Tagger(void) { release(_impl); }

    };

    class Tagger::Impl : public Util::Shared {
      public:
        Config &cfg;
        Lexicon lexicon;
        TagSet tags;
        Attributes attributes;
        std::string preface;

        Impl(Config &cfg, const std::string &preface)
          : Util::Shared(), cfg(cfg), lexicon(), tags(), attributes(), preface(preface) { }

        ~Impl(void) { /* nothing */}

        virtual void extract(Reader &reader);
        virtual void _pass1(Reader &reader);
    };

  }
}
