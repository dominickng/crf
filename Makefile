CXX = g++
INCLUDE = -Isrc/include -Iext/lbfgs/include
LDFLAGS = -Lext/lbfgs/lib -llbfgs
CXXFLAGS = -Wall -DNDEBUG -O3 $(INCLUDE)

include Makefile.targets
-include Makefile.deps
