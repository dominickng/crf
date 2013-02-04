namespace Util {
  namespace config {
    class Info : public Config {
      protected:
        const OpPath &base;
      public:
        Info(const std::string &name, const std::string &desc, const OpPath &base) :
          Config(name, desc), base(base) { }

        virtual ~Info(void) { }

        virtual void save(const std::string &preface);
        virtual void save(std::ostream &out, const std::string &prefix);
        virtual bool read_config(void);
    };
  }
}
