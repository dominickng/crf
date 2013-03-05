namespace NLP {
  namespace CRF {

    class Morph {
      protected:

        void analyse(const std::string &word) {
          fupper = isupper(word[0]);
          fhyphen = word[0] == '-';

          for (size_t i = 0; i < size; ++i) {
            char c = word[i];
            if (isupper(c)) {
              ++nupper;
              switch (c) {
                case 'I':
                case 'V':
                case 'X':
                case 'L':
                case 'C':
                case 'D':
                case 'M': ++nuromans;
              }
            }
            else if (islower(c)) {
              ++nlower;
              switch (c) {
                case 'i':
                case 'v':
                case 'x':
                case 'l':
                case 'c':
                case 'd':
                case 'm': ++nlromans;
              }
            }
            else if (isdigit(c))
              ++ndigits;
            else if (ispunct(c)) {
              ++npunct;
              switch(c) {
                case '-': ++nhyphens; break;
                case '.': ++nperiods; break;
                case ',': ++ncommas; break;
              }
            }
            else
              ++nother;
          }
          nalpha = nlower + nupper;
        }

      public:
        uint64_t size;
        uint64_t nupper;
        uint64_t nlower;
        uint64_t nalpha;

        uint64_t ndigits;
        uint64_t npunct;
        uint64_t nother;

        uint64_t nhyphens;
        uint64_t ncommas;
        uint64_t nperiods;

        uint64_t nuromans;
        uint64_t nlromans;

        bool fupper;
        bool fhyphen;

        Morph(const std::string &word)
          : size(word.size()), nupper(0), nlower(0), nalpha(0), ndigits(0),
            npunct(0), nother(0), nhyphens(0), ncommas(0), nperiods(0),
            nuromans(0), nlromans(0) { analyse(word); }

        bool all_digits(void) const { return ndigits == size; }
        bool all_punct(void) const { return npunct == size; }
        bool is_alpha(void) const { return nalpha == size; }
        bool is_alnum(void) const { return (nalpha + ndigits) == size; }
        bool is_number(void) const { return (fhyphen + ndigits + nperiods + ncommas) == size; }
        bool is_initial(void) const { return fupper + nperiods == 2; };
        bool is_acronym(void) const { return fupper && (nperiods + nupper) == size; }

        bool uppercase(void) const { return nupper == size; }
        bool lowercase(void) const { return nlower == size; }
        bool titlecase(void) const { return fupper && nlower == size - 1; }
        bool mixedcase(void) const { return (nupper + nlower) == size; }

        bool get_feature(const Type &type) const {
          if (type.name == Types::has_digit.name)
            return ndigits != 0;
          else if (type.name == Types::has_hyphen.name)
            return nhyphens != 0;
          else if (type.name == Types::has_period.name)
            return nperiods != 0;
          else if (type.name == Types::has_punct.name)
            return npunct != 0;
          else if (type.name == Types::has_uppercase.name)
            return nupper != 0;
          else if (type.name == Types::digits.name)
            return all_digits();
          else if (type.name == Types::number.name)
            return is_number();
          else if (type.name ==Types::alnum.name)
            return is_alnum();
          else if (type.name == Types::alpha.name)
            return is_alpha();
          else if (type.name == Types::roman.name)
            return (nuromans + nlromans) == size;
          else if (type.name == Types::initial.name)
            return is_initial();
          else if (type.name == Types::acronym.name)
            return is_acronym();
          else if (type.name == Types::uppercase.name)
            return uppercase();
          else if (type.name == Types::lowercase.name)
            return lowercase();
          else if (type.name == Types::titlecase.name)
            return titlecase();
          else if (type.name == Types::mixedcase.name)
            return mixedcase();
          return false;
        }

        void add_feature(const Type &type, Attributes &attributes, TagPair &tp, const bool add_state, const bool add_trans) const {
          if (get_feature(type))
            attributes(type.name, "true", tp, add_state, add_trans);
        }

        void add_feature(const Type &type, Attributes &attributes, Context &c, const bool add_state, const bool add_trans) const {
          if (get_feature(type))
            attributes(type.name, "true", c);
        }
    };
  }
}
