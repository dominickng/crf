namespace NLP {

  inline void vector_scale(PDF &vec, const lbfgsfloatval_t scale, const size_t size) {
    for (size_t i = 0; i < size; ++i)
      vec[i] *= scale;
  }

  inline void vector_scale(lbfgsfloatval_t *vec, const lbfgsfloatval_t scale, const size_t size) {
    for (size_t i = 0; i < size; ++i)
      vec[i] *= scale;
  }

  inline lbfgsfloatval_t vector_sum_log(PDF &vec, const size_t size) {
    lbfgsfloatval_t sum = 0.0;
    for (size_t i = 0; i < size; ++i)
      sum += log(vec[i]);
    return sum;
  }

  inline lbfgsfloatval_t vector_sum(PDF &vec, const size_t size) {
    lbfgsfloatval_t sum = 0.0;
    for (size_t i = 0; i < size; ++i)
      sum += vec[i];
    return sum;
  }

  inline void vector_print(PDF &vec, const size_t size) {
    for (size_t i = 0; i < size; ++i)
      printf("%f ", vec[i]);
    printf("\n");
  }

  inline void vector_print(const lbfgsfloatval_t *vec, const size_t size) {
    for (size_t i = 0; i < size; ++i)
      printf("%f ", vec[i]);
    printf("\n");
  }

}
