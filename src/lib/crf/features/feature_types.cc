#include "base.h"
#include "config.h"
#include "hashtable.h"
#include "tagset.h"
#include "lbfgs.h"
#include "crf/features/type.h"
#include "crf/features/context.h"
#include "crf/features/feature.h"
#include "crf/features/attributes.h"
#include "crf/features/feature_gen.h"
#include "crf/features/feature_types.h"

namespace NLP { namespace CRF {

const Type FeatureTypes::words = {"word features", "w", 0};
const Type FeatureTypes::pos = {"pos features", "p", 1};

FeatureTypes::FeatureTypes(const TagSet &tags)
  : config::OpGroup("types", "Feature types config"), tags(tags),
    use_words(*this, "words", "use word features", new WordGen(FeatureTypes::words)),
    use_pos(*this, "pos", "use pos features", new PosGen(FeatureTypes::pos)) { }

void FeatureTypes::get_tagpair(Sentence &sent, TagPair &tp, int i) {
  if (i == sent.words.size()) {
    tp.prev = tags.canonize(sent.entities[i-1]);
    tp.curr = Tag((uint16_t)0);
  }
  else if (i == 0) {
    tp.prev = Tag((uint16_t)0);
    tp.curr = tags.canonize(sent.entities[i]);
  }
  else {
    tp.prev = tags.canonize(sent.entities[i-1]);
    tp.curr = tags.canonize(sent.entities[i]);
  }
}

void FeatureTypes::generate(Attributes &attributes, Sentence &sent, Contexts &contexts, const bool extract) {
  TagPair tp;

  for(int i = 0; i <= sent.words.size(); ++i) {
    get_tagpair(sent, tp, i);
    if (extract) {
      use_words.generate(attributes, sent, tp, i);
      use_pos.generate(attributes, sent, tp, i);
    }
    else {
      Context context(tp);
      contexts.push_back(context);
      use_words.generate(attributes, sent, contexts.back(), i);
      use_pos.generate(attributes, sent, contexts.back(), i);
    }
  }
}

} }
