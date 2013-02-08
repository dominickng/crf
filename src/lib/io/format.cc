#include "base.h"

#include "io/format.h"

namespace NLP {

void Format::parse(const std::string &format) {
  const char *s = format.c_str();

  for (; *s; ++s) {
    if (s[0] == '%') {
      if (s[1] == '%')
        ++s;
      else
        break;
    }
    sent_pre += *s;
  }

  if (!*s)
    throw ValueException("format string must contain at least one field");

  for (; *s; s += 3) {
    if (s[0] != '%')
      break;
    if (!s[1])
      throw ValueException("unexpected end of format string after %");
    if (s[1] == '%')
      break;
    if (Sentence::type(s[1]) == Sentence::TYPE_INVALID)
      throw FormatException("unrecognised format string specifier %", s[1]);
    fields += s[1];
    if (!s[2])
      throw FormatException("format string is missing separator after %", s[1]);

    if (s[2] == '%') {
      if (s[3] == '%')
        ++s;
      else
        throw ValueException("missing separator after %");
    }
    separators += s[2];
  }

  word_sep = separators[separators.size() - 1];
  separators.erase(separators.size() - 1);

  if (!s[0])
    throw ValueException("sentence separator is missing in format string");

  for (; *s; ++s) {
    if (s[0] == '%') {
      if (s[1] == '%')
        ++s;
      else
        break;
    }
    sent_post += *s;
  }
}

}
