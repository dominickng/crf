namespace config = Util::config;

namespace NLP {

  template <typename TAGGER>
  int run_train(int argc, char **argv) {
    std::string preface;
    create_preface(argc, argv, preface);

    config::Main cfg(TAGGER::name, TAGGER::desc);
    typename TAGGER::Config tagger_cfg;
    typename TAGGER::FeatureTypes types_cfg;

    config::OpAlias model(cfg, "model", "location to store the model", tagger_cfg.model);
    config::OpInput input(cfg, "input", "training data location");

    tagger_cfg.add(&types_cfg);
    cfg.add(&tagger_cfg);
    if(cfg.process(argc, argv)) {
      Util::port::make_directory(tagger_cfg.model());
      TAGGER tagger(tagger_cfg, types_cfg, preface);
      ReaderFactory reader(TAGGER::reader, input(), input.file(), tagger_cfg.ifmt());
      tagger.train(reader);
    }

    return 0;
  }

  template <typename TAGGER>
  int run_tag(int argc, char **argv) {
    std::string preface;
    create_preface(argc, argv, preface);

    config::Main cfg(TAGGER::name, TAGGER::desc);
    typename TAGGER::Config tagger_cfg;
    typename TAGGER::FeatureTypes types_cfg;

    config::OpAlias model(cfg, "model", "location of the model", tagger_cfg.model);
    config::OpInput input(cfg, "input", "input file location");
    config::OpOutput output(cfg, "output", "output file location");

    tagger_cfg.add(&types_cfg);
    cfg.add(&tagger_cfg);
    if(cfg.process(argc, argv)) {
      TAGGER tagger(tagger_cfg, types_cfg, preface);
      ReaderFactory reader("conll", input(), input.file(), "");
      WriterFactory writer("format", output(), output.file(), tagger_cfg.ofmt());
      tagger.tag(reader, writer);
    }

    return 0;
  }
}
