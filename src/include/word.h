namespace NLP {
  class Word {
    private:
      uint64_t _id;

      uint64_t _freq(void) const;
      uint64_t _index(void) const;
      const char *_str(void) const;
    public:
      Word(void) : _id(0) { }
      Word(None) : _id(0) { }
      Word(Sentinel) : _id(1) { }

      Word(const uint64_t id) : _id(id) { }
      Word (const Word &other) : _id(other._id) { }

      Word &operator=(const Word other) {
        _id = other._id;
        return *this;
      }

      operator uint64_t(void) const { return _id; }

      uint64_t freq(void) const {
        return (_id < 2) ? 0 : _freq();
      }

      uint64_t index(void) const {
        switch (_id) {
          case 0:
            return None::val;
          case 1:
            return Sentinel::val;
          default:
            return _index();
        }
      }

      const char *str(void) const {
        switch (_id) {
          case 0:
            return None::str;
          case 1:
            return Sentinel::str;
          default:
            return _str();
        }
      }
  };
}
