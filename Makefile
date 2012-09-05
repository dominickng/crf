CXX = clang++
INCLUDE = -Isrc/include
CXXFLAGS = -g -O0 $(INCLUDE)

include Makefile.targets
-include Makefile.deps
