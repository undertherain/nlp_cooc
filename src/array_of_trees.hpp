#include <iostream>
#include <string>
#include <cstring>
#include <cmath>
#include <ctime>
#include <exception>
#include <cstring>
#include <map>

//#include "basic_utils/utils.hpp"
//#include "basic_utils/stream_reader.hpp"

//Index cnt_words_processed;
//Index cnt_words_processed_old;

//Index cnt_bigrams;
typedef std::map<int64_t,int64_t> BTree;

//#include "basic_utils/file_io.hpp"

class ArrayOfTrees
{ 
public:
    std::vector<BTree> rows;
    void accumulate(int64_t first, int64_t second)
    {
        if ((first<0)||(second<0)) return;
        BTree & ac=rows[first];
        if (ac.find( second ) != ac.end())
            ac[second]++;
        else
        {
            ac.insert(std::make_pair(second,1));
//            cnt_bigrams++;
        }
    }
};


