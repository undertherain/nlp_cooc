//#include <exception>
#include <map>
#include <vector>

typedef std::map<int64_t,int64_t> BTree;

class ArrayOfTrees
{ 
public:
    int64_t nnz;
    std::vector<BTree> rows;
    ArrayOfTrees(const int64_t size);
    void accumulate(int64_t first, int64_t second);
    void dump_csr(const char * path,const int64_t * frequencies, int n);  
};

/*
todo:
    nnz
    shape
*/
