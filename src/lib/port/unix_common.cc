#include "std.h"
#include "exception.h"
#include "port.h"

#include <cerrno>
#include <sys/resource.h>
#include <sys/stat.h>

namespace Util { namespace port {

const char PATH_SEP = '/';

void make_directory(const std::string &dir) {
  if (mkdir(dir.c_str(), 0755)){
    if(errno == EEXIST)
      std::cerr << "using existing directory " << dir << std::endl;
    else
      throw IOException("could not create directory", dir);
  }
}

} }
