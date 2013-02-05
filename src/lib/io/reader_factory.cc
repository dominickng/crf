#include "base.h"

#include "io/format.h"
#include "io/reader.h"
#include "io/reader_factory.h"
#include "io/reader_format.h"
#include "io/reader_conll.h"

namespace NLP {
  ReaderFactory::ReaderFactory(const std::string &name, const std::string &uri,
      std::istream &in, const std::string &fmt)
        : Reader(uri, in), fmt(fmt), reader(0) {
        if (name == "format")
          reader = new FormatReader(uri, in, fmt);
        else if (name == "conll")
          reader = new CoNLLReader(uri, in);
      }

  bool ReaderFactory::next(Sentence &sent) {
    return reader->next(sent);
  }
}
