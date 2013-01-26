CXX = clang++
INCLUDE = -Isrc/include -Iext/lbfgs/include
LDFLAGS = -Lext/lbfgs/lib -llbfgs
CXXFLAGS =  -DNDEBUG -O0 -g $(INCLUDE)

include Makefile.targets
-include Makefile.deps
