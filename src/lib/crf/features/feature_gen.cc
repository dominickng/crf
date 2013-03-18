#include "base.h"

#include "config.h"
#include "hashtable.h"
#include "gazetteers.h"
#include "lexicon.h"
#include "prob.h"
#include "tagset.h"
#include "crf/features.h"
#include "crf/morph.h"

namespace NLP { namespace CRF {

/**
 * _add_features.
 *
 * Utility method to add all lambdas associated with an attribute to a
 * probability distribution, indexed by the previous and current tags for each
 * lambda.
 */
void FeatureGen::_add_features(Attribute attrib, PDFs &dist) {
  for (Weight *w = attrib.begin; w != attrib.end; ++w) {
    //std::cout << "adding weight " << w->lambda << " for " << w->prev << " -> " << w->curr << std::endl;
    dist[w->prev][w->curr] += w->lambda;
  }
}

const std::string TransGen::name = "trans";

TransGen::TransGen(TransDict &dict, const bool add_state, const bool add_trans)
  : FeatureGen(add_state, add_trans), dict(dict), trans_loaded(false) { }

Attribute &TransGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void TransGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  if (i > 0 && tp.prev.type() == tp.curr.type())
    attributes(type.name, name, tp, false, true);
}

void TransGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  // load the trans features into one vector in attributes rather than
  // wastefully add feature pointers to every single context
  if (!trans_loaded) {
    attributes.load_trans_features(type.name, name);
    trans_loaded = true;
  }
}

void TransGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  if (i > 0)
    _add_features(dict.get(type), dist);
}

/**
 * _get_raw.
 * Utility function to get the appropriate offset value for subclasses of
 * OffsetGen. If the requested offset is out of range, Sentinel::str is
 * returned.
 */
const Raw *OffsetGen::_get_raw(Raws &raws, int i) {
  const Raw *raw = 0;
  i += offset;

  if (i >= 0 && (size_t)i < raws.size())
    raw = &raws[i];
  else
    raw = &Sentinel::str;
  return raw;
}

WordGen::WordGen(WordDict &dict, const bool add_state, const bool add_trans)
  : FeatureGen(add_state, add_trans), dict(dict) { }

Attribute &WordGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void WordGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  attributes(type.name, sent.words[i], tp, _add_state, _add_trans);
}

void WordGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  attributes(type.name, sent.words[i], c);
}

void WordGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  _add_features(dict.get(type, sent.words[i]), dist);
}

PrefixGen::PrefixGen(AffixDict &dict, const bool add_state, const bool add_trans)
  : FeatureGen(add_state, add_trans), dict(dict) { }

Attribute &PrefixGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void PrefixGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  Raw affix, word = sent.words[i];
  std::string::iterator j = word.begin();

  attributes(type.name, affix += *j, tp, _add_state, _add_trans);
  if (++j == word.end()) return;
  attributes(type.name, affix += *j, tp, _add_state, _add_trans);
  if (++j == word.end()) return;
  attributes(type.name, affix += *j, tp, _add_state, _add_trans);
  if (++j == word.end()) return;
  attributes(type.name, affix += *j, tp, _add_state, _add_trans);
}

void PrefixGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  Raw affix, word = sent.words[i];
  std::string::iterator j = word.begin();

  attributes(type.name, affix += *j, c);
  if (++j == word.end()) return;
  attributes(type.name, affix += *j, c);
  if (++j == word.end()) return;
  attributes(type.name, affix += *j, c);
  if (++j == word.end()) return;
  attributes(type.name, affix += *j, c);
}

void PrefixGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  Raw affix, word = sent.words[i];
  std::string::iterator j = word.begin();

  _add_features(dict.get(type, affix += *j), dist);
  if (++j == word.end()) return;
  _add_features(dict.get(type, affix += *j), dist);
  if (++j == word.end()) return;
  _add_features(dict.get(type, affix += *j), dist);
  if (++j == word.end()) return;
  _add_features(dict.get(type, affix += *j), dist);
}

SuffixGen::SuffixGen(AffixDict &dict, const bool add_state, const bool add_trans)
  : FeatureGen(add_state, add_trans), dict(dict) { }

Attribute &SuffixGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void SuffixGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  Raw affix, word = sent.words[i];
  std::string::reverse_iterator j = word.rbegin();

  attributes(type.name, affix += *j, tp, _add_state, _add_trans);
  if (++j == word.rend()) return;
  attributes(type.name, affix += *j, tp, _add_state, _add_trans);
  if (++j == word.rend()) return;
  attributes(type.name, affix += *j, tp, _add_state, _add_trans);
  if (++j == word.rend()) return;
  attributes(type.name, affix += *j, tp, _add_state, _add_trans);
}

void SuffixGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  Raw affix, word = sent.words[i];
  std::string::reverse_iterator j = word.rbegin();

  attributes(type.name, affix += *j, c);
  if (++j == word.rend()) return;
  attributes(type.name, affix += *j, c);
  if (++j == word.rend()) return;
  attributes(type.name, affix += *j, c);
  if (++j == word.rend()) return;
  attributes(type.name, affix += *j, c);
}

void SuffixGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  Raw affix, word = sent.words[i];
  std::string::reverse_iterator j = word.rbegin();

  _add_features(dict.get(type, affix += *j), dist);
  if (++j == word.rend()) return;
  _add_features(dict.get(type, affix += *j), dist);
  if (++j == word.rend()) return;
  _add_features(dict.get(type, affix += *j), dist);
  if (++j == word.rend()) return;
  _add_features(dict.get(type, affix += *j), dist);
}

ShapeGen::ShapeGen(AffixDict &dict, const bool add_state, const bool add_trans)
  : FeatureGen(add_state, add_trans), dict(dict), shape() { }

Attribute &ShapeGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void ShapeGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  attributes(type.name, shape(sent.words[i]), tp, _add_state, _add_trans);
}

void ShapeGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  attributes(type.name, shape(sent.words[i]), c);
}

void ShapeGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  _add_features(dict.get(type, shape(sent.words[i])), dist);
}

OffsetShapeGen::OffsetShapeGen(AffixDict &dict, const int offset, const bool add_state, const bool add_trans)
  : OffsetGen(offset, add_state, add_trans), dict(dict), shape() { }

Attribute &OffsetShapeGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void OffsetShapeGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  const Raw *raw = _get_raw(sent.words, i);
  if (raw != &Sentinel::str)
    attributes(type.name, shape(*raw), tp, _add_state, _add_trans);
}

void OffsetShapeGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  const Raw *raw = _get_raw(sent.words, i);
  if (raw != &Sentinel::str)
    attributes(type.name, shape(*raw), c);
}

void OffsetShapeGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  const Raw *raw = _get_raw(sent.words, i);
  if (raw != &Sentinel::str)
    _add_features(dict.get(type, shape(*raw)), dist);
}

PosGen::PosGen(TagSetDict &dict, const bool add_state, const bool add_trans)
  : FeatureGen(add_state, add_trans), dict(dict) { }

Attribute &PosGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void PosGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  attributes(type.name, sent.pos[i], tp, _add_state, _add_trans);
}

void PosGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  attributes(type.name, sent.pos[i], c);
}

void PosGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  _add_features(dict.get(type, sent.pos[i]), dist);
}

OffsetWordGen::OffsetWordGen(WordDict &dict, const int offset, const bool add_state, const bool add_trans)
  : OffsetGen(offset, add_state, add_trans), dict(dict) { }

Attribute &OffsetWordGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void OffsetWordGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  attributes(type.name, *_get_raw(sent.words, i), tp, _add_state, _add_trans);
}

void OffsetWordGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  attributes(type.name, *_get_raw(sent.words, i), c);
}

void OffsetWordGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  _add_features(dict.get(type, *_get_raw(sent.words, i)), dist);
}

OffsetPosGen::OffsetPosGen(TagSetDict &dict, const int offset, const bool add_state, const bool add_trans)
  : OffsetGen(offset, add_state, add_trans), dict(dict) { }

Attribute &OffsetPosGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void OffsetPosGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  attributes(type.name, *_get_raw(sent.pos, i), tp, _add_state, _add_trans);
}

void OffsetPosGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  attributes(type.name, *_get_raw(sent.pos, i), c);
}

void OffsetPosGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  _add_features(dict.get(type, *_get_raw(sent.pos, i)), dist);
}

/**
 * _get_raw.
 * Utility function to generate the feature value for a bigram feature, and
 * store it in the reference raw string.
 *
 * Bigram feature values are the two elements of the bigram conjoined with a
 * space. Sentinel::str is used when the offset is out of range.
 */
void BigramGen::_get_raw(Raws &raws, Raw &raw, int i) {
  i += offset;
  if (i >= 0 && (size_t)i < raws.size()) {
    raw += raws[i++];
    raw += ' ';
    if (i >= 0 && (size_t)i < raws.size())
      raw += raws[i];
    else
      raw += Sentinel::str;
  }
  else {
    raw += Sentinel::str;
    raw += ' ';
    if (++i >= 0 && (size_t)i < raws.size())
      raw += raws[i];
    else
      raw += Sentinel::str;
  }
}

