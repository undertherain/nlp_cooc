//#include <exception>
#include <map>
#include <vector>

typedef std::map<int64_t, int64_t> BTree;

class ArrayOfTrees
{
public:
    int64_t nnz;
    std::vector<BTree> rows;
    ArrayOfTrees(const int64_t size);
    void accumulate(int64_t first, int64_t second);
    void dump_csr(const char * path, const int64_t * frequencies, int size_freq);
    void get_row_ptr(int64_t * buffer, int n);
    void get_data(int64_t * buffer, int n);
    void get_data_PMI(float * buffer_f, int n, const int64_t * frequencies, int size_freq);
    void get_col_ind(int64_t * buffer, int n);
};

/*
todo:
    nnz
    shape
*/
