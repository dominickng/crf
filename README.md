# CRF (in progress)

This is a partial implementation of a conditional random field (CRF) sequence tagger.
Warning: development is ongoing, and the CRF doesn't work properly yet.

Currently, the basic CRF implementation and a named-entity tagging layer are
implemented. This code should compile with any reasonably modern version of
g++ or clang++.

The code has an external dependency on the excellent [libLBFGS library](http://www.chokkan.org/software/liblbfgs/)
by Naoaki Okazaki.

## Compiling instructions

* Unpack the code
* Build libLBFGS and install it in the `ext` directory, like this:
  ```ext/
    lbfgs/
      include/
      lib/
      share/
  ```
* Compile with `make`. Binaries will be placed in the `bin` directory
* Run `bin/ner --help` for a description of program options. The software
  will currently only read the CoNLL 2003 NER shared task formatted input.

## Licensing

This code is licensed for academic (non-commerical) use. Contact Dominick Ng for licensing terms
if you wish to use any or all of this code for any non-academic purpose.
