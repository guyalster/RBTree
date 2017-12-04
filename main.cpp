
#include <vector>
#include <iostream>
#include <unordered_map>
#include <list>
#include <sstream>
#include <string>
#include "RbTree.h"

using namespace std;
using namespace RB;


int main() {

    auto comp = [](const string &s1, const string &s2)->bool{
        return ((s1.size() < s2.size()) || (s1.size() == s2.size() && s1 < s2));
    };

    RBTree<string,int,decltype(comp)> tree(comp);
    tree.insert("change",5);
    tree.insert("hi",6);
    tree.insert("poll",7);
    tree.insert("shalomolam",8);
    tree.insert("b",9);
    tree.insert("a",10);
    tree.insert("c",134);
    tree.insert("cfsd",11);
    tree.insert("please",14);

    auto found = tree.find("please");
    cout <<  found->first << " " << found->second << endl;

    return 0;
}


