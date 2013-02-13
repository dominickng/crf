namespace NLP {

  typedef std::vector<double> PDF;
  typedef std::vector<PDF> PDFs;
  typedef std::vector<PDFs> PSIs;

  void vector_scale(PDF &vec, const double scale, const size_t size);
  double vector_sum_log(PDF &vec, const size_t size);
  double vector_sum(PDF &vec, const size_t size);
  void vector_print(PDF &vec, const size_t size);
  void vector_print(const double *vec, const size_t size);

}
