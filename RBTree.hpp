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
        std::shared_ptr<Node> parent;
        std::shared_ptr<Node> leftChild;
        std::shared_ptr<Node> rightChild;
        bool isRed = false;
        size_t subTreeSize = 0;

        const bool isLeftChild() const {return this == parent -> leftChild;}
        const bool isRightChild() const {return this == parent -> rightChild;}
        explicit Node(std::shared_ptr<Node> pa) : parent(std::move(pa)) {}
        Node() = default;
    };
private:
    std::shared_ptr<Node> root;
    size_t size = 0;
    Compare cmp = Compare();
public:
    RBTree() = default;
    void rotateRight(std::shared_ptr<Node> nd){}
    void rotateLeft(std::shared_ptr<Node> nd){}
    ~RBTree(){}
};

}
#endif //RB_TREE_HPP