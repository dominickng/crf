# CRF (in progress)

This is a partial implementation of a conditional random field (CRF) sequence tagger.
Warning: development is ongoing, and the CRF doesn't work properly yet.

Currently, the basic CRF implementation and part-of-speech and named-entity tagging layer are
implemented. This code should compile with any reasonably modern version of g++ or clang++, and
will run (but not necessarily produce good output!).

The code has an external dependency on the excellent [libLBFGS library](http://www.chokkan.org/software/liblbfgs/)
by Naoaki Okazaki.

## Compiling instructions

* Clone the repository or download the code
* `mkdir ext`
* Build libLBFGS and install it in the `ext` directory, so it looks like this:
  `ext/lbfgs/{include,lib,share}`
* Compile with `make`. Binaries will be placed in the `bin` directory
* You may need to set your `DYLD_LIBRARY_PATH` or `LD_LIBRARY_PATH`: `export DYLD_LIBRARY_PATH=/path/to/ext/lbfgs/lib`

## POS instructions

* `bin/train_pos` will train a model for POS tagging.
* `bin/pos` will take a model produced by `train_pos` and use it to tag sentences
* The software can read formatted input described by command line options. Check the `--help` flag
for more info

## NER instructions

* `bin/train_ner` will train a model for NER tagging.
* `bin/ner` will take a model produced by `train_ner` and use it to tag sentences
* Run `bin/ner --help` for a description of program options. The software
  will currently only read the CoNLL 2003 NER shared task formatted input.

## Licensing

This code is licensed for academic (non-commerical) use. Contact Dominick Ng for licensing terms
if you wish to use any or all of this code for any non-academic purpose.
