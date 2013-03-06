#include "base.h"

#include "config.h"
#include "crf/features/types.h"

namespace NLP { namespace CRF {

const Type Types::ppw = {"prev prev word", "ppw", 0};
const Type Types::pw = {"prev word", "pw", 1};
const Type Types::w = {"word feature", "w", 2};
const Type Types::nw = {"next word", "nw", 3};
const Type Types::nnw = {" next next word", "nnw", 4};

const Type Types::ppw_pw = {"prev 2 words", "ppw_pw", 0};
const Type Types::pw_w = {"prev word + curr word", "pw_w", 1};
const Type Types::w_nw = {"word + next word", "w_nw", 2};
const Type Types::nw_nnw = {"next 2 words", "nw_nnw", 3};

const Type Types::ppp = {"prev prev pos", "ppp", 0};
const Type Types::pp = {"prev pos", "pp", 1};
const Type Types::p = {"pos feature", "p", 2};
const Type Types::np = {"next pos", "np", 3};
const Type Types::nnp = {" next next pos", "nnp", 4};

const Type Types::ppp_pp = {"prev 2 pos", "ppp_pp", 0};
const Type Types::pp_p = {"prev pos + curr pos", "pp_p", 1};
const Type Types::p_np = {"pos + next pos", "p_np", 2};
const Type Types::np_nnp = {"next 2 pos", "np_nnp", 3};

const Type Types::ppc = {"prev prev chunk", "ppc", 0};
const Type Types::pc = {"prev chunk", "pc", 1};
const Type Types::c = {"chunk feature", "c", 2};
const Type Types::nc = {"next chunk", "nc", 3};
const Type Types::nnc = {" next next chunk", "nnc", 4};

const Type Types::ppc_pc = {"prev 2 chunks", "ppc_pc", 0};
const Type Types::pc_c = {"prev chunk + curr chunk", "pc_c", 1};
const Type Types::c_nc = {"chunk + next chunk", "c_nc", 2};
const Type Types::nc_nnc = {"next 2 chunks", "nc_nnc", 3};

const Type Types::prefix = {"prefix", "prefix", 0};
const Type Types::suffix = {"suffix", "suffix", 1};

const size_t Types::nmorph = 18;
const Type Types::has_digit = {"contains a digit", "has_digit", 0};
const Type Types::has_hyphen = {"contains a hyphen", "has_hyphen", 1};
const Type Types::has_period = {"contains a period", "has_period", 2};
const Type Types::has_punct = {"contains punctuation", "has_punct", 3};
const Type Types::has_uppercase = {"contains a uppercase", "has_uppercase", 4};
const Type Types::kase = {"word case feature", "kase", 5};
const Type Types::digits = {"digits feature", "digits", 6};
const Type Types::number = {"number feature", "number", 7};
const Type Types::alpha = {"alpha feature", "alpha", 8};
const Type Types::alnum = {"alnum feature", "alnum", 9};
const Type Types::length = {"length feature", "length", 10};
const Type Types::roman = {"roman feature", "roman", 11};
const Type Types::initial = {"initial feature", "initial", 12};
const Type Types::acronym = {"acronym feature", "acronym", 13};
const Type Types::uppercase = {"uppercase feature", "uppercase", 14};
const Type Types::lowercase = {"lowercase feature", "lowercase", 15};
const Type Types::titlecase = {"titlecase feature", "titlecase", 16};
const Type Types::mixedcase = {"mixed case feature", "mixedcase", 17};

const Type Types::pps = {"prev prev shape", "pps", 0};
const Type Types::ps = {"prev shape", "ps", 1};
const Type Types::s = {"shape feature", "s", 2};
const Type Types::ns = {"next shape", "ns", 3};
const Type Types::nns = {" next next shape", "nns", 4};

const Type Types::pps_ps = {"prev 2 shapes", "pps_ps", 0};
const Type Types::ps_s = {"prev shape + curr shape", "ps_s", 1};
const Type Types::s_ns = {"shape + next shape", "s_ns", 2};
const Type Types::ns_nns = {"next 2 shapes", "ns_nns", 3};

const Type Types::pgaz = {"prev gazetteer feature", "pgaz", 0};
const Type Types::gaz = {"gazetteer feature", "gaz", 0};
const Type Types::ngaz = {"next gazetteer feature", "ngaz", 0};

const Type Types::gaz_common = {"common word gazetteer feature", "gaz_common", 0};
const Type Types::gaz_first = {"first name gazetteer feature", "gaz_first", 1};
const Type Types::gaz_last = {"last name gazetteer feature", "gaz_last", 2};

const Type Types::trans = {"transition feature", "trans", 0};

Types::Types(void) :
  config::OpGroup("types", "Tagger feature types", true),
  use_words(*this, "use_words", "use words features", true, true, true),
  use_prev_words(*this, "use_prev_words", "use prev words features", true, true, true),
  use_next_words(*this, "use_next_words", "use next words features", true, true, true),
  use_word_bigrams(*this, "use_word_bigrams", "use word bigrams features", true, true, true),
  use_pos(*this, "use_pos", "use pos features", true, true, true),
  use_prev_pos(*this, "use_prev_pos", "use prev pos features", true, true, true),
  use_next_pos(*this, "use_next_pos", "use next pos features", true, true, true),
  use_pos_bigrams(*this, "use_pos_bigrams", "use pos bigrams features", true, true, true),

  use_chunks(*this, "use_chunks", "use chunks features", true, true, true),
  use_prev_chunks(*this, "use_prev_chunks", "use prev chunks features", true, true, true),
  use_next_chunks(*this, "use_next_chunks", "use next chunks features", true, true, true),
  use_chunk_bigrams(*this, "use_chunk_bigrams", "use chunk bigrams features", true, true, true),

  use_prefix(*this, "use_prefix", "use prefix features", true, true, true),
  use_suffix(*this, "use_suffix", "use suffix features", true, true, true),

  use_morph(*this, "use_morph", "use morphological features", true, true, true),

  use_shape(*this, "use_shape", "use shape features", true, true, true),
  use_prev_shape(*this, "use_prev_shape", "use prev shape features", true, true, true),
  use_next_shape(*this, "use_next_shape", "use next shape features", true, true, true),
  use_shape_bigram(*this, "use_shape_bigram", "use shape bigram features", true, true, true),

  use_gaz(*this, "use_gaz", "use gaz features", true, true, true),
  use_prev_gaz(*this, "use_prev_gaz", "use prev gaz features", true, true, true),
  use_next_gaz(*this, "use_next_gaz", "use next gaz features", true, true, true),

  use_trans(*this, "use_trans", "use transition features", true, true, true)
  { }

} }
