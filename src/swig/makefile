all:
	swig3.0 -python -c++ ternary_tree.i
	g++ -c ternary_tree_wrap.cxx -std=c++14  -I /usr/include/python3.5 -fPIC
	g++ -shared ../../obj/ternary_tree.o ../../obj/string_tools.o ternary_tree_wrap.o -o _ternary_tree.so

clean:
	rm *.o
	rm *.so
	rm ternary_tree.py
	rm ternary_tree_wrap.cxx

test:
	python3 test.py