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

    virtual void _pass2(Reader &reader) {
      Sentence sent;
      while (reader.next(sent)) {
        feature_types.generate(attributes, sent);
        sent.reset();
      }

      attributes.save_attributes(cfg.attributes(), preface);
      attributes.save_features(cfg.features(), preface);
    }

};

NER::NER(NER::Config &cfg, const std::string &preface)
  : Tagger(cfg, preface, new Impl(cfg, preface)) { }

void NER::extract(Reader &reader) { _impl->extract(reader); }

} }
