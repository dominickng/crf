#include "base.h"

#include "io/format.h"
#include "io/reader.h"
#include "io/reader_factory.h"
#include "io/reader_format.h"

namespace NLP {

  FormatReader::FormatReader(const std::string &uri, std::istream &in, const std::string &fmt)
    : Reader(uri, in), format(fmt), preface(), _nlines(0), _len(0) {
      read_preface(uri, in, preface, _nlines, false);
  }

  bool FormatReader::next_line(void) {
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

  bool FormatReader::next(Sentence &sent) {
    if (!next_line())
      return false;

    char *begin = _buffer;
    char *current = begin;
    char *end = _buffer + _len;

    int index = 0;
    //TODO more error handling
    while (*current && current != end) {
      if (*current == format.word_sep) {
        *current = '\0';
        sent.get_single(format.fields[index]).push_back(begin);
        begin = ++current;
        index = 0;
      }
      else if (*current == format.separators[index]) {
        *current = '\0';
        sent.get_single(format.fields[index++]).push_back(begin);
        begin = ++current;
      }
      else
        ++current;
    }
    sent.get_single(format.fields[index]).push_back(begin);
    return true;
  }

  void FormatReader::reset(void) {
    Reader::reset();
    _nlines = 0;
    _len = 0;
    _buffer[0] = '\0';
  }

}
