/*
 * RBtree.cpp
 *
 *  Created on: Nov 23, 2017
 *      Author: galster
 */

#ifndef __RBTREE__
#define __RBTREE__

#include <memory>
#include <utility>
#include <type_traits>

namespace RB{

    template <typename K, class Compare,typename ...V>
    class RBTree{

        using key = K;
       // using val = V;
        using comp = Compare;
        using value_type = std::tuple<K,V...>;

        class RBNode{

            using lNode = std::shared_ptr<RBNode>;
            using rNode = std::shared_ptr<RBNode>;
            using pNode = std::weak_ptr<RBNode>;

            friend class RBTree;
            friend class iterator;

            std::tuple<key,V...> __kv;

            enum class Color{
                red,black
            } __color;

            lNode __left;
            rNode __right;
            pNode __parent;

        public:

            RBNode():__color(Color::red){}

            RBNode(const key& k, V... v):
                    __kv(std::tuple_cat(std::tuple<key>(k),std::tuple<V...>(v...))),
                    __color(Color::red) {
            }

            RBNode(const RBNode &other):
                    __kv(other.__kv),
                    __color(other.__color),
                    __left(other.__left),
                    __right(other.__right),
                    __parent(other.__parent){}

            RBNode(RBNode &&other):
                    __kv(std::move(other.__kv)),
                    __color(std::move(other.__color)),
                    __left(std::move(other.__left)),
                    __right(std::move(other.__right)),
                    __parent(std::move(other.__parent)){}

            RBNode& operator=(const RBNode &other){

                if(&other != this){
                    __kv = other.__kv;
                    __color = other.__color;
                    __left.reset();
                    __right.reset();
                    __left = other.__left;
                    __right = other.__right;
                    __parent = other.__parent;
                }

                return *this;
            }

            RBNode& operator=(const RBNode && other){
                if(&other != this){
                    __kv = std::move(other.__kv);
                    __color = std::move(other.__color);
                    __left = std::move(other.__left);
                    __right = std::move(other.__right);
                    __parent = std::move(other.__parent);
                }

                return *this;
            }

            friend bool operator<(const RBNode &f, const RBNode &s){
                return comp()(std::get<0>(f.__kv),std::get<0>(s.__kv));
            }
        };

        using RBPtr = std::shared_ptr<RBNode>;

        RBPtr root;
        comp  cmp;

        static RBPtr getleftMost(RBPtr n){

            while(n->__left){
                n = n->__left;
            }

            return n;
        }

        static RBPtr getParent(RBPtr p){

            if(!p){
                return nullptr;
            }

            return( p->__parent).lock();
        }

        void rotateLeft(RBPtr n){
            RBPtr right = n->__right;
            RBPtr newRight = right->__left;
            n->__right = newRight;
            right->__left = n;
            right->__parent = n->__parent;
            n->__parent = right;

            RBPtr gpp = getParent(right);

            if(gpp){
                if(gpp->__left == n){
                    gpp->__left = right;
                }
                else{
                    gpp->__right = right;
                }
            }

            if(n == root){
                root = right;
            }
        }

        void rotateRight(RBPtr n){
            RBPtr left = n->__left;
            RBPtr newLeft = left->__right;
            n->__left = newLeft;
            left->__right = n;
            left->__parent = n->__parent;
            n->__parent = left;

            RBPtr gpp = getParent(left);

            if(gpp){
                if(gpp->__left == n){
                    gpp->__left = left;
                }
                else{
                    gpp->__right = left;
                }
            }

            if(n == root){
                root = left;
            }
        }

        void rotate(RBPtr node,RBPtr p, RBPtr gp, RBPtr uncle, bool dir){
            if(node == p->__left && dir){
                node->__parent = gp;
                p->__left = node->__right;
                node->__right = p;
                p->__parent = node;
                gp->__right = node;

                node = p;
                p = getParent(node);

            }
            else if(node == p->__right && !dir){
                node->__parent = gp;
                p->__right = node->__left;
                node->__left = p;
                p->__parent = node;
                gp->__left = node;

                node = p;
                p = getParent(node);
            }

            p->__color = RBNode::Color::black;
            gp->__color = RBNode::Color::red;

            if(!dir){
                rotateRight(gp);
            }
            else{
                rotateLeft(gp);
            }
        }

        void recolor(RBPtr startWith){

            if(startWith == root){
                startWith->__color = RBNode::Color::black;
                return;
            }

            RBPtr p = getParent(startWith);

            if(!p){
                throw std::string("couldn't acquire shared ptr");
            }

            if(p->__color == RBNode::Color::black){
                return;
            }

            RBPtr gp = getParent(p);

            if(!gp){
                throw std::string("couldn't acquire shared ptr");
            }

            RBPtr uncle;
            bool dir = false;

            if(p == gp->__left){
                uncle = gp->__right;
            }
            else{
                uncle = gp->__left;
                dir = true;
            }

            if(uncle && uncle->__color == RBNode::Color::red){

                uncle->__color = RBNode::Color::black;
                p->__color = RBNode::Color::black;
                gp->__color = RBNode::Color::red;
                recolor(gp);
            }
            else{
                rotate(startWith,p,gp,uncle,dir);
            }
        }

        RBPtr find(RBPtr r, const K& key) {
            while (r) {
                if (r->__kv == key) {
                    break;
                }

                if (cmp(std::get<0>(r->__kv), key)) {
                    r = r->__left;
                } else {
                    r = r->__right;
                }
            }

            return r;
        }

