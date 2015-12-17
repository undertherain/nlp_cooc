#include <iostream>
#include <string>
#include <cstring>
#include <cmath>
#include <ctime>
#include <fstream>
#include <sstream>
#include <exception>
#include <cstring>
#include <boost/tokenizer.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <map>
#include <set>
#include <unordered_map>
#include <cstdarg>
#include "options.hpp"
#include "basic_utils/stream_reader.hpp"

typedef int64_t Index;
std::string provenance;

#include "basic_utils/utils.hpp"
//#include "basic_utils/stream_reader.hpp"
#include "vocabulary.hpp"

Index cnt_words_processed;
Index cnt_bigrams;
typedef std::map<Index,Index> Accumulator;
//std::map<Index,Accumulator> counters;
std::vector<Index> freq_per_id;
std::vector<std::wstring> lst_id2word;
std::vector<Accumulator> counters;
Vocabulary vocab;
#include "basic_utils/file_io.hpp"

void accumulate(Accumulator & ac,Index w)
{
    if (ac.find( w ) != ac.end())
        ac[w]++;
    else
    {
        ac.insert(std::make_pair(w,1));
        cnt_bigrams++;
    }
}

void accumulate(Index first, Index second)
{
    if ((first<0)||(second<0)) return;
    //if (second.length()<2) return;
    //if (counters.find( first ) == counters.end())
      //  counters.insert(std::make_pair(first,Accumulator()));
    accumulate(counters[first],second);
}

void load_bigrams(std::string str_path_in,const Options & options)
{
    DirReader dr(str_path_in);
    auto size_window = options.size_window;
    boost::circular_buffer<int64_t> cb(size_window); //- size of n-grams
    for (uint64_t i =0 ; i<size_window; i++) cb.push_back(-1);
    wchar_t * word;
    while ((word=dr.get_word())!=NULL )
    {
        if (vocab.is_word_valid(std::wstring(word)))
        {
            //std::cerr<<wstring_to_utf8(std::wstring(word))<<"\n";
            cnt_words_processed++;
            Index id_current = vocab.get_id(word);
            if (word[0]==L'.')      
            {
                id_current=-1;
                for (size_t j=1;j<cb.size();j++)
                    cb.push_back(id_current);
                    continue;
            }
             //freq_per_id[id_current]++;
            if ((cnt_words_processed % 500000) == 0)
                std::cerr<<"bgrams "<<cnt_words_processed/1000<<"k of "<<vocab.cnt_words_processed/1000<<"k (" <<100.0*cnt_words_processed/vocab.cnt_words_processed<<"%)\n";
            cb.push_back(id_current);
            //auto i = cb.begin();
            auto last = cb[cb.size()-1];
            for (size_t j=0;j<cb.size()-1;j++)
            {
                //std::cerr<<"accing "<<last<<" - "<<cb[j]<<"\n";
                //std::cerr<<"accing "<<wstring_to_utf8(lst_id2word[last])<<" - "<<wstring_to_utf8(lst_id2word[cb[j]])<<"\n";
                accumulate(last,cb[j]);
                accumulate(cb[j],last);
             }
        }
    }
}

struct window_params
{
    uint64_t size_window;
    bool symmetric;
};

std::string get_str_time()
{
    std::time_t result = std::time(nullptr);
    return std::asctime(std::localtime(&result));
}


int main(int argc, char * argv[])
{
    Options options = ProcessOptions(argc,argv);
    auto str_path_in = options.path_in.string();
    auto path_out=options.path_out;
 
    std::cerr<<"loading vocabulary\n";
    std::wifstream infile((path_out / boost::filesystem::path("ids")).string());
    std::locale locale("en_US.UTF8");
    infile.imbue(locale);

    std::wstring line;
   // boost::char_separator<wchar_t> sep(L"\t");
    while (std::getline(infile, line))
    {
        //std::cerr<<wstring_to_utf8(line)<<"\n";
        std::vector<std::wstring> tokens;
        boost::split(tokens, line, boost::is_any_of(L"\t"));
        Index id= stoi(tokens[1]);
        //std::cerr<<id<<"\n";
        vocab.tree.set_id(tokens[0].c_str(),id);    
    }
//    freq_per_id.resize(vocab.cnt_words);
    //vocab.dump_ids("test_ids");
    std::cerr<<"loading frequencies\n";
    load_vector_from_raw((path_out / boost::filesystem::path("freq_per_id")).string(),freq_per_id); 
    std::cerr<<freq_per_id.size()<<" words in total, max freq = " << freq_per_id[0] << "\n";
    vocab.cnt_words=freq_per_id.size();
    lst_id2word.resize(freq_per_id.size());
    vocab.populate_ids(lst_id2word);
    for (Index i=0; i<freq_per_id.size(); i++)
        std::cerr<<i<<" - "<<wstring_to_utf8(lst_id2word[i])<<"\n";
    return 0;

    if (options.size_window<2)
        {
            std::cerr<<"window size <=1 - nothing else to be done! quiting...\n";
            return 0;
        }

    std::cerr<<"extracting bigrams\n";
    counters.resize(vocab.cnt_words);
    provenance+="windows size : "+FormatHelper::ConvertToStr(options.size_window);
    provenance+="\nfrequency weightening : PMI";
    load_bigrams(str_path_in,options);

    std::cerr<<"dumping results to disk\n";

    dump_crs_bin(path_out.string());
        write_value_to_file((path_out / boost::filesystem::path("provenance.txt")).string(),provenance);
    if (options.debug)
    {
        dump_crs(path_out.string());
        write_cooccurrence_text((path_out / boost::filesystem::path("bigrams_list")).string());
    }
    return 0;
}