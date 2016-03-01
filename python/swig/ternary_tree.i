%module ternary_tree
%include "stdint.i"

%{
    #define SWIG_FILE_WITH_INIT
    #include "../../src/ternary_tree.hpp"
%}

/* Let's just grab the original header file here */
%include "../../src/ternary_tree.hpp"
