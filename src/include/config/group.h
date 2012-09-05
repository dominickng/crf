namespace NLP {
  namespace config {
    class OpGroup : public OptionBase {
      // represents a group of options
      protected:
        OpGroup(const std::string &name, const std::string &desc) :
          OptionBase(name, desc), _children() { }

        std::vector<OptionBase *> _children;

        virtual void help(std::ostream &out, const std::string &prefix, const unsigned int depth) const;

      public:
        OpGroup(OpGroup &group, const std::string &name,
            const std::string &desc);

        virtual ~OpGroup(void) { }

        inline void add(OptionBase *const child) {
          _children.push_back(child);
        }

        void help(std::ostream &out) const { help(out, "  --", 0); };
        virtual OptionBase *process(const std::string &orig_key, const std::string &key);
        virtual void set(const std::string &value);
        virtual void validate(void);
    };

    class Config : public OpGroup {
      protected:
        virtual void help(std::ostream &out, const std::string &prefix, const unsigned int depth) const;

      public:
        Config(const std::string &name, const std::string &desc) :
          OpGroup(name, desc) { }

        virtual ~Config(void) { }

        using OpGroup::help;
        OptionBase *process(const std::string &orig_key, const std::string &key);
        bool process(const int argc, const char *const argv[], std::ostream &out=std::cerr);
    };
  }
}
