#include "base.h"
#include "config.h"
#include "hashtable.h"
#include "lexicon.h"
#include "tagset.h"
#include "lbfgs.h"
#include "crf/features.h"

namespace NLP { namespace CRF {

WordGen::WordGen(WordDict &dict) : FeatureGen(), dict(dict) { }

Attribute &WordGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void WordGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  Raw word = sent.words[i];
  attributes(type.name, word, tp);
  tp.prev = None::val;
  attributes(type.name, word, tp);
}

void WordGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  Raw word = sent.words[i];
  attributes(type.name, word, c);
}

void WordGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  _add_features(dict.get(type, sent.words[i]), dist);
}

PosGen::PosGen(TagDict &dict) : FeatureGen(), dict(dict) { }

Attribute &PosGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void PosGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  Raw word = sent.pos[i];
  attributes(type.name, word, tp);
  tp.prev = None::val;
  attributes(type.name, word, tp);
}

void PosGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  Raw word = sent.pos[i];
  attributes(type.name, word, c);
}

void PosGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  _add_features(dict.get(type, sent.pos[i]), dist);
}

OffsetWordGen::OffsetWordGen(WordDict &dict, const int offset)
  : OffsetGen(offset), dict(dict) { }

Attribute &OffsetWordGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void OffsetWordGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  TagPair _tp(None::val, tp.curr);
  i += offset;

  if (i >= 0 && i < sent.size()) {
    attributes(type.name, sent.words[i], tp);
    attributes(type.name, sent.words[i], _tp);
  }
}

void OffsetWordGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  i += offset;

  if (i >= 0 && i < sent.size())
    attributes(type.name, sent.words[i], c);
}

void OffsetWordGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  i += offset;

  if (i >= 0 && i < sent.size())
    _add_features(dict.get(type, sent.words[i]), dist);
}

OffsetPosGen::OffsetPosGen(TagDict &dict, const int offset)
  : OffsetGen(offset), dict(dict) { }

Attribute &OffsetPosGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void OffsetPosGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  TagPair _tp(None::val, tp.curr);
  i += offset;

  if (i >= 0 && i < sent.size()) {
    attributes(type.name, sent.pos[i], tp);
    attributes(type.name, sent.pos[i], _tp);
  }
}

void OffsetPosGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  i += offset;

  if (i >= 0 && i < sent.size())
    attributes(type.name, sent.pos[i], c);
}

void OffsetPosGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  i += offset;

  if (i >= 0 && i < sent.size())
    _add_features(dict.get(type, sent.pos[i]), dist);
}

} }
