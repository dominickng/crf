namespace Util {
  namespace config {
    class ConfigException : public Exception {
      public:
        std::string msg;
        std::string name;
        std::string value;

        explicit ConfigException(const std::string &msg) :
          Exception(msg), msg(msg), name(), value() { }
        ConfigException(const std::string &msg, const std::string &name) :
          Exception(msg), msg(msg), name(name), value() { }
        ConfigException(const std::string &msg, const std::string &name,
            const std::string &value) : Exception(msg), msg(msg), name(name), value(value) { }
        ConfigException(const ConfigException &other) : Exception(other.msg),
            msg(other.msg), name(other.name), value(other.value) { }
        virtual ~ConfigException(void) throw() { }

        virtual const char *what(void) const throw();
    };

    class OptionBase {
      protected:
        const std::string _name;
        const std::string _desc;
        const bool _requires_arg;
        bool _is_set;

      public:
        OptionBase(const std::string &name, const std::string &desc,
            const bool requires_arg=false);
        virtual ~OptionBase(void) { }

        const std::string &name(void) { return _name; }

        inline bool requires_arg(void) const { return _requires_arg; }
        inline bool is_set(void) const { return _is_set; }

        virtual void help(std::ostream &out, const std::string &prefix, const unsigned int depth) const = 0;
        virtual OptionBase *process(const std::string &orig_key, const std::string &key) = 0;
        virtual void set(const std::string &value) = 0;
        virtual void validate(void) = 0;

        virtual void save(std::ostream &out, const std::string &prefix) = 0;
        virtual void load(std::istream &in) = 0;
    };
  }
}
