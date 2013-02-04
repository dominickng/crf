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

void WordGen::operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j) {
  Raw word = sent.words[j];
  attributes(type.id, word, tp);
  tp.prev = None::val;
  attributes(type.id, word, tp);
}

void WordGen::operator()(Attributes &attributes, Sentence &sent, Context &c, int j) {
  Raw word = sent.words[j];
  attributes(type.id, word, c);
}

PosGen::PosGen(const Type &type) : FeatureGen(type) { }

void PosGen::operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j) {
  Raw word = sent.pos[j];
  attributes(type.id, word, tp);
  tp.prev = None::val;
  attributes(type.id, word, tp);
}

void PosGen::operator()(Attributes &attributes, Sentence &sent, Context &c, int j) {
  Raw word = sent.pos[j];
  attributes(type.id, word, c);
}

OffsetWordGen::OffsetWordGen(const Type &type, const int offset) : OffsetGen(type, offset) { }

void OffsetWordGen::operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j) {
  TagPair _tp(None::val, tp.curr);
  j += offset;

  if (j >= 0 && j < sent.size()) {
    attributes(type.id, sent.words[j], tp);
    attributes(type.id, sent.words[j], _tp);
  }
}

void OffsetWordGen::operator()(Attributes &attributes, Sentence &sent, Context &c, int j) {
  j += offset;

  if (j >= 0 && j < sent.size())
    attributes(type.id, sent.words[j], c);
}

OffsetPosGen::OffsetPosGen(const Type &type, const int offset) : OffsetGen(type, offset) { }

void OffsetPosGen::operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j) {
  TagPair _tp(None::val, tp.curr);
  j += offset;

  if (j >= 0 && j < sent.size()) {
    attributes(type.id, sent.pos[j], tp);
    attributes(type.id, sent.pos[j], _tp);
  }
}

void OffsetPosGen::operator()(Attributes &attributes, Sentence &sent, Context &c, int j) {
  j += offset;

  if (j >= 0 && j < sent.size())
    attributes(type.id, sent.pos[j], c);
}

} }
