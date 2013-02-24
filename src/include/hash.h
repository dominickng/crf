/**
 * hash.h
 * Provides function objects that implement various popular
 * hash algorithms for integers and strings.
 *
 * Descriptions of the hash algorithms implemented here can be found at
 * http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
 */
namespace Util {
  namespace Hasher {

    class DJBXorHash {
      private:
        const static uint64_t _INIT = 5381L;

        uint64_t _hash;
      public:
        uint64_t operator()(const char c) {
          _hash = ((_INIT << 5) ^ _INIT) ^ c;
          return _hash;
        }

        uint64_t operator()(const char *s) {
          _hash = _INIT;
          for (; *s; ++s)
            _hash = ((_hash << 5) ^ _hash) ^ *s;
          return _hash;
        }

        uint64_t operator()(const std::string &str) {
          return operator()(str.c_str());
        }

        uint64_t operator()(int32_t value) {
          _hash = ((_INIT << 5) ^ _INIT) ^ value;
          return _hash;
        }

        uint64_t operator()(uint64_t value) {
          _hash = ((_INIT << 5) ^ _INIT) ^ value;
          return _hash;
        }

        uint64_t value(void) const { return _hash; }

        explicit DJBXorHash(void) : _hash(0) {  }
        explicit DJBXorHash(const char c) { operator()(c); }
        explicit DJBXorHash(const char *s) { operator()(s); }
        explicit DJBXorHash(const std::string &str) { operator()(str); }
        explicit DJBXorHash(int32_t value) { operator()(value); }
        explicit DJBXorHash(uint64_t value) { operator()(value); }
    };

    class DJBAddHash {
      private:
        const static uint64_t _INIT = 5381L;

        uint64_t _hash;
      public:
        uint64_t operator()(const char c) {
          _hash = ((_INIT << 5) + _INIT) + c;
          return _hash;
        }

        uint64_t operator()(const char *s) {
          _hash = _INIT;
          for (; *s; ++s)
            _hash = ((_hash << 5) + _hash) + *s;
          return _hash;
        }

        uint64_t operator()(const std::string &str) {
          return operator()(str.c_str());
        }

        uint64_t operator()(int32_t value) {
          _hash = ((_INIT << 5) + _INIT) + value;
          return _hash;
        }

        uint64_t operator()(uint64_t value) {
          _hash = ((_INIT << 5) + _INIT) + value;
          return _hash;
        }

        uint64_t value(void) const { return _hash; }

        explicit DJBAddHash(void) : _hash(0) {  }
        explicit DJBAddHash(const char c) { operator()(c); }
        explicit DJBAddHash(const char *s) { operator()(s); }
        explicit DJBAddHash(const std::string &str) { operator()(str); }
        explicit DJBAddHash(int32_t value) { operator()(value); }
        explicit DJBAddHash(uint64_t value) { operator()(value); }
    };

    class KnuthHash {
      private:
        uint64_t _hash;
      public:
        uint64_t operator()(const char c) {
          _hash = ((1 << 5) ^ (1 >> 27)) ^ c;
          return _hash;
        }

        uint64_t operator()(const char *s) {
          _hash = strlen(s);
          for (; *s; ++s)
            _hash = ((_hash << 5) ^ (_hash >> 27)) ^ *s;
          return _hash;
        }

        uint64_t operator()(const std::string &str) {
          return operator()(str.c_str());
        }

        uint64_t operator()(int32_t value) {
          _hash = ((1 << 5) ^ (1 >> 27)) ^ value;
          return _hash;
        }

        uint64_t operator()(uint64_t value) {
          _hash = ((1 << 5) ^ (1 >> 27)) ^ value;
          return _hash;
        }

        uint64_t value(void) const { return _hash; }

        explicit KnuthHash(void) : _hash(0) {  }
        explicit KnuthHash(const char c) { operator()(c); }
        explicit KnuthHash(const char *s) { operator()(s); }
        explicit KnuthHash(const std::string &str) { operator()(str); }
        explicit KnuthHash(int32_t value) { operator()(value); }
        explicit KnuthHash(uint64_t value) { operator()(value); }
    };

    class SDBMHash {
      private:
        const static uint64_t _INIT = 0L;

        uint64_t _hash;
      public:
        uint64_t operator()(const char c) {
          _hash = c;
          return _hash;
        }

