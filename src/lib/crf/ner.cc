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
#include "crf/ner.h"

namespace NLP { namespace CRF {

const std::string NER::name = "ner";
const std::string NER::desc = "description";
const std::string NER::reader = "conll";

NER::FeatureTypes::FeatureTypes(void) :
  Tagger::FeatureTypes(),
  use_pos(*this, "pos", "use pos features", new PosGen(Types::pos)),
  use_prev_pos(*this, "prev_pos", "use previous pos 1 features", new OffsetPosGen(Types::prevpos, -1)),
  use_prev_prev_pos(*this, "prev_prev_pos", "use previous 2 pos features", new OffsetPosGen(Types::prevprevpos, -2)),
  use_next_pos(*this, "next_pos", "use next 1 pos features", new OffsetPosGen(Types::nextpos, 1)),
  use_next_next_pos(*this, "next_next_pos", "use next 2 pos features", new OffsetPosGen(Types::nextnextpos, 2)) { }

class NER::Impl : public Tagger::Impl {
  protected:
    typedef Tagger::Impl Base;

    virtual void add_features(Sentence &sent, PDFs &dist, size_t i) {
      Base::add_features(sent, dist, i);

      if (use_pos)
        _add_features(p_dict.get(Types::pos, sent.pos[i]), dist);

      int index = i+1;
      if (index < sent.pos.size() && use_next_pos) {
        _add_features(w_dict.get(Types::nextpos, sent.pos[index++]), dist);
        if (index < sent.pos.size())
          _add_features(w_dict.get(Types::nextnextpos, sent.pos[index]), dist);
      }
      index = i-1;
      if (index >= 0 && use_prev_pos) {
        _add_features(w_dict.get(Types::prevpos, sent.pos[index--]), dist);
        if (index >= 0)
          _add_features(w_dict.get(Types::prevprevpos, sent.pos[index]), dist);
      }
    }

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
          pos.add(sent.pos[i]);
          tags.add(sent.entities[i]);
          if (sent.size() > max_size)
            max_size = sent.size();
        }
        sent.reset();
      }

      lexicon.save(preface);
      pos.save(preface);
      tags.save(preface);
      model.max_size(max_size);
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
        Contexts contexts(sent.words.size());
        instances.push_back(contexts);
        feature_types.generate(attributes, tags, sent, instances.back(), sent.entities, false);
        sent.reset();
      }
    }

    virtual void reg(void) {
      Tagger::Impl::reg();

      feature_types.reg(Types::pos, p_dict);
      feature_types.reg(Types::prevpos, p_p_dict);
      feature_types.reg(Types::prevprevpos, pp_p_dict);
      feature_types.reg(Types::nextpos, n_p_dict);
      feature_types.reg(Types::nextnextpos, nn_p_dict);
    }

    virtual void load(void) {
      pos.load();
      Tagger::Impl::load();
    }

  public:
    TagSet pos;
    TagDict p_dict;
    TagDict p_p_dict;
    TagDict pp_p_dict;
    TagDict n_p_dict;
    TagDict nn_p_dict;

    const bool use_pos;
    const bool use_prev_pos;
    const bool use_next_pos;

    Impl(NER::Config &cfg, NER::FeatureTypes &types, const std::string &preface)
      : Base(cfg, types, preface), pos(cfg.pos()), p_dict(pos), p_p_dict(pos),
        pp_p_dict(pos), n_p_dict(pos), nn_p_dict(pos),
        use_pos(types.use_pos()), use_prev_pos(types.use_prev_pos()),
        use_next_pos(types.use_next_pos()) { }

};

NER::NER(NER::Config &cfg, NER::FeatureTypes &types, const std::string &preface)
  : Tagger(cfg, preface, new Impl(cfg, types, preface)) { }

void NER::train(Reader &reader) { _impl->train(reader); }

void NER::tag(Reader &reader, Writer &writer) { _impl->tag(reader, writer); }

void NER::extract(Reader &reader, Instances &instances) {
  _impl->extract(reader, instances);
}

} }
