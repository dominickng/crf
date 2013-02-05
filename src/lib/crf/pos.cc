#include "base.h"

#include "config.h"
#include "io.h"
#include "lbfgs.h"
#include "hashtable/size.h"
#include "shared.h"
#include "lexicon.h"
#include "tagset.h"
#include "crf/nodepool.h"
#include "crf/lattice.h"
#include "crf/features.h"
#include "crf/tagger.h"
#include "crf/pos.h"

namespace NLP { namespace CRF {

const std::string POS::name = "pos";
const std::string POS::desc = "description";
const std::string POS::reader = "format";

POS::FeatureTypes::FeatureTypes(void) :
  Tagger::FeatureTypes() { }

class POS::Impl : public Tagger::Impl {
  protected:
    typedef Tagger::Impl Base;

    virtual void tag(Reader &reader, Writer &writer) {
      load();
      Sentence sent;
      Lattice lattice(tags.size());
      PDFs dist;
      for (int i = 0; i < tags.size(); ++i)
        dist.push_back(PDF(tags.size(), 0.0));

      while (reader.next(sent)) {
        for (int i = 0; i < sent.words.size(); ++i) {
          add_features(sent, dist, i);
          lattice.viterbi(tags, dist);
        }
        //lattice.print(std::cout, tags, sent.size());
        lattice.best(tags, sent.entities, sent.words.size());
        writer.next(sent);
        sent.reset();
        lattice.reset();
      }
    }

    virtual void _pass1(Reader &reader) {
      Sentence sent;
      uint64_t max_size = 0;
      while (reader.next(sent)) {
        for (size_t i = 0; i < sent.size(); ++i) {
          lexicon.add(sent.words[i]);
          tags.add(sent.pos[i]);
          if (sent.size() > max_size)
            max_size = sent.size();
        }
        sent.reset();
      }

      lexicon.save(preface);
      tags.save(preface);
      model.max_size(max_size);
    }

    virtual void _pass2(Reader &reader) {
      Sentence sent;
      Contexts contexts; //not used in this pass
      while (reader.next(sent)) {
        feature_types.generate(attributes, tags, sent, contexts, sent.pos, true);
        sent.reset();
      }

      attributes.save_attributes(cfg.attributes(), preface);
    }

    virtual void _pass3(Reader &reader, Instances &instances) {
      Sentence sent;
      while (reader.next(sent)) {
        Contexts contexts(sent.words.size());
        instances.push_back(contexts);
        feature_types.generate(attributes, tags, sent, instances.back(), sent.pos, false);
        sent.reset();
      }
    }

  public:
    Impl(POS::Config &cfg, POS::FeatureTypes &types, const std::string &preface)
      : Base(cfg, types, preface) { }

};

POS::POS(POS::Config &cfg, POS::FeatureTypes &types, const std::string &preface)
  : Tagger(cfg, preface, new Impl(cfg, types, preface)) { }

void POS::train(Reader &reader) { _impl->train(reader); }

void POS::tag(Reader &reader, Writer &writer) { _impl->tag(reader, writer); }

void POS::extract(Reader &reader, Instances &instances) {
  _impl->extract(reader, instances);
}

} }
