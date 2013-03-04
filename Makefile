CXX = clang++
INCLUDE = -Isrc/include -Iext/lbfgs-float/include
LDFLAGS = -Lext/lbfgs-float/lib -llbfgs
CXXFLAGS = -Wall -DNDEBUG -DFASTEXP -O3 $(INCLUDE)

include Makefile.targets
-include Makefile.deps
