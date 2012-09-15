namespace NLP {

  class Tag {
    private:
      uint16_t _id;

      uint16_t _check(const uint64_t id) const {
        if (id > UINT16_MAX)
          throw Exception("id too large to convert to Tag");
        return static_cast<uint16_t>(id);
      }

    public:
      Tag(void) : _id(0) { }
      Tag(None) : _id(0) { }
      Tag(Sentinel) : _id(1) { }

      Tag(const uint16_t id) : _id(id) { }
      explicit Tag(const uint64_t id) : _id(_check(id)) { }

      Tag(const Tag &other) : _id(other._id) { }

      Tag &operator=(const Tag other) {
        _id = other._id;
        return *this;
      }

      operator uint16_t(void) const { return _id; }
      inline uint16_t id(void) const { return _id; }

      inline bool operator==(const Tag &other) { return _id == other._id; }
      inline bool operator!=(const Tag &other) { return _id != other._id; }

      inline Tag& operator++(void) {
        ++_id;
        return *this;
      }
      inline Tag operator++(int) {
        _id++;
        return *this;
      }

      inline Tag& operator--(void) {
        --_id;
        return *this;
      }
      inline Tag operator--(int) {
        _id--;
        return *this;
      }

      inline bool operator<(const size_t x) { return _id < x; }
      inline bool operator<=(const size_t x) { return _id <= x; }

  };

  typedef std::vector<Tag> Tags;

  struct TagPair {
    Tag prev;
    Tag curr;

    TagPair(void) : prev((uint16_t)0), curr((uint16_t)0) { }
    TagPair(Tag prev, Tag curr) : prev(prev), curr(curr) { }

    size_t index(const size_t ntags) const {
      return prev.id() * ntags + curr.id();
    }

    static size_t npairs(const size_t ntags) {
      return ntags * ntags;
    }

    uint16_t prev_id(void) const { return prev.id(); }
    uint16_t curr_id(void) const { return curr.id(); }

    inline bool operator==(const TagPair &other) const {
      return prev == other.prev && curr == other.curr;
    }
    inline bool operator!=(const TagPair &other) const {
      return prev != other.prev || curr != other.curr;
    }

  };

  typedef std::vector<TagPair> TagPairs;
}
