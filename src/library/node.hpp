#ifndef NODE_H
#define NODE_H

#include <string.h>
#include <boost/filesystem.hpp>
using namespace boost;
using namespace std;
#include <vector>

class node {
public:
    /*node(const string path_name)
    :node_path(path_name){
    };*/
    node(filesystem::path path)
    :node_path(path){
    };

    bool is_a_directory();
    bool contains_song_ini();

   ~node(){};
protected:
    filesystem::path node_path;

};


class music:
    public node {


};

class directory:
    public virtual node {
public:
    directory(const string path)
    :node(path) {
    };
    directory(filesystem::path path)
    :node(path){
    };
    directory(node _node)
    :node(_node){
    };

    void list_dir(int indent = 0);

};

#endif
