namespace NLP {
  class FactorGraph {
    private:
      void _build_variables(size_t max_size);
      void _build_factors(size_t max_size);

      bool converged(double *old_messages, double *new_messages, size_t nmessages, const double threshold);

      bool message_from_variable(PSIs &psis, Variable *from, Factor *to, const double threshold);

      bool message_from_factor(PSIs &psis, Factor *from, Variable *to, const double threshold);

      double _compute_state_marginals(PSIs &psis, PDFs &state_marginals, Variable *variable, size_t index);
      double _compute_trans_marginals(PSIs &psis, PDFs &trans_marginals, Factor *factor, Variable *from, Variable *to);

    public:
      typedef std::vector<Variable *> Variables;
      typedef std::vector<Factor *> Factors;

      Util::Pool *pool;
      TagLimits &limits;
      Variables variables;
      Factors factors;
      Factors randomized_factors;
      MessageMap messages;

      FactorGraph(TagLimits &limits);

      void build(size_t max_size) {
        _build_variables(max_size);
        _build_factors(max_size);
      }

      bool propagate(PSIs &psis, size_t max_size, size_t max_iterations, const double threshold);

      double marginals(PSIs &psis, PDFs &state_marginals, PDFs &trans_marginals);
  };
}
