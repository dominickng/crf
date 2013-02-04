namespace Util {
  namespace config {
    class OpBase : public OptionBase {
      // represent ops that take a required argument
      // must be part of some OpGroup
      protected:
        const bool _has_default;

        virtual void _set(const std::string &value) = 0;
        virtual void _validate(void) = 0;

      public:
        friend class OpAlias;
        OpBase(OpGroup &group, const std::string &name,
            const std::string &desc, const bool has_default,
            const bool requires_arg=true);

        virtual ~OpBase(void) { }

        virtual OptionBase *process(const std::string &orig_key, const std::string &key);

        virtual void set(const std::string &value);
        virtual void set_default(void) = 0;
        virtual void validate(void);

        inline bool has_default(void) const { return _has_default; }
    };

    template <typename T>
    class Op : public OpBase {
      // templated class of ops that take args of a particular type
      protected:

        virtual void _set(const std::string &value) {
          std::istringstream s(value);
          if (!(s >> _value))
            throw ConfigException("Invalid value", _name, value);
        }

        virtual void _validate(void) { }

      public:
        const T _default;
        T _value;

        Op(OpGroup &group, const std::string &name, const std::string &desc,
            const bool requires_arg=true) :
          OpBase(group, name, desc, false, requires_arg), _default(), _value() { }

        Op(OpGroup &group, const std::string &name, const std::string &desc,
            const T &default_value, const bool requires_arg=true) :
          OpBase(group, name, desc, true, requires_arg),
          _default(default_value), _value() { }

        virtual ~Op(void) { }

        virtual void help(std::ostream &out, const std::string &prefix,
            const unsigned int depth) const {
          out << prefix << _name << " <arg>: " << _desc;

          if (_has_default)
            out << " (def=\"" << _default << "\")";
          if (OpBase::_requires_arg)
            out << " [required]";
          out << std::endl;
        }

        virtual void set_default(void) {
          if (!OpBase::_has_default)
            throw ConfigException("Option has no default value", Op<T>::_name);
          _value = _default;
        }

        inline void operator()(const T &v) { _value = v; }
        inline const T &operator()(void) const { return _value; }

        virtual void save(std::ostream &out, const std::string &prefix) {
          out << _name << " = " << _value << '\n';
        }

        virtual void load(std::istream &in) {
          in >> _value;
        }

    };

    template <> inline void Op<std::string>::_set(const std::string &value) {
      _value = value;
    }

    class OpFlag : public Op<bool> {
      protected:
        virtual void _set(const std::string &) { _value = !_default; }

        virtual void help(std::ostream &out, const std::string &prefix,
            const unsigned int depth) const {
          out << prefix << _name << ": " << _desc;

          if (_has_default)
            out << " (def=\"" << (_default ? "true" : "false") << "\")";
          if (Op<bool>::_requires_arg)
            out << " [required]";
          out << std::endl;
        }

      public:
        OpFlag(OpGroup &group, const std::string &name,
            const std::string &desc, const bool default_value=false) :
          Op<bool>(group, name, desc, default_value, false) { }

        virtual ~OpFlag(void) { }
    };

    template <typename T>
    class OpRestricted : public Op<T> {
      // enumerable options, specified by default as a sep-delimited list
      // in the constructor
      protected:
        std::vector<T> _options;
        const std::string _options_string;
        bool _options_decoded;
        const char _sep;

        virtual void _check(const T &value) const {
          if (find(_options.begin(), _options.end(), value) == _options.end())
            throw ConfigException("Invalid value", Op<T>::_name, value);
        }

        virtual void _decode_options(std::istream &s) {
          T option;
          std::istringstream ss;
          std::string buffer;

          while (s) {
            std::getline(s, buffer, _sep);
            if (s.fail()) {
              if (s.eof())
                break;
              else
                throw ConfigException("Invalid default value", Op<T>::_name, _options_string);
            }
            ss.str(buffer);
            ss >> option;
            _options.push_back(option);
          }
        }

        virtual void _init(void) {
          if (!_options_decoded) {
            std::istringstream s(_options_string);
            _decode_options(s);
            _options_decoded = true;
          }
        }

        virtual void _validate(void) { _check(Op<T>::_value); }

      public:
        OpRestricted(OpGroup &group, const std::string &name,
            const std::string &desc, const std::string &options,
            const char sep='|') :
          Op<T>(group, name, desc), _options(),
          _options_string(options), _options_decoded(false), _sep(sep) {
          _init();
        }

        OpRestricted(OpGroup &group, const std::string &name,
            const std::string &desc, const T &default_value,
            const std::string &options, const char sep='|') :
          Op<T>(group, name, desc, default_value),  _options(),
          _options_string(options), _options_decoded(false), _sep(sep) {
          _init();
        }

        virtual ~OpRestricted(void) { }

        virtual void _set(const std::string &value) {
          Op<T>::_set(value);
          _check(Op<T>::_value);
        }

        virtual void help(std::ostream &out, const std::string &prefix,
            const unsigned int depth) const {
          out << prefix << Op<T>::_name << " <";

          for (typename std::vector<T>::const_iterator it = _options.begin();
              it != _options.end(); ++it) {
            if (it != _options.begin())
              out << _sep;
            out << *it;
          }
          out << ">: " << Op<T>::_desc;

          if (Op<T>::_has_default)
            out << " (def=\"" << Op<T>::_default << "\")";
          if (Op<T>::_requires_arg)
            out << " [required]";
          out << std::endl;
        }

        virtual void set_default(void) {
          if (!Op<T>::_has_default)
            throw ConfigException("Option has no default value", Op<T>::_name);
          _check(Op<T>::_default);
          Op<T>::_value = Op<T>::_default;
        }
    };

    template <typename T>
    class OpList : public OpRestricted<T> {
      protected:
        std::string _defaults;

        virtual void _set(const std::string &value) {
          std::istringstream s(value);
          OpRestricted<T>::_decode_options(s);
        }

        virtual void _validate(void) {  }

      public:
        OpList(OpGroup &group, const std::string &name,
            const std::string &desc, const char sep=',') :
          OpRestricted<T>(group, name, desc, "", sep), _defaults() { }

        OpList(OpGroup &group, const std::string &name,
            const std::string &desc, const std::string &defaults,
            const char sep=',') :
          OpRestricted<T>(group, name, desc, T(), "", sep), _defaults(defaults) { }

        virtual ~OpList(void) { }

        typedef typename std::vector<T>::iterator iterator;
        typedef typename std::vector<T>::const_iterator const_iterator;

        iterator begin(void) { return OpRestricted<T>::_options.begin(); }
        iterator end(void) { return OpRestricted<T>::_options.end(); }
        const_iterator begin(void) const { return OpRestricted<T>::_options.begin(); }
        const_iterator end(void) const { return OpRestricted<T>::_options.end(); }

        virtual void help(std::ostream &out, const std::string &prefix,
            const unsigned int depth) const {
          out << prefix << OpRestricted<T>::_name;
          out << " <arg1" << OpRestricted<T>::_sep << "arg2";
          out << OpRestricted<T>::_sep << "... >: " << OpRestricted<T>::_desc;

          if (OpRestricted<T>::_has_default)
            out << " (def=\"" << _defaults << "\")" << std::endl;
          if (OpRestricted<T>::_requires_arg)
            out << " [required]";
        }

        virtual void set_default(void) {
          if (!OpRestricted<T>::_has_default)
            throw ConfigException("Option has no default value", OpRestricted<T>::_name);
          std::istringstream s(_defaults);
          OpRestricted<T>::_decode_options(s);
        }

        virtual void save(std::ostream &out, const std::string &prefix) {
          out << OpRestricted<T>::_name << " = ";
          for (const_iterator it = begin(); it != end(); ++it) {
            if (it != begin())
              out << OpRestricted<T>::_sep;
            out << *it;
          }
          out << '\n';
        }

        virtual void load(std::istream &in) {
          OpRestricted<T>::_decode_options(in);
        }
    };

    class OpPath : public Op<std::string> {
      protected:
        const OpPath *_base;
        mutable std::string _derived;

      public:
        OpPath(OpGroup &group, const std::string &name, const std::string &desc,
            const OpPath *base=0) :
          Op<std::string>(group, name, desc, true), _base(base), _derived() { }

        OpPath(OpGroup &group, const std::string &name, const std::string &desc,
            const std::string &default_value, const OpPath *base=0) :
          Op<std::string>(group, name, desc, default_value, true),
          _base(base), _derived() { }

        virtual ~OpPath(void) { }

        const std::string &operator()(void) const {
          if (_base && _value.size() > 2 && _value[0] == port::PATH_SEP
              && _value[1] == port::PATH_SEP) {
            _derived = (*_base)() + port::PATH_SEP + Op<std::string>::_value.substr(2);
            return _derived;
          }
          return Op<std::string>::_value;
        };
    };

    class OpInput : public Op<std::string> {
      public:
        static const char *const STDIN;

      protected:
        std::istream *_in;
        bool _is_stdin;

        virtual void _validate(void);

      public:
        OpInput(OpGroup &group, const std::string &name,
            const std::string &desc) :
          Op<std::string>(group, name, desc), _in(0),
          _is_stdin(false) { }

        OpInput(OpGroup &group, const std::string &name,
            const std::string &desc, const bool stdin_default) :
          Op<std::string>(group, name, desc, stdin_default ? STDIN : "", true),
          _in(0), _is_stdin(false) { }

        OpInput(OpGroup &group, const std::string &name,
            const std::string &desc, const std::string &default_value) :
          Op<std::string>(group, name, desc, default_value, true), _in(0),
          _is_stdin(false) { }

        virtual ~OpInput(void);

        inline std::istream &file(void) const { return *_in; }
    };

    class OpOutput : public Op<std::string> {
      public:
        static const char *const STDOUT;

      protected:
        std::ostream *_out;
        bool _is_stdout;

        virtual void _validate(void);

      public:
        OpOutput(OpGroup &group, const std::string &name, const std::string &desc) :
          Op<std::string>(group, name, desc, STDOUT, true), _out(0), _is_stdout(false) { }

        virtual ~OpOutput(void);

        inline std::ostream &file(void) const { return *_out; }
    };

    class OpAlias : public OpBase {
      protected:
        OpBase &_aliased;

        virtual void _set(const std::string &value) { _aliased._set(value); };
        virtual void _validate(void) { _aliased._validate(); };
      public:
        OpAlias(OpGroup &group, const std::string &name,
            const std::string &desc, OpBase &aliased) :
          OpBase(group, name, desc, aliased.has_default(),
              aliased.requires_arg()), _aliased(aliased) { }

        virtual ~OpAlias(void) { }

        virtual void help(std::ostream &out, const std::string &prefix,
            const unsigned int depth) const {
          _aliased.help(out, prefix, depth);
        }

        virtual void set(const std::string &value) { _aliased.set(value); _is_set = true; };
        virtual void set_default(void) { _aliased.set_default(); };
        virtual void validate(void) { _aliased.validate(); };

        virtual void save(std::ostream &out, const std::string &prefix) {
          _aliased.save(out, prefix);
        }

        virtual void load(std::istream &in) {
          return _aliased.load(in);
        }
    };
  }
}
