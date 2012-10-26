namespace NLP {

  typedef std::vector<double> PDF;
  typedef std::vector<PDF> PDFs;
  typedef std::vector<PDFs> PSIs;

  void vector_scale(PDF &vec, const double scale);
  double vector_sum_log(PDF &vec);

}
