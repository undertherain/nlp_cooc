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

#include "basic_utils/utils.hpp"
#include "vocabulary.hpp"
#include "json.hpp"

using json = nlohmann::json;
typedef std::map<Index,Index> Accumulator;
std::vector<Accumulator> counters;
json metadata;

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
    auto res = std::asctime(std::localtime(&result));
    return std::string(res);
}


int main(int argc, char * argv[])
{
    auto time_start = std::time(nullptr);
    Options options = ProcessOptions(argc,argv);
    auto str_path_in = options.path_in.string();
    auto path_out=options.path_out;
    if (boost::filesystem::create_directory(path_out))
    {
        std::cerr << "creating target directory\n";
    }
    metadata["timestamp"] = get_str_time();
    metadata["path_source"] = str_path_in;

    std::cerr<<"assigning ids\n";
    vocab.read_from_dir(str_path_in);

    metadata["size_corpus"] = vocab.cnt_words_processed;
    metadata["cnt_words_untrimmed"] = vocab.cnt_words;
    
    vocab.reduce(options.min_frequency);
    metadata["min_frequency"] = options.min_frequency;
    metadata["cnt_words"] = vocab.cnt_words;

    std::cerr<<"creating list of frequencies\n";

    vocab.freq_per_id.resize(vocab.cnt_words);
    vocab.lst_id2word.resize(vocab.cnt_words);
    std::fill (vocab.freq_per_id.begin(),vocab.freq_per_id.end(),0);   
    std::cerr<<"populating frequencies\n";
    vocab.populate_frequency();
    vocab.reassign_ids(vocab.freq_per_id);
    vocab.populate_frequency();
    vocab.populate_ids();
    auto time_end = std::time(nullptr);    
    metadata["execution_time"] = time_end - time_start;
    std::cerr<<"dumping ids and frequencies\n";

    vocab.dump_ids((path_out / boost::filesystem::path("ids")).string());
    vocab.dump_frequency((path_out / boost::filesystem::path("frequencies")).string());
    write_vector_to_file((path_out / boost::filesystem::path("freq_per_id")).string(),vocab.freq_per_id);
    write_value_to_file((path_out / boost::filesystem::path("metadata.json")).string(), metadata.dump(4));

    return 0;
}