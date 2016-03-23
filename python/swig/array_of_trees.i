%module array_of_trees
%include "stdint.i"
%include "std_vector.i"
%include "std_map.i"

%{
    #define SWIG_FILE_WITH_INIT
    #include "../../src/array_of_trees.hpp"
%}

%include "numpy.i"

%init %{
    import_array();
%}

%apply (int64_t* INPLACE_ARRAY1,int DIM1) {(const int64_t* frequencies, int size_freq)}
%apply (int64_t* INPLACE_ARRAY1,int DIM1) {(int64_t * buffer, int n)}
%apply (float * INPLACE_ARRAY1,int DIM1) {(float * buffer_f, int n)}

%include "../../src/array_of_trees.hpp"
