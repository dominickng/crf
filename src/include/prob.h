namespace NLP {

  typedef std::vector<lbfgsfloatval_t> PDF;
  typedef std::vector<PDF> PDFs;
  typedef std::vector<PDFs> PSIs;

  template <typename T>
  inline bool isinf(T value) {
    return std::numeric_limits<T>::has_infinity && value == std::numeric_limits<T>::infinity();
}


}
