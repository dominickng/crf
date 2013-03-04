#include "base.h"

#include "config.h"
#include "fastmath.h"
#include "hashtable/size.h"
#include "io.h"
#include "lbfgs.h"
#include "lexicon.h"
#include "prob.h"
#include "tagset.h"
#include "vector.h"
#include "crf/nodepool.h"
#include "crf/lattice.h"
#include "crf/state.h"
#include "crf/features.h"
#include "crf/tagger.h"
#include "crf/pos.h"

namespace NLP { namespace CRF {

const std::string POS::name = "pos";
const std::string POS::desc = "Part of speech CRF tagger";
const std::string POS::reader = "format";

class POS::Impl : public Tagger::Impl {
  protected:
    typedef Tagger::Impl Base;

    virtual void run_tag(Reader &reader, Writer &writer) {
      load();
      Sentence sent;
      State state(tags.size());

      while (reader.next(sent)) {
        tag(state, sent);
        writer.next(sent);
        sent.reset();
        state.reset();
      }
    }

    virtual void tag(State &state, Sentence &sent) {
      for (size_t i = 0; i < sent.size(); ++i) {
        registry.add_features(lexicon, sent, state.dist, i);
        state.lattice.viterbi(tags, state.dist);
        state.next_word();
      }
      //state.lattice.print(std::cout, tags, sent.size());
      state.lattice.best(tags, sent.pos, sent.size());
    }

    virtual void _pass1(Reader &reader) {
      Sentence sent;
      uint64_t max_size = 0;
      uint64_t nsents = 0;
      while (reader.next(sent)) {
        ++nsents;
        for (size_t i = 0; i < sent.size(); ++i) {
          lexicon.add(sent.words[i]);
          tags.add(sent.pos[i]);
          if (sent.size() > max_size)
            max_size = sent.size();
        }
        sent.reset();
      }

      instances.reserve(nsents);
      lexicon.save(preface);
      tags.save(preface);
      model.max_size(max_size);
    }

    virtual void _pass2(Reader &reader) {
      Sentence sent;
      Contexts contexts; //not used in this pass
      while (reader.next(sent)) {
        registry.generate(attributes, lexicon, tags, sent, sent.pos, contexts, true);
        sent.reset();
      }

      attributes.save_attributes(cfg.attributes(), preface);
    }

    virtual void _pass3(Reader &reader, Instances &instances) {
      Sentence sent;
      while (reader.next(sent)) {
        Contexts contexts(sent.words.size());
        instances.push_back(contexts);
        registry.generate(attributes, lexicon, tags, sent, sent.pos, instances.back(), false);
        sent.reset();
      }
    }

  public:
    Impl(POS::Config &cfg, Types &types, const std::string &preface)
      : Base(cfg, types, preface) { }

};

POS::POS(POS::Config &cfg, Types &types, const std::string &preface)
  : Tagger(cfg, preface, new Impl(cfg, types, preface)) { }

void POS::train(Reader &reader, const std::string &trainer) { _impl->train(reader, trainer); }

void POS::run_tag(Reader &reader, Writer &writer) { _impl->run_tag(reader, writer); }

void POS::tag(State &state, Sentence &sent) { _impl->tag(state, sent); }

void POS::extract(Reader &reader, Instances &instances) {
  _impl->extract(reader, instances);
}

} }
