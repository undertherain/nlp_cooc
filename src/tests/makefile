.Phony: clean run

LIBS = -lboost_filesystem -lboost_system

CXXFLAGS = -Wall -std=c++14 -O3 -ggdb3 -g3 -fdiagnostics-color=auto -fopenmp

all: test_tree

test_reader: test_reader.cpp
	g++  $^ $(CXXFLAGS) -o $@ $(LIBS)

test_tree.o: test_tree.cpp
	g++  $^ $(CXXFLAGS) -c -o $@ $(LIBS)

test_tree: test_tree.o ../../obj/ternary_tree.o
	g++  $^ $(CXXFLAGS) -o $@ $(LIBS)

run: test_tree
	./test_tree
	dot -Tpng original.gv -o original.png
	dot -Tpng reduced.gv  -o reduced.png
