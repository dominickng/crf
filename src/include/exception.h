/**
 * exception.h.
 * Defines broad exception classes used in the CRF
 */

/**
 * Exception.
 * general errors with a message
 */
class Exception : public std::exception {
  public:
    const std::string msg;

    Exception(const std::string &msg) : msg(msg) { }
    Exception(const Exception &other) : std::exception(other), msg(other.msg) { }
    virtual ~Exception(void) throw() { }

    virtual const char* what(void) const throw() { return msg.c_str(); }
};


/**
 * IOException.
 * I/O errors with an error message, filename, and line number
 * normally these occur when files are missing or file reading
 * when the text does not match the expected input format
 */
class IOException : public Exception {
  public:
    const std::string uri;
    const int line;

    IOException(const std::string &msg) : Exception(msg), uri(), line(0) { }
    IOException(const std::string &msg, const std::string &uri, int line=0) : Exception(msg), uri(uri), line(line) { }
    IOException(const IOException &other) : Exception(other), uri(other.uri), line(other.line) { }
    virtual ~IOException(void) throw() { }
};

/**
 * FormatException.
 * an exception used in the IO format system
 */
class FormatException : public Exception {
  public:
    const char c;
    FormatException(const std::string &msg, const char c) : Exception(msg), c(c) { }
    FormatException(const FormatException &other) : Exception(other), c(other.c) { }
    virtual ~FormatException(void) throw() { }
};

/**
 * ValueException.
 * a general exception indicating some sort of malformed value
 */
class ValueException : public Exception {
  public:
    const std::string &value;

    ValueException(const std::string &msg, const std::string &value) : Exception(msg), value(value) { }
    ValueException(const ValueException &other) : Exception(other), value(other.value) { }
    virtual ~ValueException(void) throw() { }
};
