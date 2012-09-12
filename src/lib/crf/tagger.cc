#include "base.h"

#include "config.h"
#include "io.h"
#include "hashtable/size.h"
#include "shared.h"
#include "lexicon.h"
#include "tagset.h"
#include "crf/features.h"
#include "crf/tagger.h"

namespace NLP { namespace CRF {

Tagger::Tagger(Tagger::Config &cfg, const std::string &preface, Impl *impl)
  : _impl(impl), cfg(cfg) { }

Tagger::Tagger(const Tagger &other)
  : _impl(share(other._impl)), cfg(other.cfg) { }

void Tagger::Impl::extract(Reader &reader) {
  _pass1(reader);
}

void Tagger::Impl::_pass1(Reader &reader) {
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

} }
