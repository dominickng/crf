#include "base.h"

#include "io/reader.h"
#include "io/reader_factory.h"
#include "io/reader_conll.h"

namespace NLP {
  ReaderFactory::ReaderFactory(const std::string &name, const std::string &uri,
      std::istream &input, const std::string &fmt)
        : Reader(uri, input), fmt(fmt), reader(0) {
        if (name == "conll")
          reader = new CoNLLReader(uri, input);
      }

  bool ReaderFactory::next(Sentence &sent) {
    return reader->next(sent);
  }
}
