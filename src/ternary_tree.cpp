#include <cstring>
#include <fstream>
#include <stdexcept>
#include <codecvt>
#include "ternary_tree.hpp"

//#define MAX_STR_SIZE  1500
char  buffer[MAX_STR_SIZE];

int TernaryTree::myfunc()
{
    return 34;
}

int64_t TernaryTree::set_id_and_increment(const char * str)
{
    auto node = get_node_or_create(str);
    if (!node->data) node->id=id_global++;
    node->data++;
    return node->id;    
}

void TernaryTree::set_id(const char * str,int64_t id)
{
    auto node = get_node_or_create(str);
    node->data=1;
    node->id=id;    
}

TernaryTreeNode<int64_t> * TernaryTree::get_node_or_create(const char * str)
{
    if (tree==NULL)
    { 
        //std::cerr<<"creating new head\n";
        tree = new TernaryTreeNode<int64_t>();
        tree->c=str[0];
        //tree->data=id_global++;
    } 
    auto node=tree;
    unsigned int depth=0;
    bool is_done=false;
    if ((strlen(str)==1)&&(node->c==str[0])) is_done=true;
    while (!is_done)
    {
        if (depth>=MAX_STR_SIZE) {
            std::cerr<<"string too long in tree, aborting \n"; 
            throw std::exception();
        }
        char c=str[depth];
//        std::cerr<<"depth = "<<depth<<"\tnode.c = "<<node->c<<"\tc = "<<c<<"\n";
        if (c==node->c)
        {
            if (node->down==NULL) 
            {
                node->down= new TernaryTreeNode<int64_t>();
                node->down->c=str[depth+1];     
//                std::cerr<<"creating down\n";
            }
            node=node->down;
//            std::cerr<<"moving down\n";
            depth++;
            c=str[depth];
        } else
        if (c<node->c)
        {
            if (node->left==NULL)   
            {   
                node->left= new TernaryTreeNode<int64_t>();
                node->left->c=c;
//                std::cerr<<"creating left\n";
            }
            node=node->left;
//            std::cerr<<"moving left\n";
        } else
        if (c>node->c)
        {
            if (node->right==NULL) 
            {
                node->right= new TernaryTreeNode<int64_t>();
                node->right->c=c;
            }
            node=node->right;
//            std::cerr<<"moving right\n";
        }
        //if (depth>=strlen(str)) is_done=true;
        if ((depth>=strlen(str)-1)&&(node->c==c)) is_done=true;
    }
    return node;
}


int64_t TernaryTree::get_id(const char * str)
{
    auto node=tree;
    if (node==NULL) return -1;
    unsigned int depth=0;
    bool is_done=false;
    while (!is_done)
    {
        char c=str[depth];
        if (c==node->c)
        {
            if (depth>=strlen(str)-1) return node->id;
            if (node->down==NULL) return -1;
            node=node->down;
            depth++;
        } else
        if (c<node->c)
        {
            if (node->left==NULL)  return -1;
            node=node->left;
        } else
        if (c>node->c)
        {
            if (node->right==NULL) return -1;
            node=node->right;
        }
        c=str[depth];
        //std::cerr<<"depth = "<<depth<<" node.c = "<<node->c<<"\n";
        if ((depth>=strlen(str)-1)&&(node->c==c)) is_done=true;
    }
    //std::cerr<<"found at depth "<<depth<<" id = "<<node->id-1<<"\n";
    return node->id;
}


ActionFile::ActionFile(std::string name_file)
    {
        file.open (name_file);    
        if(!file)  throw  std::runtime_error("can not open output file "+name_file+" , check the path");
    }
ActionFile::~ActionFile()
    {
        file.close();
    }

void ActionFileWriteFrequency::operator()(TernaryTreeNode<int64_t>* node,unsigned int depth)
    {
    if (node->data)
//        file<<wstring_to_utf8(std::wstring(buffer,buffer+depth+1))<<"\t"<<node->data<<"\n";
        file<<std::string(buffer,buffer+depth+1)<<"\t"<<node->data<<"\n";
    }


void ActionFileWriteId::operator()(TernaryTreeNode<int64_t>* node,unsigned int depth)
    {
    if (node->data)
//        file<<wstring_to_utf8(std::wstring(buffer,buffer+depth+1))<<"\t"<<node->id<<"\n";
        file<<std::string(buffer,buffer+depth+1)<<"\t"<<node->id<<"\n";
    }

