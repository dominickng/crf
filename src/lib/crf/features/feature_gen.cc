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
  const Raw *word;
  i += offset;

  if (i >= 0 && i < sent.size())
    word = &sent.words[i];
  else
    word = &Sentinel::str;

  attributes(type.name, *word, tp);
  attributes(type.name, *word, _tp);
}

void OffsetWordGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  const Raw *word;
  i += offset;

  if (i >= 0 && i < sent.size())
    word = &sent.words[i];
  else
    word = &Sentinel::str;
  attributes(type.name, *word, c);
}

void OffsetWordGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  const Raw *word;
  i += offset;

  if (i >= 0 && i < sent.size())
    word = &sent.words[i];
  else
    word = &Sentinel::str;
  _add_features(dict.get(type, *word), dist);
}

OffsetPosGen::OffsetPosGen(TagDict &dict, const int offset)
  : OffsetGen(offset), dict(dict) { }

Attribute &OffsetPosGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void OffsetPosGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  TagPair _tp(None::val, tp.curr);
  const Raw *raw;
  i += offset;

  if (i >= 0 && i < sent.size())
    raw = &sent.pos[i];
  else
    raw = &Sentinel::str;

  attributes(type.name, *raw, tp);
  attributes(type.name, *raw, _tp);
}

void OffsetPosGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  const Raw *raw;
  i += offset;

  if (i >= 0 && i < sent.size())
    raw = &sent.pos[i];
  else
    raw = &Sentinel::str;

  attributes(type.name, *raw, c);
}

void OffsetPosGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  const Raw *raw;
  i += offset;

  if (i >= 0 && i < sent.size())
    raw = &sent.pos[i];
  else
    raw = &Sentinel::str;
  _add_features(dict.get(type, *raw), dist);
}

BigramWordGen::BigramWordGen(BiWordDict &dict, const int offset)
  : OffsetGen(offset), dict(dict) { }

Attribute &BigramWordGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void BigramWordGen::_get_raw(Sentence &sent, Raw &raw, int i) {
  i += offset;
  if (i >= 0 && i < sent.size()) {
    raw += sent.words[i++];
    raw += ' ';
    if (i >= 0 && i < sent.size())
      raw += sent.words[i];
    else
      raw += Sentinel::str;
  }
  else {
    raw += Sentinel::str;
    raw += ' ';
    if (++i >= 0 && i < sent.size())
      raw += sent.words[i];
    else
      raw += Sentinel::str;
  }
}

void BigramWordGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  TagPair _tp(None::val, tp.curr);
  Raw raw;

  _get_raw(sent, raw, i);

  attributes(type.name, raw, tp);
  attributes(type.name, raw, _tp);
}

void BigramWordGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  Raw raw;

  _get_raw(sent, raw, i);

  attributes(type.name, raw, c);
}

void BigramWordGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  const Raw *raw1, *raw2;

  if (i >= 0 && i < sent.size()) {
    raw1 = &sent.words[i++];
    if (i >= 0 && i < sent.size())
      raw2 = &sent.words[i];
    else
      raw2 = &Sentinel::str;
  }
  else {
    raw1 = &Sentinel::str;
    if (++i >= 0 && i < sent.size())
      raw2 = &sent.words[i];
    else
      raw2 = &Sentinel::str;
  }

  _add_features(dict.get(type, *raw1, *raw2), dist);
}

BigramPosGen::BigramPosGen(BiTagDict &dict, const int offset)
  : OffsetGen(offset), dict(dict) { }

Attribute &BigramPosGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void BigramPosGen::_get_raw(Sentence &sent, Raw &raw, int i) {
  i += offset;
  if (i >= 0 && i < sent.size()) {
    raw += sent.pos[i++];
    raw += ' ';
    if (i >= 0 && i < sent.size())
      raw += sent.pos[i];
    else
      raw += Sentinel::str;
  }
  else {
    ++i;
    raw += Sentinel::str;
    raw += ' ';
    if (i >= 0 && i < sent.size())
      raw += sent.pos[i];
    else
      raw += Sentinel::str;
  }
}

void BigramPosGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  TagPair _tp(None::val, tp.curr);
  Raw raw;

  _get_raw(sent, raw, i);

  attributes(type.name, raw, tp);
  attributes(type.name, raw, _tp);
}

void BigramPosGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  Raw raw;

  _get_raw(sent, raw, i);

  attributes(type.name, raw, c);
}

void BigramPosGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  const Raw *raw1, *raw2;

  if (i >= 0 && i < sent.size()) {
    raw1 = &sent.pos[i++];
    if (i >= 0 && i < sent.size())
      raw2 = &sent.pos[i];
    else
      raw2 = &Sentinel::str;
  }
  else {
    raw1 = &Sentinel::str;
    if (i >= 0 && i < sent.size())
      raw2 = &sent.pos[i];
    else
      raw2 = &Sentinel::str;
  }

  _add_features(dict.get(type, *raw1, *raw2), dist);
}
} }
