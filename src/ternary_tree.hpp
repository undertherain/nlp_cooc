#include <iostream>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <vector>

#include "string_tools.hpp"

#if !defined(MAX_STR_SIZE)
#define MAX_STR_SIZE 1000
#endif

//typedef int64_t Index;


template <typename T>
class TernaryTreeNode
{
public:
    char c;
    TernaryTreeNode * left;
    TernaryTreeNode * down;
    TernaryTreeNode * right;
    T data;
    int64_t id;
    TernaryTreeNode(): left(NULL), down(NULL), right(NULL), data(0), id(-1) {}
};

class TernaryTree
{
private:
public:
    int myfunc();
    int64_t id_global = 0;
    TernaryTreeNode<int64_t> * tree;
    TernaryTree(): tree(NULL)
    {
    }
    TernaryTree(const TernaryTree & t2): tree(t2.tree)
    {

    }
//Index get_value(const TernaryTreeNode<Index> * node, const char * str) ;
    int64_t get_id(const char * str);
    void set_id(const char * str, int64_t id);
    int64_t set_id_and_increment(const char * str);
    TernaryTreeNode<int64_t> * get_node_or_create(const char * str);
    void dump_frequency(const std::string & name_file) const;
    void dump_ids(const char * name_file) const;
    void dump_dot(const std::string & name_file) const;
    void reassign_ids();
    void reassign_ids_new(std::vector<int64_t> const  & lst_new_ids);
    void populate_frequency(std::vector<int64_t> & lst_frequency ) const;
    void populate_ids(std::vector<std::string> & lst_id2word ) const;
    size_t count_nodes() const;
};

class Action {
public:
    virtual void operator()(TernaryTreeNode<int64_t>* node, unsigned int depth) = 0;
};

class ActionReassignIds: public Action {
public:
    uint64_t current_id;
    ActionReassignIds(): current_id(0) {}
    void operator()(TernaryTreeNode<int64_t>* node, unsigned int depth)
    {
        if (node->data > 0) node->id = current_id++;
    }
};

class ActionReassignIdsFreq: public Action {
public:
    std::vector<int64_t> const & lst_new_ids;
    ActionReassignIdsFreq(std::vector<int64_t> const & _lst_new_ids): lst_new_ids(_lst_new_ids) {}
    void operator()(TernaryTreeNode<int64_t>* node, unsigned int depth)
    {
        if (node->data > 0)
            if (node->id >= 0)
            {
                //   std::cerr<<"replaceing "<<node->id<<" with "<<lst_new_ids[node->id]<<"\n";
                node->id = lst_new_ids[node->id];
            }
    }
};

class ActionFile: public Action
{
protected:
    std::ofstream file;
public:
    ActionFile(std::string name_file);
    virtual ~ActionFile();
};

class ActionFileWriteFrequency: public ActionFile
{
public:
    ActionFileWriteFrequency(std::string name_file): ActionFile(name_file) {}
    void operator()(TernaryTreeNode<int64_t>* node, unsigned int depth);
};

class ActionFileWriteId: public ActionFile
{
public:
    ActionFileWriteId(std::string name_file): ActionFile(name_file) {}
    void operator()(TernaryTreeNode<int64_t>* node, unsigned int depth);
};

class ActionCountNodes: public Action
{
public:
    size_t cnt;
    ActionCountNodes(): cnt(0) {}
    void operator()(TernaryTreeNode<int64_t>* node, unsigned int depth);
};

class ActionFileWriteDot: public ActionFile
{
public:
    ActionFileWriteDot(std::string name_file);
    ~ActionFileWriteDot();
    void operator()(TernaryTreeNode<int64_t>* node, unsigned int depth);
};

class ActionPopulateFrequency: public Action
{
public:
    size_t pos;
    std::vector<int64_t> & lst_frequency;
    ActionPopulateFrequency(std::vector<int64_t> & _lst_frequency): pos(0), lst_frequency(_lst_frequency) {}
    void operator()(TernaryTreeNode<int64_t>* node, unsigned int depth);
};

class ActionPopulateIds: public Action
{
public:
    size_t pos;
    std::vector<std::string> & lst_id2word;
    ActionPopulateIds(std::vector<std::string> & _lst_id2word): pos(0), lst_id2word(_lst_id2word) {}
    void operator()(TernaryTreeNode<int64_t>* node, unsigned int depth);
};


void visit_recursively(TernaryTreeNode<int64_t> * node, unsigned int depth, Action & action);


bool trim(TernaryTreeNode<int64_t> * * pnode, int64_t threshold, unsigned int depth);