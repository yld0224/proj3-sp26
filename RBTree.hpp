#ifndef RB_TREE_HPP
#define RB_TREE_HPP

#include <functional>
#include <memory>
#include <cassert>
namespace sjtu{

template<typename T, typename Compare = std::less<T>>
class RBTree{
public:
    struct Node{
        T value;
        Node* parent = nullptr;
        std::unique_ptr<Node> leftChild;
        std::unique_ptr<Node> rightChild;
        bool isRed = false;
        size_t subTreeSize = 0;

        const bool isLeftChild() const {return parent && this == parent -> leftChild.get();}
        const bool isRightChild() const {return parent && this == parent -> rightChild.get();}
        explicit Node(Node* pa) : parent(pa) {}
        Node() = default;
    };
private:
    std::unique_ptr<Node> root;
    size_t size = 0;
    Compare cmp = Compare();
public:
    RBTree() = default;
    ~RBTree() = default;//由于使用了unique_ptr,在析构的时候删除root会递归删除树的其他部分
    void rotateRight(std::unique_ptr<Node>& nd){//引用unique_ptr,防止转移所有权到局部
        assert(nd -> leftChild);
        std::unique_ptr<Node> l = std::move(nd -> leftChild);
        l -> parent = nd.parent;
        nd -> parent = l.get();
        if (l -> rightChild) {
            l -> rightChild -> parent = nd.get();
        }
        nd -> leftChild = std::move(l -> rightChild);
        l -> rightChild = std::move(nd);
        nd = std::move(l);//把node重新连接回树,否则node的parent直接丢失了node
    }
    void rotateLeft(std::unique_ptr<Node>& nd){
        assert(nd -> rightChild);
        std::unique_ptr<Node> r = std::move(nd -> rightChild);
        r -> parent = nd.parent;
        nd -> parent = r.get();
        if (r -> leftChild) {
            r -> leftChild -> parent = nd.get();
        }
        nd -> rightChild = std::move(r -> leftChild);
        r -> leftChild = std::move(nd);
        nd = std::move(r);//把node重新连接回树,否则node的parent直接丢失了node
    }
};

}
#endif //RB_TREE_HPP