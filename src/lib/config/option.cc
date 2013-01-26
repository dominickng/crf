#include "base.h"
#include "config/base.h"
#include "config/group.h"
#include "config/option.h"

namespace Util { namespace config {

OpBase::OpBase(OpGroup &group, const std::string &name, const std::string &desc,
    const bool has_default, const bool requires_arg) :
  OptionBase(name, desc, requires_arg), _has_default(has_default) {
  group.add(this);
}

OptionBase *OpBase::process(const std::string &, const std::string &key) {
  if (key.empty() || key != _name)
    return NULL;
  return this;
}

void OpBase::set(const std::string &value) {
  _set(value);
  _is_set = true;
}

void OpBase::validate(void) {
  if (!_is_set) {
    if (_has_default) {
      set_default();
      _is_set = true;
    }
    else
      throw ConfigException("Required argument not set", _name);
  }
  _validate();
}

const char *const OpInput::STDIN = "stdin";

OpInput::~OpInput(void) {
  if (!_is_stdin)
    delete _in;
}

void OpInput::_validate(void) {
  if (_value == STDIN) {
    _is_stdin = true;
    _in = &std::cin;
  }
  else {
    _is_stdin = false;
    _in = new std::ifstream(_value.c_str());
    if (!*_in)
      throw ConfigException("Could not open file for reading", _name, _value);
  }
}

const char *const OpOutput::STDOUT = "stdout";

OpOutput::~OpOutput(void) {
  if (!_is_stdout)
    delete _out;
}

void OpOutput::_validate(void) {
  if (_value == STDOUT) {
    _is_stdout = true;
    _out = &std::cout;
  }
  else {
    _is_stdout = false;
    _out = new std::ofstream(_value.c_str());
    if (!_out)
      throw ConfigException("Could not open file for writing", _name, _value);
  }
}

} }
