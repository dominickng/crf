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
  //tp.prev = None::val;
  //attributes(type.id, word, tp);
}

void WordGen::operator()(Attributes &attributes, Sentence &sent, Context &c, int j) {
  Raw word = sent.words[j];
  attributes(type.id, word, c);
}

PosGen::PosGen(const Type &type) : FeatureGen(type) { }

void PosGen::operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j) {
  Raw word = sent.pos[j];
  attributes(type.id, word, tp);
  //tp.prev = None::val;
  //attributes(type.id, word, tp);
}

void PosGen::operator()(Attributes &attributes, Sentence &sent, Context &c, int j) {
  Raw word = sent.pos[j];
  attributes(type.id, word, c);
}

PrevWordGen::PrevWordGen(const Type &type) : FeatureGen(type) { }

void PrevWordGen::operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j) {
  Raw word;
  TagPair _tp(None::val, tp.curr);
  j -= 1;

  if (j >= 0) {
    word = sent.words[j--];
    attributes(Types::prevword.id, word, tp);
    attributes(Types::prevword.id, word, _tp);
    if (j >= 0) {
      word = sent.words[j];
      attributes(Types::prevprevword.id, word, tp);
      attributes(Types::prevprevword.id, word, _tp);
    }
  }
}

void PrevWordGen::operator()(Attributes &attributes, Sentence &sent, Context &c, int j) {
  Raw word;
  j -= 1;

  if (j >= 0) {
    word = sent.words[j--];
    attributes(Types::prevword.id, word, c);
    if (j >= 0) {
      word = sent.words[j];
      attributes(Types::prevprevword.id, word, c);
    }
  }
}

NextWordGen::NextWordGen(const Type &type) : FeatureGen(type) { }

void NextWordGen::operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j) {
  Raw word;
  TagPair _tp(None::val, tp.curr);
  j += 1;

  if (j < sent.words.size()) {
    word = sent.words[j++];
    attributes(Types::nextword.id, word, tp);
    attributes(Types::nextword.id, word, _tp);
    if (j < sent.words.size()) {
      word = sent.words[j];
      attributes(Types::nextnextword.id, word, tp);
      attributes(Types::nextnextword.id, word, _tp);
    }
  }
}

void NextWordGen::operator()(Attributes &attributes, Sentence &sent, Context &c, int j) {
  Raw word;
  j += 1;

  if (j < sent.words.size()) {
    word = sent.words[j++];
    attributes(Types::nextword.id, word, c);
    if (j < sent.words.size()) {
      word = sent.words[j];
      attributes(Types::nextnextword.id, word, c);
    }
  }
}

PrevPosGen::PrevPosGen(const Type &type) : FeatureGen(type) { }

void PrevPosGen::operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j) {
  Raw pos;
  TagPair _tp(None::val, tp.curr);
  j -= 1;

  if (j >= 0) {
    pos = sent.pos[j--];
    attributes(Types::prevpos.id, pos, tp);
    attributes(Types::prevpos.id, pos, _tp);
    if (j >= 0) {
      pos = sent.pos[j];
      attributes(Types::prevprevpos.id, pos, tp);
      attributes(Types::prevprevpos.id, pos, _tp);
    }
  }
}

void PrevPosGen::operator()(Attributes &attributes, Sentence &sent, Context &c, int j) {
  Raw pos;
  j -= 1;

  if (j >= 0) {
    pos = sent.pos[j--];
    attributes(Types::prevpos.id, pos, c);
    if (j >= 0) {
      pos = sent.pos[j];
      attributes(Types::prevprevpos.id, pos, c);
    }
  }
}

NextPosGen::NextPosGen(const Type &type) : FeatureGen(type) { }

void NextPosGen::operator()(Attributes &attributes, Sentence &sent, TagPair tp, int j) {
  Raw pos;
  TagPair _tp(None::val, tp.curr);
  j += 1;

  if (j < sent.pos.size()) {
    pos = sent.pos[j++];
    attributes(Types::nextpos.id, pos, tp);
    attributes(Types::nextpos.id, pos, _tp);
    if (j < sent.pos.size()) {
      pos = sent.pos[j];
      attributes(Types::nextnextpos.id, pos, tp);
      attributes(Types::nextnextpos.id, pos, _tp);
    }
  }
}

void NextPosGen::operator()(Attributes &attributes, Sentence &sent, Context &c, int j) {
  Raw pos;
  j += 1;

  if (j < sent.pos.size()) {
    pos = sent.pos[j++];
    attributes(Types::nextpos.id, pos, c);
    if (j < sent.pos.size()) {
      pos = sent.pos[j];
      attributes(Types::nextnextpos.id, pos, c);
    }
  }
}

} }
