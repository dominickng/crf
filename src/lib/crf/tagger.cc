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

void Tagger::Impl::train(Reader &reader) {
  Instances instances;
  extract(reader, instances);
  //TODO actually train
}

void Tagger::Impl::extract(Reader &reader, Instances &instances) {
  std::cerr << "beginning pass 1" << std::endl;
  _pass1(reader);
  reader.reset();
  std::cerr << "beginning pass 2" << std::endl;
  _pass2(reader);
  reader.reset();
  std::cerr << "beginning pass 3" << std::endl;
  _pass3(reader, instances);
}

} }
