CXX = clang++
INCLUDE = -Isrc/include -Iext/lbfgs/include
LDFLAGS = -Lext/lbfgs/lib -llbfgs
CXXFLAGS =  -DNDEBUG -O3 $(INCLUDE)

mkdir -p bin

include Makefile.targets
-include Makefile.deps
