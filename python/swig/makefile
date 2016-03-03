all: ternary_tree.py array_of_trees.py

ternary_tree.py: ternary_tree.i 
	swig3.0 -python -c++ ternary_tree.i
	g++ -c ternary_tree_wrap.cxx -std=c++14  -I /usr/include/python3.5 -fPIC
	g++ -shared ../../obj/ternary_tree.o ../../obj/string_tools.o ternary_tree_wrap.o -o _ternary_tree.so

array_of_trees.py: array_of_trees.i ../../src/array_of_trees.hpp
	swig3.0 -python -c++ array_of_trees.i
	g++ -c array_of_trees_wrap.cxx -std=c++14  -I /usr/include/python3.5m -fPIC
	g++ -shared ../../obj/array_of_trees.o array_of_trees_wrap.o -lboost_filesystem -o _array_of_trees.so

clean:
	rm *.o  2>/dev/null || true 
	rm *.so 2>/dev/null || true
	rm ternary_tree.py 2>/dev/null || true
	rm ternary_tree_wrap.cxx 2>/dev/null || true
	rm array_of_trees.py 2>/dev/null || true
	rm array_of_trees_wrap.cxx 2>/dev/null || true
	rm -rf __pycache__ || true

test:
	python3 test.py