namespace NLP {
  namespace CRF {
    class Shape {
      private:
        std::string _buffer;

        void _add(const char *word) {
          for (const char *w = word; *w; ++w) {
            if (islower(*w)) {
              if (*_buffer.rbegin() != 'a')
                _buffer += 'a';
            }
            else if (isupper(*w)) {
              if (*_buffer.rbegin() != 'A')
                _buffer += 'A';
            }
            else {
              switch (*w) {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': if (*_buffer.rbegin() != '0') _buffer += '0'; break;
                case '-':
                case ':': if(*_buffer.rbegin() != '-') _buffer += '-'; break;
                case '.':
                case '?':
                case '!': if(*_buffer.rbegin() != '.') _buffer += '.'; break;
                case ',':
                case ';': if(*_buffer.rbegin() != ',') _buffer += ','; break;
                default: if(*_buffer.rbegin() != *w) _buffer += *w; break;
              }
            }
          }
        }
      public:
        Shape(void) : _buffer(1024, '\0') { }

        const std::string &operator()(const char *word) {
          _buffer.clear();
          _add(word);
          return _buffer;
        }

        const std::string &operator()(const std::string &word) {
          return (*this)(word.c_str());
        }

        const std::string &operator()(const char *w1, const char *w2) {
          _buffer.clear();
          _add(w1);
          _buffer += '_';
          _add(w2);
          return _buffer;
        }

        const std::string &operator()(const std::string &w1, const std::string &w2) {
          return (*this)(w1.c_str(), w2.c_str());
        }

        const std::string &operator()(const char *w1, const char *w2, const char *w3) {
          _buffer.clear();
          _add(w1);
          _buffer += '_';
          _add(w2);
          _buffer += '_';
          _add(w3);
          return _buffer;
        }

        const std::string &operator()(const std::string &w1, const std::string &w2, const std::string &w3) {
          return (*this)(w1.c_str(), w2.c_str(), w3.c_str());
        }
    };
  }
}
