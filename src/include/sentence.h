namespace NLP {
  struct Sentence {
    Raws words;
    Raws pos;
    Raws chunks;
    Raws entities;

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
    }

    Raws &get_single(char c) {
      switch (c) {
        case 'w':
          return words;
        case 'p':
          return pos;
        case 'c':
          return chunks;
        case 'n':
          return entities;
        default:
          return words;
      }
    }
  };
}
