/**
 * taglimits.h
 * Wraps a vector of tagpairs that stores the limits of each type of tag
 * in the factorial CRF
 */

namespace NLP {
  class TagLimits {
    public:
      TagLimits(void) : _limits(), _ntags(0), _max_index(0) { }
      ~TagLimits(void) { }

      const TagPair &operator[](const size_t index) const {
        return _limits[index];
      }

      TagPair &operator[](const size_t index) {
        return _limits[index];
      }

      void push_back(const uint16_t begin, const uint16_t end) {
        _limits.push_back(TagPair(begin, end));
      }

      void calc(const TagSet tags) {
        uint16_t prev_type = 0;
        uint16_t prev_id = 0;
        size_t i = 0;
        _ntags = tags.size();
        for (; i < _ntags; ++i) {
          Tag t = tags[i];
          if (t.type() != prev_type) {
            push_back(prev_id, i+1);
            prev_id = i;
          }
          prev_type = t.type();
        }
        push_back(prev_id, i);
      }

      void print(std::ostream &out) const {
        for (size_t i = 0; i < _limits.size(); ++i)
          std::cout << i << ' ' << _limits[i].prev << ' ' << _limits[i].curr << std::endl;
      }

      size_t max_index(void) {
        if (_max_index == 0) {
          _max_index = 1;
          for (size_t i = 0; i < _limits.size(); ++i)
            _max_index *= ntags(i);
          ++_max_index;
        }
        return _max_index;
      }

      inline size_t ntypes(void) const {
        return _limits.size();
      }

      inline size_t ntags(void) const {
        return _ntags;
      }

      inline size_t ntags(size_t type) const {
        return _limits[type].curr - _limits[type].prev;
      }

      size_t nskip(size_t type) {
        size_t skip = 1;
        for (int i = type; i < ntypes(); ++i)
          skip *= ntags(i);
        return skip;
      }

    private:
      TagPairs _limits;
      size_t _ntags;
      size_t _max_index;
  };
}