void ActionCountNodes::operator()(TernaryTreeNode<int64_t>* node,unsigned int depth)
    {
        cnt++;
    }


ActionFileWriteDot::ActionFileWriteDot(std::string name_file):ActionFile(name_file)
    {
        file<<"digraph tree {\n";
    }
ActionFileWriteDot::~ActionFileWriteDot()
    {
        file<<"}\n";
    }
    
void ActionFileWriteDot::operator()(TernaryTreeNode<int64_t>* node,unsigned int depth)
{
    std::string label(buffer,buffer+depth+1);
    //file<<wstring_to_utf8(label)<<" [label="<<node->c<<"];\n";
    file<<label<<" [label="<<label<<"];\n";
    if (node->left!=NULL)
    {
        std::string label2=label;
        label2[label2.length()-1]=node->left->c;
        file<<label<<"\t->\t"<<label2<<"  [color=blue];\n";
    }
    if (node->down!=NULL)
    {
        std::string label2=label;
        label2=label2+node->down->c;
        file<<label<<"\t->\t"<<label2<<" [style=dotted];\n";
    }
    if (node->right!=NULL)
    {
        std::string label2=label;
        label2[label2.length()-1]=node->right->c;
        file<<label<<"\t->\t"<<label2<<"[color=red] ;\n";
    }
        
    }


void visit_recursively(TernaryTreeNode<int64_t> * node,unsigned int depth, Action & action)
{
    if (node==NULL) return;
    buffer[depth]=node->c;  
    if (node->left!=NULL) visit_recursively(node->left,depth,action);
    buffer[depth]=node->c;  
    buffer[depth+1]=0;  

    action(node,depth);

    if (node->down!=NULL) visit_recursively(node->down,depth+1,action);
    buffer[depth]=node->c;  
    if (node->right!=NULL) visit_recursively(node->right,depth,action);
    buffer[depth]=node->c;  
}

void ActionPopulateFrequency::operator()(TernaryTreeNode<int64_t>* node,unsigned int depth)
{
    if (node->id>=0)
    lst_frequency[node->id]=node->data;
}

void ActionPopulateIds::operator()(TernaryTreeNode<int64_t>* node,unsigned int depth)
{
    if (node->id>=0)
    lst_id2word[node->id]=std::string(buffer,buffer+depth+1);
}


bool trim(TernaryTreeNode<int64_t> * * pnode, int64_t threshold, unsigned int depth)
{
    auto node = * pnode;
    if (node==NULL) return true;

    buffer[depth]=node->c;  

    int cnt_kids=3;
    if (trim(&(node->left),threshold,depth))
    {
        cnt_kids--;
    }
    if (trim(&(node->down),threshold,depth+1))
    {
        cnt_kids--;
    }

    if (trim(&(node->right),threshold,depth))
    {
        cnt_kids--;
    }

    if (node->data<threshold)
    {
        if (cnt_kids==0)
        {
            delete *pnode;
            *pnode=NULL;
            return true;
        }
        else
        {
            node->id=-1;
            node->data=0;
        }
    }
    return false;
}



void TernaryTree::populate_frequency(std::vector<int64_t> & lst_frequency ) const
{
    ActionPopulateFrequency a(lst_frequency);
    visit_recursively(tree,0,a);
}

void TernaryTree::populate_ids(std::vector<std::string> & lst_id2word ) const
{
    ActionPopulateIds a(lst_id2word);
    visit_recursively(tree,0,a);
}


void TernaryTree::dump_frequency(const std::string & name_file) const
{
    ActionFileWriteFrequency a(name_file);
    visit_recursively(tree,0,a);
}

void TernaryTree::dump_dot(const std::string & name_file) const
{
    ActionFileWriteDot a(name_file);
    visit_recursively(tree,0,a);
}

void TernaryTree::dump_ids(const char * name_file) const
{
    ActionFileWriteId a(name_file);
    visit_recursively(tree,0,a);
}

void TernaryTree::reassign_ids()
{
    ActionReassignIds a;
    visit_recursively(tree,0,a);
    id_global=a.current_id;
}

void TernaryTree::reassign_ids_new(std::vector<int64_t> const  & lst_new_ids)
{
    ActionReassignIdsFreq a(lst_new_ids);
    visit_recursively(tree,0,a);
}

size_t TernaryTree::count_nodes() const
{
    ActionCountNodes a;
    visit_recursively(tree,0,a);
    return a.cnt;
}
