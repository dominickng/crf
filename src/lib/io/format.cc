#include "base.h"

#include "io/format.h"

namespace NLP {

void Format::_parse_escape(const char **str, std::string &dest) {
  const char *s = *str;
  if (s[0] == '\\') {
    if (s[1] == 't')
      dest += '\t';
    else if (s[1] == 'n')
      dest += '\n';
    else if (s[1] == 'r')
      dest += '\r';
    else
      throw FormatException("bad separator character following '\\'", s[1]);
    ++(*str);
  }
  else
    dest += *s;
}

void Format::parse_chain(const std::string &format) {
  const char *s = format.c_str();

  if (!*s)
    throw Exception("format string must contain at least one field");

  while (*s) {
    if (s[0] != '%')
      break;
    if (!s[1])
      throw Exception("unexpected end of format string after %");
    if (s[1] == '%')
      break;
    if (Sentence::type(s[1]) == Sentence::TYPE_INVALID)
      throw FormatException("unrecognised format string specifier %", s[1]);

    fields += s[1];
    if (!s[2])
      break;

    s += 2;
    if (s[0] == '%') {
      if (s[1] == '%')
        ++s;
      else
        throw Exception("missing separator after %");
    }
    _parse_escape(&s, separators);
    ++s;
  }
}


void Format::parse(const std::string &format) {
  const char *s = format.c_str();

  for (; *s; ++s) {
    if (s[0] == '%') {
      if (s[1] == '%')
        ++s;
      else
        break;
    }
    _parse_escape(&s, sent_pre);
  }

  if (!*s)
    throw Exception("format string must contain at least one field");

  while (*s) {
    if (s[0] != '%')
      break;
    if (!s[1])
      throw Exception("unexpected end of format string after %");
    if (s[1] == '%')
      break;
    if (Sentence::type(s[1]) == Sentence::TYPE_INVALID)
      throw FormatException("unrecognised format string specifier %", s[1]);

    fields += s[1];
    if (!s[2])
      throw FormatException("format string is missing separator after %", s[1]);

    s += 2;
    if (s[0] == '%') {
      if (s[1] == '%')
        ++s;
      else
        throw Exception("missing separator after %");
    }
    _parse_escape(&s, separators);
    ++s;
  }

  word_sep = separators[separators.size() - 1];
  separators.erase(separators.size() - 1);

  if (!s[0])
    throw Exception("sentence separator is missing in format string");

  for (; *s; ++s) {
    if (s[0] == '%') {
      if (s[1] == '%')
        ++s;
      else
        break;
    }
    _parse_escape(&s, sent_post);
  }
}

}
