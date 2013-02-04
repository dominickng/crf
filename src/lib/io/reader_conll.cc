#include "base.h"

#include "io/reader.h"
#include "io/reader_factory.h"
#include "io/reader_conll.h"

namespace NLP {

  CoNLLReader::CoNLLReader(const std::string &uri, std::istream &in)
    : Reader(uri, in), _nlines(0), _len(0) { }

  bool CoNLLReader::next_line(void) {
    in.getline(_buffer, sizeof(_buffer), '\n');
    _len = in.gcount();

    if (in.eof() && _len == 0)
      return false;

    if (_buffer[_len - 1] == '\x0d')
      _buffer[_len - 1] = '\0';

    ++_nlines;

    if (!in)
      throw IOException("unexpected input reading problem");

    return true;
  }

  bool CoNLLReader::next(Sentence &sent) {
    if (!next_line())
      return false;

    char cols[4] = { 'w', 'p', 'c', 'e', };
    while (_len != 1) {
      char *begin = _buffer;
      char *current = begin;
      char *end = _buffer + _len;

      int index = 0;

      //TODO more error handling
      while (current != end) {
        if (isspace(*current)) {
          *current = '\0';
          sent.get_single(cols[index++]).push_back(begin);
          begin = ++current;
        }
        else
          ++current;
      }
      sent.get_single(cols[index++]).push_back(begin);
      next_line();
    }
    return true;
  }

  void CoNLLReader::reset(void) {
    Reader::reset();
    _nlines = 0;
    _len = 0;
    _buffer[0] = '\0';
  }

}
