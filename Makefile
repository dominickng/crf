CXX = g++
INCLUDE = -Isrc/include -Iext/lbfgs/include
LDFLAGS = -Lext/lbfgs/lib -llbfgs
CXXFLAGS = -O0 -g $(INCLUDE)

include Makefile.targets
-include Makefile.deps
