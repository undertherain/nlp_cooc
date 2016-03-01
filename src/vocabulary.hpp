#ifndef VOCABULARY
#define VOCABULARY

#include <iostream>
#include <string>  
#include <locale>
#include <boost/locale/encoding_utf.hpp>
//#include <codecvt>
#include <set>
//#include <boost/tokenizer.hpp>
//#include "basic_utils/stream_reader.hpp"
//#include "basic_utils/utils.hpp"
#include "ternary_tree.hpp"

class Vocabulary
{
	std::set<std::wstring> stopwords;
public:
    std::vector<std::string> lst_id2word;
    std::vector<int64_t> freq_per_id;
    TernaryTree tree;
    size_t cnt_words;
    size_t cnt_words_processed;
    const std::locale locale;
    Vocabulary();
    bool is_word_valid(std::wstring const & w);
    void read_from_dir(std::string dir);
    void read_from_precomputed(std::string dir);
    void dump_frequency(const std::string & name_file) const;
    void dump_ids(const std::string & name_file) const;
    void populate_frequency();
    void populate_ids();
    void reassign_ids(std::vector<int64_t> const & lst_frequency);
	int64_t get_id(const wchar_t * str) ;
	void reduce(int64_t threshold=5);
};

#endif