        template <typename F,typename R>
        void fixWithSibling(RBPtr &toFix,RBPtr &p,F getLeft, F getRight,R RLeft, R RRight){

            RBPtr sibling = getRight(p);

            if(sibling->__color == RBNode::Color::red){
                sibling->__color = RBNode::Color::black;
                RLeft(p);
                sibling = getRight(p);
            }

            if(getLeft(sibling)->__color == RBNode::Color::black &&
               getRight(sibling)->__color == RBNode::Color::black){
                sibling->__color = RBNode::Color::red;
                toFix = p;
            }
            else if(getRight(sibling)->__color == RBNode::Color::black){
                getLeft(sibling)->__color = RBNode::Color::black;
                sibling->__color = RBNode::Color::red;
                RRight(sibling);
                sibling = getRight(p);
            }

            sibling->__color = p->__color;
            p->__color = RBNode::Color::black;
            getRight(sibling)->__color = RBNode::Color::black;
            RLeft(p);
            toFix = root;
        }


        void fixAfterDelete(RBPtr toFix){
            while(toFix != root &&
                    toFix->__color == RBNode::Color::black) {

                RBPtr p = getParent(toFix);

                if (toFix == p->__left) {
                    fixWithSibling(toFix, p, [](RBPtr n) -> RBPtr { return n->__left; },
                                   [&](RBPtr n) -> RBPtr { return n->__right; },
                                   [&](RBPtr n) { rotateLeft(n); },
                                   [&](RBPtr n) { rotateRight(n); });

                } else {
                    fixWithSibling(toFix, p, [](RBPtr n) -> RBPtr { return n->__right; },
                                   [&](RBPtr n) -> RBPtr { return n->__left; },
                                   [&](RBPtr n) { rotateRight(n);},
                                   [&](RBPtr n) { rotateLeft(n);});
                }
            }

            toFix->__color = RBNode::Color ::black;
        }

        void erase(RBPtr toErase){
            RBPtr erase = nullptr;

            if(toErase->__left == nullptr ||
                    toErase->__right == nullptr){
                erase = toErase;
            }
            else{
                RBPtr succ = toErase->__right;

                while(succ->__left){
                    succ = succ->__left;
                }

                erase = succ;
            }

            RBPtr x = (erase->__right ? erase->__right : erase->__left);
            x->__parent = erase->__parent;

            if(x->__parent == nullptr){
              root = x;
            }

            RBPtr p = getParent(erase);

            if(erase == p->__left){
                p->__left = x;
            }
            else{
                p->__right = x;
            }

            if(erase != toErase){
                swap(erase->__kv,toErase->__kv);
            }

            if(erase->__color == RBNode::Color::black){
                fixAfterDelete(x);
            }
        }

    public:

        class iterator{

            friend class RBTree;
            RBPtr node;

        public:

            iterator(RBPtr n):
                    node(n){
            }

            iterator(const iterator &other)
                    :
                    node(other.node){}

            iterator(iterator &&other):
                    node(std::move(other.node)){}

            iterator& operator=(const iterator &other){
                if(&other != this){
                    node = other.node;
                }

                return *this;
            }

            iterator& operator=(const iterator &&other){

                if(&other != this){
                    node = std::move(other.node);
                }

                return *this;
            }

            bool operator==(const iterator &other) const {
                return ((other.node) == node);
            }

            bool operator!=(const iterator &other) const {
                return !(operator==(other));
            }

            value_type operator*() const{
                return node->__kv;
            }

            value_type *operator->(){
                return &(node->__kv);
            }

            iterator &operator++(){

                if(node->__right){
                    node = getleftMost(node->__right);
                }
                else{

                    while(1){
                        RBPtr p = getParent(node);

                        if(!p){
                            node = nullptr;
                            break;
                        }

                        if(node == p->__left){
                            node = p;
                            break;
                        }

                        node = p;
                    }
                }

                return *this;
            }

            iterator operator++(int){

                iterator ret(this->node);

                if(node->__right){
                    node = getleftMost(node->__right);
                }
                else{

                    while(1){

                        RBPtr p = getParent(node);

                        if(!p){
                            node = nullptr;
                            break;
                        }

                        if(node == p->__left){
                            node = p;
                            break;
                        }

                        node = p;
                    }
                }

                return ret;
            }
        };

        RBTree(){}

        RBTree(const comp &c):
            cmp(c){}

        RBTree(const RBTree &other):
                root(other.root){}

        RBTree(RBTree &&other){
            swap(root,other.root);
        }

        iterator begin(){
            RBPtr left = getleftMost(root);
            return iterator(left);
        }

        iterator end(){
            return iterator(nullptr);
        }

        void insert(const key& k, V... v){
            if(!root){
                root = std::make_shared<RBNode>(k,v...);
                root->__color = RBNode::Color::black;
            }
            else{

                RBPtr temp = root;

                while(temp){
                    if(cmp(k,std::get<0>(temp->__kv))){
                        if(temp->__left){
                            temp = temp->__left;
                        }
                        else{
                            temp->__left = std::make_shared<RBNode>(k,v...);
                            temp->__left->__parent = temp;
                            temp = temp->__left;
                            break;
                        }
                    }
                    else{
                        if(temp->__right){
                            temp = temp->__right;
                        }
                        else{
                            temp->__right = std::make_shared<RBNode>(k,v...);
                            temp->__right->__parent = temp;
                            temp = temp->__right;
                            break;
                        }
                    }
                }

                recolor(temp);
            }
        }

        void erase(const K& key){
            return erase(find(root,key));
        }

        void erase(iterator i){
            if(i == end()){
                return;
            }

            RBPtr node = i.node;
            erase(node);
        }

        iterator find(const key &k){
            return iterator(find(root,k));
        }
    };
};
#endif