        uint64_t operator()(const char *s) {
          _hash = _INIT;
          for (; *s; ++s)
            _hash = *s + (_hash << 6) + (_hash << 16) - _hash;
          return _hash;
        }

        uint64_t operator()(const std::string &str) {
          return operator()(str.c_str());
        }

        uint64_t operator()(int32_t value) {
          _hash = value;
          return _hash;
        }

        uint64_t operator()(uint64_t value) {
          _hash = value;
          return _hash;
        }

        uint64_t value(void) const { return _hash; }

        explicit SDBMHash(void) : _hash(0) {  }
        explicit SDBMHash(const char c) { operator()(c); }
        explicit SDBMHash(const char *s) { operator()(s); }
        explicit SDBMHash(const std::string &str) { operator()(str); }
        explicit SDBMHash(int32_t value) { operator()(value); }
        explicit SDBMHash(uint64_t value) { operator()(value); }
    };

    class JSHash {
      private:
        const static uint64_t _INIT = 1315423911L;

        uint64_t _hash;
      public:
        uint64_t operator()(const char c) {
          _hash = _INIT ^ ((_INIT << 5) + c + (_INIT >> 2));
          return _hash;
        }

        uint64_t operator()(const char *s) {
          _hash = _INIT;
          for (; *s; ++s)
            _hash ^= ((_hash << 5) + *s + (_hash >> 2));
          return _hash;
        }

        uint64_t operator()(const std::string &str) {
          return operator()(str.c_str());
        }

        uint64_t operator()(int32_t value) {
          _hash = _INIT ^ ((_INIT << 5) + value + (_INIT >> 2));
          return _hash;
        }

        uint64_t operator()(uint64_t value) {
          _hash = _INIT ^ ((_INIT << 5) + value + (_INIT >> 2));
          return _hash;
        }

        uint64_t value(void) const { return _hash; }

        explicit JSHash(void) : _hash(0) {  }
        explicit JSHash(const char c) { operator()(c); }
        explicit JSHash(const char *s) { operator()(s); }
        explicit JSHash(const std::string &str) { operator()(str); }
        explicit JSHash(int32_t value) { operator()(value); }
        explicit JSHash(uint64_t value) { operator()(value); }
    };

    class FNV1aHash {
      private:
        const static uint64_t _FNV_PRIME = 1099511628211Lu;
        const static uint64_t _OFFSET_BASIS = 14695981039346656037Lu;

        uint64_t _hash;
      public:
        bool operator==(const FNV1aHash &other) const {
          return _hash == other._hash;
        }

        bool operator==(const uint64_t hash) const {
          return _hash == hash;
        }

        uint64_t operator()(const char c) {
          _hash = (_OFFSET_BASIS ^ c) * _FNV_PRIME;
          return _hash;
        }

        uint64_t operator()(const char *s) {
          _hash = _OFFSET_BASIS;
          for (; *s; ++s)
            _hash = (_hash ^ *s) * _FNV_PRIME;
          return _hash;
        }

        uint64_t operator()(const std::string &str) {
          return operator()(str.c_str());
        }

        uint64_t operator()(int32_t value) {
          _hash = _OFFSET_BASIS;
          unsigned char *begin = (unsigned char *)&value;
          unsigned char *end = begin + sizeof(int32_t);
          for(; begin != end; ++begin)
            _hash = (_hash ^ *begin) * _FNV_PRIME;
          return _hash;
        }

        uint64_t operator()(uint64_t value) {
          _hash = _OFFSET_BASIS;
          unsigned char *begin = (unsigned char *)&value;
          unsigned char *end = begin + sizeof(int32_t);
          for(; begin != end; ++begin)
            _hash = (_hash ^ *begin) * _FNV_PRIME;
          return _hash;
        }

        uint64_t value(void) const { return _hash; }

        explicit FNV1aHash(void) : _hash(0) {  }
        explicit FNV1aHash(const char c) { operator()(c); }
        explicit FNV1aHash(const char *s) { operator()(s); }
        explicit FNV1aHash(const std::string &str) { operator()(str); }
        explicit FNV1aHash(int32_t value) { operator()(value); }
        explicit FNV1aHash(uint64_t value) { operator()(value); }
    };

    typedef FNV1aHash Hash;
  }
}
