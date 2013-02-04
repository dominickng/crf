namespace NLP {
  struct Sentence {
    Raws words;
    Raws pos;
    Raws chunks;
    Raws entities;

    static const int NMISC = 10;
    static const int TYPE_INVALID = 0;
    static const int TYPE_OPTIONAL = 1;
    static const int TYPE_IGNORE= 2;
    static const int TYPE_SINGLE = 3;

    Raws misc[NMISC];

    static int type(const char c) {
      switch (c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': return TYPE_OPTIONAL;
        case 'w':
        case 'p':
        case 'e': return TYPE_SINGLE;
        case '?': return TYPE_IGNORE;
        default: return TYPE_INVALID;
      }
    }

    template <typename T>
    void reset(T &v) {
      if (!v.empty())
        v.clear();
    }

    void reset(void) {
      reset(words);
      reset(pos);
      reset(chunks);
      reset(entities);

      for (int i = 0; i < NMISC; ++i)
        reset(misc[i]);
    }

    Raws &get_single(char c) {
      switch (c) {
        case 'w':
          return words;
        case 'p':
          return pos;
        case 'c':
          return chunks;
        case 'e':
          return entities;
        default:
          return words;
      }
    }

    size_t size(void) {
      return words.size();
    }

  };
}
