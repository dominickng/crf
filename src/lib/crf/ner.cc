#include "base.h"

#include "config.h"
#include "io.h"
#include "lbfgs.h"
#include "hashtable/size.h"
#include "shared.h"
#include "lexicon.h"
#include "tagset.h"
#include "crf/features.h"
#include "crf/tagger.h"
#include "crf/ner.h"

namespace NLP { namespace CRF {

const std::string NER::name = "ner";
const std::string NER::desc = "description";

NER::FeatureTypes::FeatureTypes(void) :
  Tagger::FeatureTypes(),
  use_pos(*this, "pos", "use pos features", new PosGen(Types::pos)),
  use_prev_pos(*this, "prev_pos", "use previous pos features", new PrevPosGen(Types::pos)),
  use_next_pos(*this, "next_pos", "use next pos features", new NextPosGen(Types::pos)) { }

class NER::Impl : public Tagger::Impl {
  protected:
    typedef Tagger::Impl Base;

    virtual void _pass1(Reader &reader) {
      Sentence sent;
      while (reader.next(sent)) {
        for (size_t i = 0; i < sent.size(); ++i) {
          lexicon.add(sent.words[i]);
          pos.add(sent.pos[i]);
          tags.add(sent.entities[i]);
        }
        sent.reset();
      }

      lexicon.save(preface);
      pos.save(preface);
      tags.save(preface);
    }

    virtual void _pass2(Reader &reader) {
      Sentence sent;
      Contexts contexts; //not used in this pass
      while (reader.next(sent)) {
        feature_types.generate(attributes, tags, sent, contexts, sent.entities, true);
        sent.reset();
      }

      attributes.save_attributes(cfg.attributes(), preface);
    }

    virtual void _pass3(Reader &reader, Instances &instances) {
      Sentence sent;
      while (reader.next(sent)) {
        Contexts contexts(sent.words.size(), tags.size());
        instances.push_back(contexts);
        feature_types.generate(attributes, tags, sent, instances.back(), sent.entities, false);
        sent.reset();
      }
    }

    virtual void reg(void) {
      Tagger::Impl::reg();

      feature_types.reg(Types::pos, p_dict);
      feature_types.reg(Types::prevpos, p_dict);
      feature_types.reg(Types::prevprevpos, p_dict);
      feature_types.reg(Types::nextpos, p_dict);
      feature_types.reg(Types::nextnextpos, p_dict);
    }
  public:
    TagSet pos;
    TagDict p_dict;

    Impl(NER::Config &cfg, NER::FeatureTypes &types, const std::string &preface)
      : Base(cfg, types, preface), pos(cfg.pos()), p_dict(pos) { }

};

NER::NER(NER::Config &cfg, NER::FeatureTypes &types, const std::string &preface)
  : Tagger(cfg, preface, new Impl(cfg, types, preface)) { }

void NER::train(Reader &reader) { _impl->train(reader); }

void NER::extract(Reader &reader, Instances &instances) {
  _impl->extract(reader, instances);
}

} }
