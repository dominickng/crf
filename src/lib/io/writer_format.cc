#include "base.h"

#include "io/format.h"
#include "io/writer.h"
#include "io/writer_factory.h"
#include "io/writer_format.h"

namespace NLP {

bool FormatWriter::next(Sentence &sent) {
  if (!sent.size())
    return false;

  out << format.sent_pre;
  int i, j;
  for (i = 0; i < sent.size() - 1; ++i) {
    for (j = 0; j < format.fields.size() - 1; ++j)
      out << sent.get_single(format.fields[j])[i] << format.separators[j];
    out << sent.get_single(format.fields[j])[i] << format.word_sep;
  }
  for (j = 0; j < format.fields.size() - 1; ++j)
    out << sent.get_single(format.fields[j])[sent.size() - 1] << format.separators[j];
  out << sent.get_single(format.fields[j])[sent.size() - 1] << format.sent_post;

  return true;
}

}
