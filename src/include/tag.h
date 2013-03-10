#define UINT16_MAX 65535
namespace NLP {

  class Tag {
    private:
      uint16_t _id;
      uint16_t _type;

      uint16_t _check(const uint64_t id) const {
        if (id > UINT16_MAX)
          throw Exception("id too large to convert to Tag");
        return static_cast<uint16_t>(id);
      }

    public:
      Tag(void) : _id(0), _type(0) { }
      Tag(None) : _id(0), _type(0) { }
      Tag(Sentinel) : _id(1), _type(0) { }

      Tag(const uint16_t id) : _id(id), _type(0) { }
      Tag(const uint16_t id, const uint16_t type) : _id(id), _type(type) { }
      //explicit Tag(const uint64_t id) : _id(_check(id)) { }

      Tag(const Tag &other) : _id(other._id), _type(other._type) { }

      Tag &operator=(const Tag other) {
        _id = other._id;
        _type = other._type;
        return *this;
      }

      operator uint16_t(void) const { return _id; }
      inline uint16_t id(void) const { return _id; }
      inline uint16_t type(void) const { return _type; }

      inline bool operator==(const Tag &other) const  { return _id == other._id; }
      inline bool operator==(const uint16_t other) const  { return _id == other; }
      inline bool operator==(const uint64_t other) const  { return _id == other; }
      inline bool operator!=(const Tag &other) const { return _id != other._id; }

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

  };

  typedef std::vector<Tag> Tags;

  struct TagPair {
    Tag prev;
    Tag curr;

    TagPair(void) : prev(0), curr(0) { }
    TagPair(uint16_t prev, uint16_t curr) : prev(prev), curr(curr) { }
    //TagPair(Tag prev, Tag curr) : prev(prev), curr(curr) { }

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
      return *this != other;
    }

  };

  typedef std::vector<TagPair> TagPairs;
}
