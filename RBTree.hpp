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
        explicit Node(T val) : value(val) {}
        explicit Node(T val, Node* pa) : value(val), parent(pa) {}
        explicit Node(T val, Node* pa, bool col) : value(val), parent(pa), isRed(col) {}
        Node() = default;
    };
private:
    std::unique_ptr<Node> root;
    Compare cmp = Compare();
public:
    RBTree() = default;
    ~RBTree() = default;//由于使用了unique_ptr,在析构的时候删除root会递归删除树的其他部分
    size_t getSize(Node* nd){return nd ? nd -> subTreeSize : 0;}
    size_t treeSize() {return root -> subTreeSize + 1;}
    bool isRed(Node* nd){
        if (!nd) {return false;}
        return nd -> isRed;
    }
    bool isRoot(Node* nd) {return nd == root.get();}
    void rotateRight(std::unique_ptr<Node>& nd){//引用unique_ptr,防止转移所有权到局部
        assert(nd -> leftChild);
        std::unique_ptr<Node> l = std::move(nd -> leftChild);
        l -> parent = nd.parent;
        nd -> parent = l.get();
        if (l -> rightChild) {
            l -> rightChild -> parent = nd.get();
        }
        l -> subTreeSize = nd -> subTreeSize;
        nd -> subTreeSize = getSize(l -> rightChild.get()) + getSize(nd -> rightChild.get()) + 1;
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
        r -> subTreeSize = nd -> subTreeSize;
        nd -> subTreeSize = getSize(r -> leftChild.get()) + getSize(nd -> leftChild.get()) + 1;
        nd -> rightChild = std::move(r -> leftChild);
        r -> leftChild = std::move(nd);
        nd = std::move(r);
    }
    void insert(T val){//先使用void,迭代器类还没设计
        if (!root) {
            root = std::make_unique<Node>(std::move(val));
            return;
        }//插入根节点,一个黑色节点
        Node* currentNode = root.get();
        bool hasInserted = false;
        while (true) {
            if (cmp(val, currentNode -> value)){
                if (currentNode -> leftChild) {
                    currentNode = currentNode -> leftChild.get();
                } else {
                    Node* newNode = new Node(std::move(val), currentNode, true);
                    currentNode -> leftChild.reset(newNode);
                    hasInserted = true;
                    currentNode = currentNode -> leftChild;
                    break;
                }
            } else {
                if (!cmp(currentNode -> value, val)) {
                    break;
                }//不小于也不大于,两者等价
                else {
                    if (currentNode -> rightChild) {
                        currentNode = currentNode -> rightChild.get();
                    } else {
                        Node* newNode = new Node(std::move(val), currentNode, true);
                        currentNode -> rightChild.reset(newNode);
                        hasInserted = true;
                        currentNode = currentNode -> rightChild;
                        break;
                    }
                }
            }
        }
        if (!hasInserted) {return;}
        updateSubTreeSize(currentNode);//从下往上更新子树大小
        Node* father;
        Node* uncle;
        while (isRed(father = currentNode -> parent)) {
            if (father -> isLeftChild()) {
                uncle = father -> parent -> rightChild.get();
            } else {
                uncle = father -> parent -> leftChild.get();
            }//先找到叔节点
            if (isRed(uncle)) {
                uncle -> isRed = false;
                father -> isRed = false;
                father -> parent -> isRed = true;
                currentNode = father -> parent;
                if (isRoot(currentNode)) {
                    currentNode -> isRed = false;
                    break;
                }//特判,已经是root了就返回
                continue;
            }//情况1:叔节点是红色
            if ((currentNode -> isLeftChild() && father -> isRightChild()) 
                || (currentNode -> isRightChild() && father ->isLeftChild())) {
                    if (currentNode -> isLeftChild()) {
                        rotateRight(father -> parent -> rightChild);
                        currentNode = currentNode -> rightChild.get();
                        father = currentNode -> parent;
                    } else {
                        rotateLeft(father -> parent -> leftChild);
                        currentNode = currentNode -> leftChild.get();
                        father = currentNode -> parent;
                    }
            }//情况2:父亲和自己的方向不同,旋转并转到情况3
            if (!isRed(uncle)) {
                Node* grandFather = father -> parent;
                if (father -> isLeftChild()) {
                    rotateRight(grandFather);
                } else {
                    rotateLeft(grandFather);
                }
                grandFather -> isRed = true;
                father -> isRed = false;
                currentNode = father -> parent;
                if (!currentNode) {break;}//特判,当前节点已经是根节点的父节点,即nullptr
                continue;
            }//情况3:父亲和自己方向相同,且叔节点是黑色
        }
    }
    void updateSubTreeSize(Node* cur){
        while (cur -> parent) {
            Node* nxt = cur -> parent;
            nxt -> subTreeSize++;
            cur = nxt;
        }
    }
    size_t erase(){}//返回0/1表示是否找到元素
};

}
#endif //RB_TREE_HPP