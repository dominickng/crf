namespace Util {
  namespace config {
    class OpBase : public OptionBase {
      // represent ops that hold some form an argument
      // must be part of some OpGroup
      protected:
        const bool _has_default;

        virtual void _set(const std::string &value) = 0;
        virtual void _validate(void) = 0;

        void _print(std::ostream &out, const std::string &str) {
          for (size_t i = 0; i < str.size(); ++i)
            if (str[i] == '\n')
              out << '\\' << 'n';
            else
              out << str[i];
        }

      public:
        friend class OpAlias;
        OpBase(OpGroup &group, const std::string &name,
            const std::string &desc, const bool has_default,
            const bool hide_help, const bool requires_arg=true);

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

        T escape(const T &val) const {
          return val;
        }

      public:
        const T _default;
        T _value;

        Op(OpGroup &group, const std::string &name, const std::string &desc,
            const bool hide_help, const bool requires_arg=true) :
          OpBase(group, name, desc, false, hide_help, requires_arg),
            _default(), _value() { }

        Op(OpGroup &group, const std::string &name, const std::string &desc,
            const T &default_value, const bool hide_help,
            const bool requires_arg=true) :
          OpBase(group, name, desc, true, hide_help, requires_arg),
            _default(default_value), _value() { }

        virtual ~Op(void) { }

        virtual void help(std::ostream &out, const std::string &prefix,
            const unsigned int depth, const bool full=false) const {
          out << prefix << _name << " <arg>: " << _desc;

          if (_has_default)
            out << " (def = " << escape(_default) << ")";
          out << std::endl;
        }

        virtual void set_default(void) {
          if (!OpBase::_has_default)
            throw ConfigException("Option has no default value", Op<T>::_name);
          _value = _default;
        }

        /**
         * operator().
         * Sets the value of this option to v.
         */
        inline void operator()(const T &v) {
          _value = v;
        }

        /**
         * operator().
         * Returns the current value of this option.
         */
        inline const T &operator()(void) const {
          return _value;
        }

        /**
         * save.
         * Saves the name and value of this option to the ostream.
         */
        virtual void save(std::ostream &out, const std::string &prefix) {
          out << _name << " = " << _value << '\n';
        }

        /**
         * load.
         * Reads in the value of this option from the istream.
         */
        virtual void load(std::istream &in) {
          in >> _value;
        }

    };

    /**
     * _set.
     * Extracts the value of this option from the provided string.
     *
     * std::boolalpha allows conversion of the strings true and false to their
     * boolean versions.
     */
    template <> inline void Op<bool>::_set(const std::string &value) {
      std::istringstream s(value);
      if (!(s >> std::boolalpha >> _value >> std::noboolalpha))
        throw ConfigException("Invalid value", _name, value);
    }

    /**
     * escape.
     * Specialised version for Op<string> that escapes special chars
     */
    template <> inline std::string Op<std::string>::escape(const std::string &val) const {
      std::string out;
      for (size_t i = 0; i < val.size(); ++i) {
        switch(val[i]) {
          case '\t': out += "\\t"; break;
          case '\n': out += "\\n"; break;
          default: out += val[i];
        }
      }
      return out;
    }

    /**
     * _set.
     * Specialised version for string based options that skips creating a
     * new stringstream, and just sets the value.
     */
    template <> inline void Op<std::string>::_set(const std::string &value) {
      _value = value;
    }

    template <> inline void Op<bool>::help(std::ostream &out,
        const std::string &prefix, const unsigned int depth,
        const bool full) const {
          out << prefix << _name << " <arg>: " << _desc;

          if (_has_default)
            out << " (def = " << ((_default) ? "true" : "false") << ")";
          out << std::endl;
    }

    class OpFlag : public Op<bool> {
      protected:
        virtual void _set(const std::string &) { _value = !_default; }

        virtual void help(std::ostream &out, const std::string &prefix,
            const unsigned int depth, const bool full=false) const {
          out << prefix << _name << ": " << _desc;

          if (_has_default)
            out << " (def = " << (_default ? "true" : "false") << ")";
          out << std::endl;
        }

      public:
        OpFlag(OpGroup &group, const std::string &name, const std::string &desc,
            const bool hide_help, const bool default_value=false) :
          Op<bool>(group, name, desc, default_value, hide_help, false) { }

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

        /**
         * _decode_options.
         * Given an istream, extract the available options for this op. The
         * options are stored in a list, and any attempt to set this option
         * is checked against the list to make sure that an acceptable value
         * is provided.
         */
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
            ss.clear();
            ss.str(buffer);
            ss >> option;
            _options.push_back(option);
            buffer.clear();
          }
        }

        virtual void _init(void) {
          if (!_options_decoded) {
            std::istringstream s(_options_string);
            _decode_options(s);
            _options_decoded = true;
          }
        }

        virtual void _validate(void) {
          _check(Op<T>::_value);
        }

      public:
        OpRestricted(OpGroup &group, const std::string &name,
            const std::string &desc, const std::string &options,
            const bool hide_help, const char sep='|') :
          Op<T>(group, name, desc, hide_help), _options(),
          _options_string(options), _options_decoded(false), _sep(sep) {
          _init();
        }

        OpRestricted(OpGroup &group, const std::string &name,
            const std::string &desc, const T &default_value,
            const std::string &options, const bool hide_help,
            const char sep='|') :
          Op<T>(group, name, desc, default_value, hide_help),  _options(),
          _options_string(options), _options_decoded(false), _sep(sep) {
          _init();
        }

        virtual ~OpRestricted(void) { }

        virtual void _set(const std::string &value) {
          Op<T>::_set(value);
          _check(Op<T>::_value);
        }

        virtual void help(std::ostream &out, const std::string &prefix,
            const unsigned int depth, const bool full=false) const {
          out << prefix << Op<T>::_name << " <";

          for (typename std::vector<T>::const_iterator it = _options.begin();
              it != _options.end(); ++it) {
            if (it != _options.begin())
              out << _sep;
            out << *it;
          }
          out << ">: " << Op<T>::_desc;

          if (Op<T>::_has_default)
            out << " (def = " << Op<T>::_default << ")";
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
      // allows for a separator delimited list of options, and provides an
      // iterator like interface to those options
      protected:
        std::string _defaults;

        virtual void _set(const std::string &value) {
          std::istringstream s(value);
          OpRestricted<T>::_decode_options(s);
        }

        virtual void _validate(void) {  }

      public:
        OpList(OpGroup &group, const std::string &name,
            const std::string &desc, const bool hide_help, const char sep=',') :
          OpRestricted<T>(group, name, desc, "", hide_help, sep),
            _defaults() { }

        OpList(OpGroup &group, const std::string &name,
            const std::string &desc, const std::string &defaults,
            const bool hide_help, const char sep=',') :
          OpRestricted<T>(group, name, desc, T(), "", hide_help, sep),
            _defaults(defaults) { }

        virtual ~OpList(void) { }

        typedef typename std::vector<T>::iterator iterator;
        typedef typename std::vector<T>::const_iterator const_iterator;

        iterator begin(void) { return OpRestricted<T>::_options.begin(); }
        iterator end(void) { return OpRestricted<T>::_options.end(); }
        const_iterator begin(void) const { return OpRestricted<T>::_options.begin(); }
        const_iterator end(void) const { return OpRestricted<T>::_options.end(); }

        virtual void help(std::ostream &out, const std::string &prefix,
            const unsigned int depth, const bool full=false) const {
          out << prefix << OpRestricted<T>::_name;
          out << " <arg1" << OpRestricted<T>::_sep << "arg2";
          out << OpRestricted<T>::_sep << "... >: " << OpRestricted<T>::_desc;

          if (OpRestricted<T>::_has_default)
            out << " (def = " << Op<std::string>::escape(_defaults) << ")" << std::endl;
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
      // path to a directory or a file
      protected:
        const OpPath *_base;
        mutable std::string _derived;

      public:
        OpPath(OpGroup &group, const std::string &name, const std::string &desc,
            const bool hide_help, const OpPath *base=0) :
          Op<std::string>(group, name, desc, hide_help, true),
            _base(base), _derived() { }

        OpPath(OpGroup &group, const std::string &name, const std::string &desc,
            const std::string &default_value, const bool hide_help,
            const OpPath *base=0) :
          Op<std::string>(group, name, desc, default_value, hide_help, true),
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
            const std::string &desc, const bool hide_help) :
          Op<std::string>(group, name, desc, hide_help), _in(0),
            _is_stdin(false) { }

        OpInput(OpGroup &group, const std::string &name,
            const std::string &desc, const bool hide_help,
            const bool stdin_default) :
          Op<std::string>(group, name, desc, stdin_default ? STDIN : "",
            hide_help, true), _in(0), _is_stdin(false) { }

        OpInput(OpGroup &group, const std::string &name,
            const std::string &desc, const std::string &default_value,
            const bool hide_help) :
          Op<std::string>(group, name, desc, default_value, hide_help, true),
            _in(0), _is_stdin(false) { }

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
        OpOutput(OpGroup &group, const std::string &name,
            const std::string &desc, const bool hide_help) :
          Op<std::string>(group, name, desc, hide_help, true),
            _out(0), _is_stdout(false) { }

        OpOutput(OpGroup &group, const std::string &name,
            const std::string &desc, const bool hide_help,
            const bool stdout_default) :
          Op<std::string>(group, name, desc, stdout_default ? STDOUT : "",
            hide_help, true), _out(0), _is_stdout(false) { }

        OpOutput(OpGroup &group, const std::string &name,
            const std::string &desc, const std::string &default_value,
            const bool hide_help) :
          Op<std::string>(group, name, desc, default_value, hide_help, true),
            _out(0), _is_stdout(false) { }

        virtual ~OpOutput(void);

        inline std::ostream &file(void) const { return *_out; }
    };

    class OpError : public Op<std::string> {
      public:
        static const char *const STDERR;

      protected:
        std::ostream *_out;
        bool _is_stderr;

        virtual void _validate(void);

      public:
        OpError(OpGroup &group, const std::string &name,
            const std::string &desc, const bool hide_help) :
          Op<std::string>(group, name, desc, hide_help, true),
            _out(0), _is_stderr(false) { }

        OpError(OpGroup &group, const std::string &name,
            const std::string &desc, const bool hide_help,
            const bool stderr_default) :
          Op<std::string>(group, name, desc, stderr_default ? STDERR : "",
            hide_help, true), _out(0), _is_stderr(false) { }

        OpError(OpGroup &group, const std::string &name,
            const std::string &desc, const std::string &default_value,
            const bool hide_help) :
          Op<std::string>(group, name, desc, default_value, hide_help, true),
            _out(0), _is_stderr(false) { }

        virtual ~OpError(void);

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
          OpBase(group, name, desc, aliased.has_default(), aliased.hide_help(),
              aliased.requires_arg()), _aliased(aliased) { }

        OpAlias(OpGroup &group, const std::string &name,
            const std::string &desc, const bool hide_help, OpBase &aliased) :
          OpBase(group, name, desc, aliased.has_default(), hide_help,
              aliased.requires_arg()), _aliased(aliased) { }

        virtual ~OpAlias(void) { }

        virtual void help(std::ostream &out, const std::string &prefix,
            const unsigned int depth, const bool full=false) const {
          _aliased.help(out, prefix, depth, full);
        }

        virtual void set(const std::string &value) {
          _aliased.set(value);
          _is_set = true;
        };

        virtual void set_default(void) {
          _aliased.set_default();
        };

        virtual void validate(void) {
          _aliased.validate();
        };

        virtual void save(std::ostream &out, const std::string &prefix) {
          _aliased.save(out, prefix);
        }

        virtual void load(std::istream &in) {
          return _aliased.load(in);
        }
    };
  }
}
