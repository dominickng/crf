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
            config::OpPath attributes;
            config::OpPath features;
            Config(const std::string &name, const std::string &desc)
              : config::Config(name, desc),
            model(*this, "model", "default location to save the model"),
            lexicon(*this, "lexicon", "default location to save the lexicon file", "//lexicon", &model),
            tags(*this, "tags", "default location to save the tag file", "//tags", &model),
            attributes(*this, "attributes", "default location to save the attributes file", "//attributes", &model),
            features(*this, "features", "default location to save the features file", "//features", &model) { }

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
        FeatureTypes feature_types;
        std::string preface;

        Impl(Config &cfg, const std::string &preface)
          : Util::Shared(), cfg(cfg), lexicon(), tags(), attributes(),
            feature_types(tags), preface(preface) { }

        virtual ~Impl(void) { /* nothing */ }

        virtual void train(Reader &reader);
        virtual void extract(Reader &reader, Instances &instances);
        virtual void _pass1(Reader &reader) = 0;
        virtual void _pass2(Reader &reader) = 0;
        virtual void _pass3(Reader &reader, Instances &instances) = 0;
    };

  }
}
