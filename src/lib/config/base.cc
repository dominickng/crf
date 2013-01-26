#include "base.h"
#include "config/base.h"

namespace Util { namespace config {

const char *
ConfigException::what(void) const throw() {
  std::stringstream ss;
  ss << msg;
  if (!name.empty()) {
    ss << " for option \"" << name << "\"";
    if (!value.empty())
      ss << " (value \"" << value << "\")";
  }
  return ss.str().c_str();
}


OptionBase::OptionBase(const std::string &name, const std::string &desc,
    const bool requires_arg) : _name(name), _desc(desc),
      _requires_arg(requires_arg), _is_set(false) {
  if (name.empty())
    throw ConfigException("Option names cannot be empty", _name);

  if (name.size() > 1) {
    for (int i = 0; i < name.size(); ++i) {
      if (name[i] == '-')
        throw ConfigException("Option names cannot contain dashes", name);
      if (std::isspace(name[i]))
        throw ConfigException("Option names cannot contain spaces", name);
    }
  }
}

} }
