#include "base.h"
#include "config.h"
#include "hashtable.h"
#include "crf/features/type.h"
#include "crf/features/context.h"
#include "crf/features/feature.h"
#include "crf/features/attributes.h"
#include "crf/features/feature_gen.h"
#include "crf/features/feature_types.h"

namespace NLP { namespace CRF {

const Type FeatureTypes::words = {"word features", "w", 0};

FeatureTypes::FeatureTypes(const std::string &name, const std::string &desc)
  : config::Config(name, desc),
    use_words(*this, "words", "use word features", new WordGen(FeatureTypes::words)) { }

} }
