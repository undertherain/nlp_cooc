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
#include <map>
#include <set>
#include <unordered_map>
#include <cstdarg>
#include "options.hpp"
#include "basic_utils/stream_reader.hpp"

typedef int64_t Index;
std::string provenance;

#include "basic_utils/utils.hpp"
#include "vocabulary.hpp"


//Index cnt_bigrams;
typedef std::map<Index,Index> Accumulator;
std::vector<Accumulator> counters;

Vocabulary vocab;

#include "basic_utils/file_io.hpp"

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
    if (boost::filesystem::create_directory(path_out))
    {
        std::cerr << "creating target directory\n";
    }
    provenance = std::string();
    provenance += "vocab collected on " + get_str_time();
    provenance += "source corpus : " + str_path_in + "\n";

    std::cerr<<"assigning ids\n";
    vocab.read_from_dir(str_path_in);

    provenance = provenance + "words in corpus : "+ FormatHelper::ConvertToStr(vocab.cnt_words_processed)+"\n";
    provenance = provenance + "unique words : "+ FormatHelper::ConvertToStr(vocab.cnt_words)+"\n";
    
    vocab.reduce(options.min_frequency);
    provenance=provenance+"filtered with minimal frequency: "+FormatHelper::ConvertToStr(options.min_frequency)+"\n";
    provenance = provenance + "unique words : "+ FormatHelper::ConvertToStr(vocab.cnt_words)+"\n";

    std::cerr<<"creating list of frequencies\n";

    vocab.freq_per_id.resize(vocab.cnt_words);
    vocab.lst_id2word.resize(vocab.cnt_words);
    std::fill (vocab.freq_per_id.begin(),vocab.freq_per_id.end(),0);   
    std::cerr<<"populating frequencies\n";
    vocab.populate_frequency();
    vocab.reassign_ids(vocab.freq_per_id);
    vocab.populate_frequency();
    vocab.populate_ids();
    
    std::cerr<<"dumping ids and frequencies\n";

    vocab.dump_ids((path_out / boost::filesystem::path("ids")).string());
    vocab.dump_frequency((path_out / boost::filesystem::path("frequencies")).string());
    write_value_to_file((path_out / boost::filesystem::path("cnt_unique_words")).string(),vocab.cnt_words);
    write_value_to_file((path_out / boost::filesystem::path("cnt_words")).string(),vocab.cnt_words_processed);
    write_vector_to_file((path_out / boost::filesystem::path("freq_per_id")).string(),vocab.freq_per_id);
    write_value_to_file((path_out / boost::filesystem::path("provenance.txt")).string(),provenance);

    return 0;
}