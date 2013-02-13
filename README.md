# CRF (in progress)

This is an implementation of a conditional random field (CRF) sequence tagger
in C++. Development is ongoing, and only a limited number of possible features
for training are currently implemented. However, the CRF will train a model and
can use it to tag new sentences.

Currently, the underlying CRF training code as well as part-of-speech,
chunking, and named-entity tagging layers are implemented. This code should
compile with any reasonably modern version of g++ or clang++.

The code has an external dependency on the excellent [libLBFGS
library](http://www.chokkan.org/software/liblbfgs/) by Naoaki Okazaki.
Otherwise, it only uses the standard C++ library, and does not require
C++11.

## Compiling instructions

* Clone the repository or download the code
* `mkdir ext`
* Build libLBFGS and install it in the `ext` directory, so it looks like this:
  `ext/lbfgs/{include,lib,share}`
* Compile with `make`. Binaries will be placed in the `bin` directory
* You may need to set your `DYLD_LIBRARY_PATH` or `LD_LIBRARY_PATH` for the code to run: `export DYLD_LIBRARY_PATH=/path/to/ext/lbfgs/lib`

## POS instructions

* `bin/train_pos` will train a model for POS tagging.
* `bin/pos` will take a model produced by `train_pos` and use it to tag sentences
* The software reads pipe-formatted input by default as described by command line
  options. Check the `--help` flag for more info. Custom formats can also be used.

## Chunking instructions

* `bin/train_chunk` will train a model for chunking.
* `bin/chunk` will take a model produced by `train_chunk` and use it to tag sentences
* The software will currently only read data in the CoNLL 2000 chunking shared task
  format. This data is available at [http://www.cnts.ua.ac.be/conll2000/chunking/](http://www.cnts.ua.ac.be/conll2000/chunking/).

## NER instructions

* `bin/train_ner` will train a model for NER tagging.
* `bin/ner` will take a model produced by `train_ner` and use it to tag sentences
* Run `bin/ner --help` for a description of program options. The software
  will currently only read the CoNLL 2003 NER shared task formatted input 
  (see [http://www.cnts.ua.ac.be/conll2003/ner/](http://www.cnts.ua.ac.be/conll2003/ner/)
  for more information).

## Licensing

This code is licensed for academic (non-commerical) use. Contact me for licensing terms
if you wish to use any or all of this code for any non-academic purpose.
