#include "base.h"
#include "config/base.h"
#include "config/group.h"
#include "config/option.h"
#include "config/info.h"

namespace Util { namespace config {

void Info::save(const std::string &preface) {
  std::string path = base() + "/info";
  std::ofstream out(path.c_str());

  if (!out)
    throw ConfigException("cannot open file for writing", path);
  out << preface << '\n';

  save(out, "");
}

void Info::save(std::ostream &out, const std::string &prefix) {
  for (std::vector<OptionBase *>::const_iterator child = _children.begin(); child != _children.end(); ++child) {
    (*child)->save(out, "");
    out << '\n';
  }
}

bool Info::read_config(void) {
  std::string preface;
  std::string path = base() + "/info";
  uint64_t nlines;
  std::ifstream in(path.c_str());

  if (!in)
    throw ConfigException("cannot open info file for reading", path);

  NLP::read_preface("info file", in, preface, nlines);
  load(in);
  return true;
}

} }
