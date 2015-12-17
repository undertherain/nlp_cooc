#include <map>
#include <vector>
#include "../vocabulary.hpp"

typedef int64_t Index;

typedef std::map<Index,Index> Accumulator;

size_t get_cnt_lines(const std::string &name_file);
size_t read_int(const std::string &name_file);
size_t get_filesize(const std::string & name_file);
void dump_crs(std::string path_out,std::vector<Accumulator> const & counters,Vocabulary const & vocab, bool binary);
void write_cooccurrence_text(std::string name_file,std::vector<Accumulator> const & counters,Vocabulary const & vocab);

template <typename T>
size_t load_from_raw(std::string name_file,T*&buffer)
{
    size_t size_file = get_filesize(name_file);
    size_t cnt_items = size_file / sizeof(T);
    if (buffer==NULL)
        buffer = new T[cnt_items];
    std::ifstream is(name_file, std::ifstream::in | std::ifstream::binary);
    is.read(reinterpret_cast<char *>(buffer),size_file);
    return cnt_items;
}

template <typename T>
size_t load_vector_from_raw(std::string name_file,std::vector<T>& buffer)
{
    size_t size_file = get_filesize(name_file);
    size_t cnt_items = size_file / sizeof(T);
    buffer.resize(cnt_items);

    std::ifstream is(name_file, std::ifstream::in | std::ifstream::binary);
    is.read(reinterpret_cast<char *>(&buffer[0]),size_file);
    return cnt_items;
}


template<typename Type> 
void write_value_to_file(std::string name,Type value)
{
    std::ofstream file(name);
    file<<value;
    file.close();
}

template<typename Type> 
void write_values_to_file(std::ofstream &file,Type value)
{
    file<<value<<'\n';
}

template<typename T, typename ... Types> 
void write_values_to_file(std::ofstream &file,T head, Types ... tail)
{
    file<<head<<'\t';
    write_values_to_file(file,tail...);
}
template<typename ... Types> 
void write_values_to_file(std::string name, Types ... args)
{
    std::ofstream file(name);
//    std::cerr<<sizeof...(Types);//<<'\n';
    write_values_to_file(file,args...);
    file.close();
}
template<typename ... Types> 
void append_values_to_file(std::string name, Types ... args)
{
    std::ofstream file(name, std::ofstream::app);
//    std::cerr<<sizeof...(Types);//<<'\n';
    write_values_to_file(file,args...);
    file.close();
}

template <typename T>
void write_vector_to_file(std::string name,std::vector<T> const &  values)
{
    std::ofstream file(name);
    for (size_t i=0;i<values.size();i++)
//      file<<i<<"\t"<<values[i]<<"\n";
    {
        T v=values[i];
        file.write( reinterpret_cast<const char*>(&v),sizeof(T));
    }
     //   file<<values[i]<<"\n";
    file.close();
}

