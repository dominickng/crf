#include "base.h"

#include "io/format.h"
#include "io/writer.h"
#include "io/writer_factory.h"
#include "io/writer_format.h"

namespace NLP {
  WriterFactory::WriterFactory(const std::string &name, const std::string &uri,
      std::ostream &out, const std::string &fmt)
        : Writer(uri, out), fmt(fmt), writer(0) {
        if (name == "format")
          writer = new FormatWriter(uri, out, fmt);
  }

  WriterFactory::~WriterFactory(void) {
    delete writer;
  }

  bool WriterFactory::next(Sentence &sent) {
    return writer->next(sent);
  }
}
