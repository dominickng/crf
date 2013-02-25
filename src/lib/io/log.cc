#include "base.h"

#include "io/log.h"

namespace NLP {

Log::Log(const std::string &uri, std::ostream &output) :
  uri(uri), out(&output), file(0) {
    if (uri.size()) {
      file = new std::ofstream(uri.c_str());
      if (!*file)
        throw IOException("could not open log fil e for writing", uri);
    }
}

Log::~Log(void) {
  if (file)
    delete file;
}

}
