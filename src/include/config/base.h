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

    /**
     * OptionBase. Abstract base class for a configuration option.
     *
     * Configuration options are specified in object form. They may be set
     * in code or processed from the command line. Options can be namespaced by
     * organising them in logical groups.
     *
     * Options have names and descriptions. They may be hidden by default
     * when --help is called, and may optionally require a following
     * positional argument. OptionBase defines all of these properties.
     */
    class OptionBase {
      protected:
        const std::string _name;
        const std::string _desc;
        const bool _hide_help;
        const bool _requires_arg;
        bool _is_set;

      public:
        OptionBase(const std::string &name, const std::string &desc,
            const bool hide_help, const bool requires_arg);
        virtual ~OptionBase(void) { }

        const std::string &name(void) { return _name; }

        inline bool requires_arg(void) const { return _requires_arg; }
        inline bool is_set(void) const { return _is_set; }
        inline bool hide_help(void) const { return _hide_help; }

        /**
         * help. Pure virtual function to print a help message for this
         * option to an ostream. Optionally, the message may be truncated or
         * not printed (based on the full flag), prefixed, or printed at a
         * certain depth in the option hierarchy
         */
        virtual void help(std::ostream &out, const std::string &prefix,
            const unsigned int depth, const bool full=false) const = 0;

        /**
         * process. Pure virtual function that checks whether this option
         * matches the provided key, and sets the option accordingly.
         *
         * Namespaced options are processed by continually removing each
         * component of the namespace from the key. orig_key always holds the
         * original unaltered key, while key may have components removed from
         * it. A match is found is key is equal to the name of this option
         */
        virtual OptionBase *process(const std::string &orig_key, const std::string &key) = 0;

        /**
         * set. Pure virtual function that processes the value string and
         * sets this option to that value.
         */
        virtual void set(const std::string &value) = 0;

        /**
         * validate. Pure virtual function that checks whether this option
         * has been set to a valid value.
         */
        virtual void validate(void) = 0;

        /**
         * save. Pure virtual function that saves the value of this option to
         * an ostream, optionally prefixed for namespacing
         */
        virtual void save(std::ostream &out, const std::string &prefix) = 0;

        /**
         * load. Pure virtual function that loads the value of this option from
         * an istream. It is expected that the istream is formatted as per the
         * save() function.
         */
        virtual void load(std::istream &in) = 0;
    };
  }
}
