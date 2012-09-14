#include "base.h"

#include "config.h"
#include "io.h"
#include "hashtable/size.h"
#include "shared.h"
#include "lexicon.h"
#include "tagset.h"
#include "crf/features.h"
#include "crf/tagger.h"
#include "crf/ner.h"

namespace NLP { namespace CRF {

const std::string NER::name = "ner";
const std::string NER::desc = "desc";

class NER::Impl : public Tagger::Impl {
  private:
    typedef Tagger::Impl Base;
  public:
    Impl(NER::Config &cfg, const std::string &preface) : Base(cfg, preface) { }

    virtual void _pass1(Reader &reader) {
      Sentence sent;
      while (reader.next(sent)) {
        for (size_t i = 0; i < sent.size(); ++i) {
          lexicon.add(sent.words[i]);
          tags.add(sent.entities[i]);
        }
        sent.reset();
      }

      lexicon.save(cfg.lexicon(), preface);
      tags.save(cfg.tags(), preface);
    }

    virtual void _pass2(Reader &reader) {
      Sentence sent;
      while (reader.next(sent)) {
        feature_types.generate(attributes, sent);
        sent.reset();
      }

      attributes.save_attributes(cfg.attributes(), preface);
      attributes.save_features(cfg.features(), preface);
    }

    virtual void _pass3(Reader &reader, Instances &instances) {
      Sentence sent;
      while (reader.next(sent)) {
        Contexts contexts;
        instances.push_back(contexts);
        feature_types.generate(attributes, sent, instances.back());
        sent.reset();
      }
    }
};

NER::NER(NER::Config &cfg, const std::string &preface)
  : Tagger(cfg, preface, new Impl(cfg, preface)) { }

void NER::train(Reader &reader) { _impl->train(reader); }

void NER::extract(Reader &reader, Instances &instances) {
  _impl->extract(reader, instances);
}

} }
