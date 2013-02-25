namespace NLP {
  class Log {
    protected:
      const std::string &uri;
      std::ostream *out;
      std::ofstream *file;

    public:
      Log(const std::string &uri, std::ostream &out);
      ~Log(void);

      template <typename T>
      Log &operator<<(const T &msg) {
        if (file)
          *file << msg;
        *out << msg;
        return *this;
      }

      // for std::endl and its ilk
      Log &operator<<(std::ostream& (*pf)(std::ostream&)) {
        if (file)
          pf(*file);
        pf(*out);
        return *this;
      }
  };
}
