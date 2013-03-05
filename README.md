##WARNING

Floats do not contain enough precision for the CRF to train properly;
experimentation has shown that training will terminate too early (possibly
with status code -1001 if using L-BFGS; this indicates that line searching
failed). The regularization and log-likelihood calculation requires the summation
of many small floating point values, and errors in rounding inevitably creep in.

# CRF (in progress)

This is an implementation of a conditional random field (CRF) sequence tagger
in C++. Development is ongoing, and only a limited number of possible features
for training are currently implemented. However, the CRF will train a model and
can use it to tag new sentences.

Currently, the underlying CRF training code as well as part-of-speech,
chunking, and named-entity tagging layers are implemented.

If you're interested in how CRFs work, there is a brief introduction in
`src/include/crf/tagger.h`. There are also good tutorials on the web, such as
[Sutton and McCallum (2012)](http://homepages.inf.ed.ac.uk/csutton/publications/crftut-fnt.pdf)

The code has an external dependency on the excellent [libLBFGS
library](http://www.chokkan.org/software/liblbfgs/) by Naoaki Okazaki.
Otherwise, it only uses the standard C++ library, and does not require
C++11. It should compile with any reasonably modern version of g++ or clang++.

## Compiling instructions

* Clone the repository or download the code
* `mkdir ext`
* Build libLBFGS and install it in the `ext` directory, so it looks like this:
  `ext/lbfgs/{include,lib,share}`
* Compile with `make`. Binaries will be placed in the `bin` directory
* You may need to set your `DYLD_LIBRARY_PATH` or `LD_LIBRARY_PATH` for the code to run: `export DYLD_LIBRARY_PATH=/path/to/ext/lbfgs/lib`

## Input and output

* Formats are controlled by the "--ifmt" and "--ofmt" command line options.
* The Chunker and NER taggers currently only read the CoNLL 2000 and CoNLL 2003 data
  formats. The POS tagger can read flexible formats
* All the taggers can produce output in flexible formats
* There is a mini printf-style language for specifying input and output formats.
  The format specifies how each word in the sentence should be formatted along with
  its accompanying tags. Each word in the sentence is printed in the same way.
* Formats look like the following (`+` means "at least one"): `"<sent_pre>(<format><sep>)+<word_sep><sent_pos>"`
    * `<sent_pre>` is a string printed before each sentence
    * `<format>` is one of the format strings
    * `<sep>` is a one character (only) separator between format items (escapes like `\n` are allowed)
    * `<word_sep>` is a one character (only) separator between each word block (escapes like `\n` are allowed)
    * `<sent_pos>` is a string printed at the end of each sentence
* Available format strings are:
    * `%w` for the word
    * `%p` for the part of speech tag
    * `%c` for the chunk tag
    * `%e` for the entity tag
* Note that you should only print out format strings that are actually present
  in the input or produced by the tagger.
* For example, to produce output from the chunker in the CoNLL 2000 evaluation format:
  `--ofmt "%w %p %c %e\n\n\n"`

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
