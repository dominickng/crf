#include "base.h"
#include "config.h"
#include "hashtable.h"
#include "lexicon.h"
#include "tagset.h"
#include "lbfgs.h"
#include "crf/features.h"

namespace NLP { namespace CRF {

void FeatureGen::_add_features(Attribute attrib, PDFs &dist) {
  for (Weight *w = attrib.begin; w != attrib.end; ++w) {
    //std::cout << "adding weight " << w->lambda << " for " << tags.str(w->prev) << " -> " << tags.str(w->curr) << std::endl;
    if (w->prev == None::val)
      for (Tag t = 0; t < dist.size(); ++t)
        dist[t][w->curr] += w->lambda;
    else
      dist[w->prev][w->curr] += w->lambda;
  }
}

const Raw *OffsetGen::_get_raw(Raws &raws, int i) {
  const Raw *raw;
  i += offset;

  if (i >= 0 && i < raws.size())
    raw = &raws[i];
  else
    raw = &Sentinel::str;
  return raw;
}

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
  const Raw *word = _get_raw(sent.words, i);

  attributes(type.name, *word, tp);
  attributes(type.name, *word, _tp);
}

void OffsetWordGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  const Raw *word = _get_raw(sent.words, i);

  attributes(type.name, *word, c);
}

void OffsetWordGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  const Raw *word = _get_raw(sent.words, i);

  _add_features(dict.get(type, *word), dist);
}

OffsetPosGen::OffsetPosGen(TagDict &dict, const int offset)
  : OffsetGen(offset), dict(dict) { }

Attribute &OffsetPosGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void OffsetPosGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  TagPair _tp(None::val, tp.curr);
  const Raw *raw = _get_raw(sent.pos, i);

  attributes(type.name, *raw, tp);
  attributes(type.name, *raw, _tp);
}

void OffsetPosGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  const Raw *raw = _get_raw(sent.pos, i);

  attributes(type.name, *raw, c);
}

void OffsetPosGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  const Raw *raw = _get_raw(sent.pos, i);

  _add_features(dict.get(type, *raw), dist);
}

void BigramGen::_get_raw(Raws &raws, Raw &raw, int i) {
  i += offset;
  if (i >= 0 && i < raws.size()) {
    raw += raws[i++];
    raw += ' ';
    if (i >= 0 && i < raws.size())
      raw += raws[i];
    else
      raw += Sentinel::str;
  }
  else {
    raw += Sentinel::str;
    raw += ' ';
    if (++i >= 0 && i < raws.size())
      raw += raws[i];
    else
      raw += Sentinel::str;
  }
}

BigramWordGen::BigramWordGen(BiWordDict &dict, const int offset)
  : BigramGen(offset), dict(dict) { }

Attribute &BigramWordGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void BigramWordGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  TagPair _tp(None::val, tp.curr);
  Raw raw;

  _get_raw(sent.words, raw, i);

  attributes(type.name, raw, tp);
  attributes(type.name, raw, _tp);
}

void BigramWordGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  Raw raw;

  _get_raw(sent.words, raw, i);

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
  : BigramGen(offset), dict(dict) { }

Attribute &BigramPosGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void BigramPosGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  TagPair _tp(None::val, tp.curr);
  Raw raw;

  _get_raw(sent.pos, raw, i);

  attributes(type.name, raw, tp);
  attributes(type.name, raw, _tp);
}

void BigramPosGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  Raw raw;

  _get_raw(sent.pos, raw, i);

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
