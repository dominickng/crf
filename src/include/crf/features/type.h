namespace NLP {
  namespace CRF {
      struct Type {
        const char *desc;
        const char *id;
        uint64_t index;

        bool equals(const Type &other) const {
          return index == other.index;
        }
      };

      namespace Types {
        extern const Type words;
        extern const Type pos;
        extern const Type prevword;
        extern const Type prevprevword;
        extern const Type nextword;
        extern const Type nextnextword;
        extern const Type prevpos;
        extern const Type prevprevpos;
        extern const Type nextpos;
        extern const Type nextnextpos;
      }
  }
}
