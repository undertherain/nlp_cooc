#include "file_io.hpp"
#include <boost/filesystem.hpp>
#include <cmath>

size_t get_cnt_lines(const std::string &name_file)
{
    throw (std::runtime_error("not yet implemented"));
    return 0;
}

size_t read_int(const std::string &name_file)
{
    std::ifstream is(name_file, std::ifstream::in);
    size_t res; 
    is >> res;
    return res;
}

size_t get_filesize(const std::string & name_file)
{
    std::ifstream in(name_file, std::ifstream::ate | std::ifstream::binary);
    if (!in.is_open())
    {
        std::string message="get_filesize() can not open file ";
        throw std::runtime_error (message+name_file);
    }
    return in.tellg(); 
}

double get_PMI(Index cnt,Index id1,Index id2,Vocabulary const & vocab)
{
    return log2((static_cast<double>(cnt)*vocab.cnt_words_processed)/(vocab.freq_per_id[id1]*vocab.freq_per_id[id2]));    
}

void dump_crs(std::string path_out,std::vector<Accumulator> const & counters,Vocabulary const & vocab, bool binary)
{
    std::ofstream file;
    auto mode=binary? std::ios::out | std::ios::binary : std::ios::out;
    std::string str_path = (boost::filesystem::path(path_out) / boost::filesystem::path(binary?"bigrams.data.bin":"bigrams.data")).string();
    file.open (str_path,mode);
    if(!file) throw  std::runtime_error("can not open output file " + str_path + " , check the path");
    for (size_t first=0;first<counters.size();first++)
        for (const auto& second : counters[first]) 
        {
            float v=get_PMI(second.second,first,second.first,vocab);
            if (binary)
                file.write( reinterpret_cast<const char*>(&v),sizeof(v));
            else
                file<<v<<"\n";
        }
    file.close();

    str_path = (boost::filesystem::path(path_out) / boost::filesystem::path(binary?"bigrams.col_ind.bin":"bigrams.col_ind")).string();
    file.open (str_path,mode);
    for (size_t first=0;first<counters.size();first++)
        for (const auto& second : counters[first]) 
        {
            size_t v=second.first;
            if (binary)
               file.write( reinterpret_cast<const char*>(&v),sizeof(v));
            else
                file<<v<<"\n";
        }
    file.close();

    str_path = (boost::filesystem::path(path_out) / boost::filesystem::path(binary?"bigrams.row_ptr.bin":"bigrams.row_ptr")).string();
    file.open (str_path,mode);
    Index row_ptr=0;
    Index id_last=0;
    for (size_t first=0;first<counters.size();first++)
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
            row_ptr+=counters[first].size();
        }
        for (size_t k=id_last;k<vocab.cnt_words;k++)
            if (binary)
                file.write( reinterpret_cast<const char*>(&row_ptr),sizeof(row_ptr));
            else
                file<<row_ptr<<"\n";
       file.close();
   }

void write_cooccurrence_text(std::string name_file,std::vector<Accumulator> const & counters,Vocabulary const & vocab)
{
std::ofstream file;
file.open (name_file);
if(!file) throw  std::runtime_error("can not open output file "+name_file+" , check the path");
for (size_t first=0;first<counters.size();first++)
{
    for (const auto& second : counters[first]) 
    {
      //if (t.second>0)
       // file<<first.first<<"\t"<<second.first<<"\t"<<second.second<<"\n";
        double v=log2((static_cast<double>(second.second)*vocab.cnt_words_processed)/(vocab.freq_per_id[first]*vocab.freq_per_id[second.first]));
        //file<<first<<"\t"<<second.first<<"\t"<<v<<"\n";
        file<<vocab.lst_id2word[first]<<"\t"<<vocab.lst_id2word[second.first]<<"\t"<<second.second<<"\t"<<v<<"\n"; 
    }
}

file.close();
}