namespace NLP {
  typedef std::string Raw;
  typedef std::vector<Raw> Raws;

  struct ScoredRaw {
    Raw raw;
    double score;

    ScoredRaw(Raw &raw) : raw(raw), score(0.0) { }
    ScoredRaw(Raw &raw, double score) : raw(raw), score(score) { }
  };

  //typedef std::vector<ScoredRaw> ScoredRaws;
  //typedef std::vector<ScoredRaws> MultiRaw;
  //typedef std::vector<MultiRaw> MultiRaws;

  //typedef Util::offset_vector<Word, 2, 2> OffsetWords;
  //typedef Util::offset_vector<Tag, 2, 2> OffsetTags;
}
