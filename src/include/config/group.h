namespace Util {
  namespace config {
    class OpGroup : public OptionBase {
      // represents a group of options
      protected:
        OpGroup(const std::string &name, const std::string &desc,
            const bool hide_help) :
          OptionBase(name, desc, hide_help, false), _children() { }

        typedef std::vector<OptionBase *> Children;
        Children _children;

        virtual void help(std::ostream &out, const std::string &prefix,
            const unsigned int depth, const bool full=false) const;

      public:
        OpGroup(OpGroup &group, const std::string &name,
            const std::string &desc, const bool hide_help);

        virtual ~OpGroup(void) { }

        inline void add(OptionBase *const child) {
          _children.push_back(child);
        }

        virtual OptionBase *process(const std::string &orig_key, const std::string &key);
        virtual void set(const std::string &value);
        virtual void validate(void);

        virtual void save(std::string &path, const std::string &preface);
        virtual void save(std::ostream &out, const std::string &prefix);
        virtual void load(std::istream &in);
        virtual bool read_config(std::string &filename);
    };

    class Config : public OpGroup {
      protected:
        virtual void help(std::ostream &out, const std::string &prefix,
            const unsigned int depth, const bool full=false) const;

      public:
        Config(const std::string &name, const std::string &desc) :
          OpGroup(name, desc, false) { }

        virtual ~Config(void) { }

        void help(std::ostream &out, const bool full=false) const {
          help(out, "  --", 0, full);
        };

        OptionBase *process(const std::string &orig_key, const std::string &key);
        bool process(const int argc, const char *const argv[],
            std::ostream &out=std::cerr);
    };

  }
}