BigramWordGen::BigramWordGen(BiWordDict &dict, const int offset, const bool add_state, const bool add_trans)
  : BigramGen(offset, add_state, add_trans), dict(dict) { }

Attribute &BigramWordGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void BigramWordGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  Raw raw;
  _get_raw(sent.words, raw, i);

  attributes(type.name, raw, tp, _add_state, _add_trans);
}

void BigramWordGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  Raw raw;
  _get_raw(sent.words, raw, i);

  attributes(type.name, raw, c);
}

void BigramWordGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  const Raw *raw1, *raw2;

  if (i >= 0 && (size_t)i < sent.size()) {
    raw1 = &sent.words[i++];
    if (i >= 0 && (size_t)i < sent.size())
      raw2 = &sent.words[i];
    else
      raw2 = &Sentinel::str;
  }
  else {
    raw1 = &Sentinel::str;
    if (++i >= 0 && (size_t)i < sent.size())
      raw2 = &sent.words[i];
    else
      raw2 = &Sentinel::str;
  }

  _add_features(dict.get(type, *raw1, *raw2), dist);
}

BigramPosGen::BigramPosGen(BiTagSetDict &dict, const int offset, const bool add_state, const bool add_trans)
  : BigramGen(offset, add_state, add_trans), dict(dict) { }

Attribute &BigramPosGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void BigramPosGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  Raw raw;
  _get_raw(sent.pos, raw, i);

  attributes(type.name, raw, tp, _add_state, _add_trans);
}

void BigramPosGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  Raw raw;
  _get_raw(sent.pos, raw, i);

  attributes(type.name, raw, c);
}

void BigramPosGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  const Raw *raw1, *raw2;

  if (i >= 0 && (size_t)i < sent.size()) {
    raw1 = &sent.pos[i++];
    if (i >= 0 && (size_t)i < sent.size())
      raw2 = &sent.pos[i];
    else
      raw2 = &Sentinel::str;
  }
  else {
    raw1 = &Sentinel::str;
    if (i >= 0 && (size_t)i < sent.size())
      raw2 = &sent.pos[i];
    else
      raw2 = &Sentinel::str;
  }

  _add_features(dict.get(type, *raw1, *raw2), dist);
}

MorphGen::MorphGen(BinDict &dict, const bool add_state, const bool add_trans) :
  FeatureGen(add_state, add_trans), dict(dict) { }

Attribute &MorphGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void MorphGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  Morph morph(sent.words[i]);

  morph.add_feature(type, attributes, tp, _add_state, _add_trans);
}

void MorphGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  Morph morph(sent.words[i]);

  morph.add_feature(type, attributes, c, _add_state, _add_trans);
}

void MorphGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  _add_features(dict.get(type), dist);
}

GazGen::GazGen(GazDict &dict, Gazetteers gaz, const bool add_state, const bool add_trans) :
  FeatureGen(add_state, add_trans), dict(dict), gaz(gaz) { }

Attribute &GazGen::load(const Type &type, std::istream &in) {
  return dict.load(type, in);
}

void GazGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, TagPair tp, int i) {
  uint64_t flags = gaz.lower(sent.words[i]);
  Gazetteers::GazNames names = gaz.gaz_names();

  for (size_t i = 0; i < names.size(); ++i) {
    uint64_t flag = 1 << i;
    if (flag & flags)
      attributes(type.name, names[i], tp, _add_state, _add_trans);
  }
}

void GazGen::operator()(const Type &type, Attributes &attributes, Sentence &sent, Context &c, int i) {
  uint64_t flags = gaz.lower(sent.words[i]);
  Gazetteers::GazNames names = gaz.gaz_names();

  for (size_t i = 0; i < names.size(); ++i) {
    uint64_t flag = 1 << i;
    if (flag & flags)
      attributes(type.name, names[i], c);
  }
}

void GazGen::operator()(const Type &type, Sentence &sent, PDFs &dist, int i) {
  uint64_t flags = gaz.lower(sent.words[i]);
  Gazetteers::GazNames names = gaz.gaz_names();

  for (size_t i = 0; i < names.size(); ++i) {
    uint64_t flag = 1 << i;
    if (flag & flags)
      _add_features(dict.get(names[i]), dist);
  }
}

} }
