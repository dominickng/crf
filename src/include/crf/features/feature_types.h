namespace NLP {
  namespace CRF {
      namespace config = Util::config;

      class OpType : public config::OpFlag {
        protected:
          FeatureGen *_gen;
          FeatureDict *_dict;

        public:
          OpType(config::OpGroup &group, const std::string &name,
              const std::string &desc, FeatureGen *gen,
              const bool default_value=true)
            : config::OpFlag(group, name, desc, default_value), _gen(gen),
          _dict(0) { }

          ~OpType(void) {
            delete _gen;
          }

          void reg(FeatureDict *dict) { _dict = dict; }

          bool has_type(const Type &type) { return type.equals(_gen->type); }

          bool has_type(const std::string &type) { return type == _gen->type.id; }

          Attribute &load(const std::string &type, std::istream &in) {
            return _dict->load(type, in);
          }

          template <typename TPC>
          void generate(Attributes &attributes, Sentence &sent, TPC &tpc, int index) { (*_gen)(attributes, sent, tpc, index); }
      };

  }
}
