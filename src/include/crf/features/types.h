namespace config = Util::config;

namespace NLP {
  namespace CRF {
      struct Type {
        const char *desc;
        const char *name;
        uint64_t index;

        bool equals(const Type &other) const {
          return name == other.name;
        }
      };

      class Types : public config::OpGroup {
        public:
          //word types
          const static Type w;

          const static Type pw;
          const static Type ppw;
          const static Type nw;
          const static Type nnw;

          const static Type ppw_pw;
          const static Type pw_w;
          const static Type w_nw;
          const static Type nw_nnw;

          //pos types
          const static Type p;
          const static Type pp;
          const static Type ppp;
          const static Type np;
          const static Type nnp;

          const static Type ppp_pp;
          const static Type pp_p;
          const static Type p_np;
          const static Type np_nnp;

          //chunk types
          const static Type c;
          const static Type pc;
          const static Type ppc;
          const static Type nc;
          const static Type nnc;

          const static Type ppc_pc;
          const static Type pc_c;
          const static Type c_nc;
          const static Type nc_nnc;

          //prefix and suffix
          const static Type prefix;
          const static Type suffix;

          //orthographic and morphological
          const static size_t nmorph;
          const static Type has_digit;
          const static Type has_hyphen;
          const static Type has_period;
          const static Type has_punct;
          const static Type has_uppercase;
          const static Type kase;
          const static Type digits;
          const static Type number;
          const static Type alpha;
          const static Type alnum;
          const static Type length;
          const static Type roman;
          const static Type initial;
          const static Type acronym;
          const static Type uppercase;
          const static Type lowercase;
          const static Type titlecase;
          const static Type mixedcase;

          //shapes
          const static Type s;
          const static Type ps;
          const static Type pps;
          const static Type ns;
          const static Type nns;

          const static Type pps_ps;
          const static Type ps_s;
          const static Type s_ns;
          const static Type ns_nns;

          const static Type pps_ps_s;
          const static Type ps_s_ns;
          const static Type s_ns_nns;

          //gazetteers
          const static Type pgaz;
          const static Type gaz;
          const static Type ngaz;

          const static Type gaz_common;
          const static Type gaz_first;
          const static Type gaz_last;

          //transition
          const static Type trans;

          config::Op<bool> use_words;
          config::Op<bool> use_prev_words;
          config::Op<bool> use_next_words;
          config::Op<bool> use_word_bigrams;

          config::Op<bool> use_pos;
          config::Op<bool> use_prev_pos;
          config::Op<bool> use_next_pos;
          config::Op<bool> use_pos_bigrams;

          config::Op<bool> use_chunks;
          config::Op<bool> use_prev_chunks;
          config::Op<bool> use_next_chunks;
          config::Op<bool> use_chunk_bigrams;

          config::Op<bool> use_prefix;
          config::Op<bool> use_suffix;

          config::Op<bool> use_morph;

          config::Op<bool> use_shape;
          config::Op<bool> use_prev_shape;
          config::Op<bool> use_next_shape;
          config::Op<bool> use_shape_bigram;

          config::Op<bool> use_gaz;
          config::Op<bool> use_prev_gaz;
          config::Op<bool> use_next_gaz;

          config::Op<bool> use_trans;

          Types(void);
          ~Types(void) { }
      };
  }
}
