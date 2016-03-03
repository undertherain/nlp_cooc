#include "array_of_trees.hpp"
#include <fstream>
#include <boost/filesystem.hpp>

ArrayOfTrees::ArrayOfTrees(const int64_t size)
{
    rows.resize(size);
    nnz=0;
}

void ArrayOfTrees::accumulate(int64_t first, int64_t second)
{
    if ((first<0)||(second<0)) return;
    BTree & ac=rows[first];
    if (ac.find( second ) != ac.end())
        ac[second]++;
    else
    {
        ac.insert(std::make_pair(second,1));
        nnz+=1;
    }
}

double get_PMI(int64_t cnt,int64_t id1,int64_t id2,int64_t cnt_words_processed, int64_t const * frequencies)
{
    return log2((static_cast<double>(cnt)*cnt_words_processed)/(frequencies[id1]*frequencies[id2]));    
}

void ArrayOfTrees::dump_csr(const char * path, const int64_t * frequencies, int n)
{
    int64_t cnt_words_processed=0;
    for (size_t i=0;i<rows.size();i++)
        cnt_words_processed+=frequencies[i];
    bool binary=true;
    std::ofstream file;
    std::string str_path;
    auto mode=binary? std::ios::out | std::ios::binary : std::ios::out;
    str_path = (boost::filesystem::path(path) / boost::filesystem::path(binary?"bigrams.data.bin":"bigrams.data")).string();
    file.open (str_path,mode);
    if(!file) throw  std::runtime_error("can not open output file " + str_path + " , check the path");
    for (size_t first=0;first<rows.size();first++)
        for (const auto& second : rows[first]) 
        {
            float v=get_PMI(second.second,first,second.first,cnt_words_processed, frequencies);
            if (binary)
                file.write( reinterpret_cast<const char*>(&v),sizeof(v));
            else
                file<<v<<"\n";
        }
    file.close();
    //----------------------------------------
    str_path = (boost::filesystem::path(path) / boost::filesystem::path(binary?"bigrams.col_ind.bin":"bigrams.col_ind")).string();
    file.open (str_path,mode);
    for (size_t first=0;first<rows.size();first++)
        for (const auto& second : rows[first]) 
        {
            size_t v=second.first;
            if (binary)
               file.write( reinterpret_cast<const char*>(&v),sizeof(v));
            else
                file<<v<<"\n";
        }
    file.close();
    //---------------write row ptrs
    str_path = (boost::filesystem::path(path) / boost::filesystem::path(binary?"bigrams.row_ptr.bin":"bigrams.row_ptr")).string();
    file.open (str_path,mode);
    int64_t row_ptr=0;
    int64_t id_last=0;
    for (size_t first=0;first<rows.size();first++)
    {
        if (first==0) 
            {
                if (binary)
                    file.write( reinterpret_cast<const char*>(&row_ptr),sizeof(row_ptr));
                else
                    file<<row_ptr<<"\n";
            }
        else
            for (size_t k=id_last;k<first;k++)
                if (binary)
                    file.write( reinterpret_cast<const char*>(&row_ptr),sizeof(row_ptr));
                else
                    file<<row_ptr<<"\n";
            id_last=first;
            row_ptr+=rows[first].size();
        }
        for (size_t k=id_last;k<rows.size();k++)
            if (binary)
                file.write( reinterpret_cast<const char*>(&row_ptr),sizeof(row_ptr));
            else
                file<<row_ptr<<"\n";
       file.close();
}



