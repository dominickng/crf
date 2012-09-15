#include "base.h"
#include "hashtable.h"
#include "lbfgs.h"
#include "crf/features/type.h"
#include "crf/features/context.h"
#include "crf/features/feature.h"
#include "crf/features/attributes.h"
#include "crf/features/feature_gen.h"

namespace NLP { namespace CRF {

WordGen::WordGen(const Type &type) : FeatureGen(type) { }

void WordGen::operator()(Attributes &attributes, Sentence &sent, TagPair &tp, int j) {
  if (j < sent.words.size()) {
    Raw word = sent.words[j];
    attributes(type.id, word, tp);
  }
}

void WordGen::operator()(Attributes &attributes, Sentence &sent, Context &c, int j) {
  if (j < sent.words.size()) {
    Raw word = sent.words[j];
    attributes(type.id, word, c);
  }
}

PosGen::PosGen(const Type &type) : FeatureGen(type) { }

void PosGen::operator()(Attributes &attributes, Sentence &sent, TagPair &tp, int j) {
  if (j < sent.words.size()) {
    Raw word = sent.pos[j];
    attributes(type.id, word, tp);
  }
}

void PosGen::operator()(Attributes &attributes, Sentence &sent, Context &c, int j) {
  if (j < sent.words.size()) {
    Raw word = sent.pos[j];
    attributes(type.id, word, c);
  }
}


} }
