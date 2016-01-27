#include <iostream>
#include <string>
#include <cstring>
#include <cmath>
#include <ctime>
#include <fstream>
#include <sstream>
#include <exception>
#include <cstring>
//#include <boost/tokenizer.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/filesystem.hpp>
#include <map>
//#include <set>
//#include <unordered_map>
#include <cstdarg>
#include "options.hpp"
#include "basic_utils/stream_reader.hpp"

typedef int64_t Index;
std::string provenance;

#include "basic_utils/utils.hpp"
//#include "basic_utils/stream_reader.hpp"
#include "vocabulary.hpp"

Index cnt_words_processed;
//Index cnt_words_processed_old;

Index cnt_bigrams;
typedef std::map<Index,Index> Accumulator;
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
            if ( (word[0]==L'.') ||  (word==std::wstring(L"eos")) ||  (word==std::wstring(L"eop")) )
            {
                if (options.obey_sentence_bounds)
                {
                    id_current=-1;
                    for (size_t j=1;j<cb.size();j++)
                        cb.push_back(id_current);
                }
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
    vocab.read_from_precomputed(path_out.string());
    std::ifstream f_prov((path_out / boost::filesystem::path("provenance.txt")).string());
    provenance = std::string((std::istreambuf_iterator<char>(f_prov)), std::istreambuf_iterator<char>());

    if (options.size_window<2)
        {
            std::cerr<<"window size <=1 - nothing else to be done! quiting...\n";
            return 0;
        }

    provenance += "cooccurrences collected on " + get_str_time();
    provenance += "source corpus : " + str_path_in + "\n";

    std::cerr<<"extracting bigrams\n";
    counters.resize(vocab.cnt_words);
    provenance+="windows size : "+FormatHelper::ConvertToStr(options.size_window);
    provenance+="\nobey sentence boundaries : ";
    provenance+=options.obey_sentence_bounds?"yes":"no";
    provenance+="\nfrequency weightening : PMI\n";
    load_bigrams(str_path_in,options);

    std::cerr<<"dumping results to disk\n";

    dump_crs(path_out.string(),counters,vocab,true);
    write_value_to_file((path_out / boost::filesystem::path("provenance.txt")).string(),provenance);
    if (options.debug)
    {
        dump_crs(path_out.string(),counters,vocab,false);
        write_cooccurrence_text((path_out / boost::filesystem::path("bigrams_list")).string(),counters,vocab);
    }
    return 0;
}