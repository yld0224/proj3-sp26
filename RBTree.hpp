#ifndef RB_TREE_HPP
#define RB_TREE_HPP

#include <functional>
#include <memory>
#include <cassert>
#include <stdexcept>
#include <algorithm>

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
        size_t subTreeSize = 1;

        const bool isLeftChild() const {return parent && this == parent -> leftChild.get();}
        const bool isRightChild() const {return parent && this == parent -> rightChild.get();}
        explicit Node(T val) : value(val) {}
        explicit Node(T val, Node* pa) : value(val), parent(pa) {}
        explicit Node(T val, Node* pa, bool col) : value(val), parent(pa), isRed(col) {}
        Node() = default;
    };

    size_t getSize(Node* nd){return nd ? nd -> subTreeSize : 0;}
    bool isRed(Node* nd){
        if (!nd) {return false;}
        return nd -> isRed;
    }
    bool isRoot(Node* nd) {return nd == root.get();}

    static const Node* predecessor(Node* nd){
        if (!nd) {return nullptr;}
        if (nd -> leftChild) {
            nd = nd -> leftChild.get();
            while (nd -> rightChild) {
                nd = nd -> rightChild.get();
            }
            return nd;
        }
        while (nd -> parent && nd == nd -> parent -> leftChild.get()) {
            nd = nd -> parent;
        }
        return nd -> parent;
    }
    static const Node* successor(Node* nd){
        if (!nd) {return nullptr;}
        if (nd -> rightChild) {
            nd = nd -> rightChild.get();
            while (nd -> leftChild) {
                nd = nd -> leftChild.get();
            }
            return nd;
        }
        while (nd -> parent && nd == nd -> parent -> rightChild.get()) {
            nd = nd -> parent;
        }
        return nd -> parent;
    }

    static const Node* getFront(Node* nd){
        while (nd -> parent) {
            nd = nd -> parent;
        }
        while (nd -> leftChild) {
            nd = nd -> leftChild.get();
        }
        return nd;
    }//辅助函数,对任意node返回树中第一个node
    static const Node* getBack(Node* nd){
        while (nd -> parent) {
            nd = nd -> parent;
        }
        while (nd -> rightChild) {
            nd = nd -> rightChild.get();
        }
        return nd;
    }//辅助函数,对任意node返回树中最后一个node

    std::unique_ptr<Node>& getUniquePtrOf(Node* nd) {
        if (!nd->parent) {
            return root;
        }
        if (nd->isLeftChild()) {
            return nd->parent->leftChild;
        } else {
            return nd->parent->rightChild;
        }
    }//辅助函数,返回node的uniqueptr形式

private:
    std::unique_ptr<Node> root;
    Compare cmp = Compare();
    
public:
class iterator{
public:
    Node* holdingRoot = nullptr;
    Node* holdingNode = nullptr;
    bool isEnd = false;

    iterator() = default;
    explicit iterator(Node* nd, Node* rt) : holdingNode(nd), holdingRoot(rt) {}
    explicit iterator(bool end, Node* rt) : isEnd(end), holdingRoot(rt) {}
    ~iterator() = default;

    iterator& operator=(const iterator& other){
        holdingNode = other.holdingNode;
        holdingRoot = other.holdingRoot;
        isEnd = other.isEnd;
        return *this;
    }

    const T& operator*(){
        if (isEnd || !holdingNode) {throw std::runtime_error("Decomposing an end iterator");}
        return holdingNode -> value;
    }

    iterator& operator++(){
        if (isEnd) {return *this;}//如果已经是end迭代器,按照spec,啥也不做
        holdingNode = successor(holdingNode);
        if (!holdingNode) {isEnd = true;}//如果加到了end位置,打上end标记
        return *this;
    }
    iterator operator++(int){
        iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    iterator& operator--(){
        if (isEnd) {
            holdingNode = getBack(holdingRoot);
            isEnd = false;
            return *this;
        }//对于end迭代器,总是在--的时候指向现在树里的最后一个元素
        Node* pre = predecessor(holdingNode);
        if (pre) {holdingNode = pre;}//当前不是第一个元素的时候才赋值,否则不做改变
        return *this;
    }
    iterator operator--(int){
        iterator tmp = *this;
        --(*this);
        return tmp;
    }

    bool operator!=(const iterator& other) const {
        return holdingNode != other.holdingNode || holdingRoot != other.holdingRoot;
    }
    bool operator==(const iterator& other) const {
        return holdingNode == other.holdingNode && holdingRoot == other.holdingRoot;
    }

};

    iterator begin() const noexcept {
        if (!root) {return end();}
        Node* currentNode = root.get();
        while (currentNode -> leftChild) {
            currentNode = currentNode -> leftChild.get();
        }
        return iterator(currentNode, root.get());
    }
    iterator end() const noexcept {
        return iterator(true, root.get());
    }

    RBTree() = default;
    ~RBTree() = default;//由于使用了unique_ptr,在析构的时候删除root会递归删除树的其他部分
    size_t size() const noexcept { return root ? root->subTreeSize : 0; }

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

    iterator find(const T& val) const {
        if (!root) {return end();}
        Node* currentNode = root.get();
        while (true) {
            if (cmp(val, currentNode -> value)){
                if (currentNode -> leftChild) {
                    currentNode = currentNode -> leftChild.get();
                } else {return end();}
            } else if (cmp(currentNode -> value, val)){
                if (currentNode -> rightChild) {
                    currentNode = currentNode -> rightChild.get();
                } else {return end();}
            } else {
                return iterator(currentNode, root.get());
            }
        }
    }

    template<class ...Args>
    std::pair<iterator, bool> emplace(Args&& ...args){
        return insert(T(std::forward<Args>(args)...));
    }

    std::pair<iterator, bool> insert(const T val){//Fix:为了调用移动构造,这里传值.
        if (!root) {
            root = std::make_unique<Node>(std::move(val));
            return {iterator(root.get(), root.get()), true};
        }//插入根节点,一个黑色节点
        //为了让insert快一些,接下来就不先find一次val,而是直接向下
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
                    currentNode = currentNode -> leftChild.get();
                    break;
                }
            } else {
                if (!cmp(currentNode -> value, val)) {
                    break;
                } else {
                    if (currentNode -> rightChild) {
                        currentNode = currentNode -> rightChild.get();
                    } else {
                        Node* newNode = new Node(std::move(val), currentNode, true);
                        currentNode -> rightChild.reset(newNode);
                        hasInserted = true;
                        currentNode = currentNode -> rightChild.get();
                        break;
                    }
                }
            }
        }
        if (!hasInserted) {return {iterator(currentNode, root.get()), false};}
        //接下来maintain红黑树性质
        Node* ret = currentNode;
        updateSubTreeSize(currentNode, 1);//从下往上更新子树大小
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
                    } else {
                        rotateLeft(father -> parent -> leftChild);
                    }
                currentNode = father;
                father = currentNode -> parent;
            }//情况2:父亲和自己的方向不同,旋转并转到情况3
            if (!isRed(uncle)) {
                Node* grandFather = father -> parent;
                if (father -> isLeftChild()) {
                    rotateRight(getUniquePtrOf(grandFather));
                } else {
                    rotateLeft(getUniquePtrOf(grandFather));
                }
                grandFather -> isRed = true;
                father -> isRed = false;
                currentNode = father -> parent;
                break;
            }//情况3:父亲和自己方向相同,且叔节点是黑色
        }
        return {iterator(ret, root.get()), true};
    }

    void updateSubTreeSize(Node* cur, int delta){
        while (cur -> parent) {
            Node* nxt = cur -> parent;
            nxt -> subTreeSize += delta;
            cur = nxt;
        }
    }

    void swapNode(Node* A, Node* B) {
        std::swap(A->isRed, B->isRed);
        std::swap(A->subTreeSize, B->subTreeSize);
        std::unique_ptr<Node>& APtr = getUniquePtrOf(A);

        if (A->rightChild.get() == B) {
            Node* AParent = A->parent;
            std::unique_ptr<Node> ALeft = std::move(A->leftChild);
            std::unique_ptr<Node> BRight = std::move(B->rightChild); 

            B->parent = AParent;
            A->parent = B;
            A->leftChild = nullptr;
            A->rightChild = std::move(BRight);
            if (A->rightChild) A->rightChild->parent = A;
            B->leftChild = std::move(ALeft);
            if (B->leftChild) B->leftChild->parent = B;

            APtr.release(); 
            B->rightChild.reset(A); 
            APtr.reset(B); 
        } else {
            Node* AParent = A->parent;
            Node* BParent = B->parent;
            std::unique_ptr<Node>& BPtr = getUniquePtrOf(B);

            std::unique_ptr<Node> ALeft = std::move(A->leftChild);
            std::unique_ptr<Node> ARight = std::move(A->rightChild);
            std::unique_ptr<Node> BRight = std::move(B->rightChild); 

            A->parent = BParent;
            B->parent = AParent;

            A->leftChild = nullptr;
            A->rightChild = std::move(BRight);
            if (A->rightChild) A->rightChild->parent = A;

            B->leftChild = std::move(ALeft);
            if (B->leftChild) B->leftChild->parent = B;
            B->rightChild = std::move(ARight);
            if (B->rightChild) B->rightChild->parent = B;

            Node* rawA = APtr.release();
            Node* rawB = BPtr.release();

            APtr.reset(rawB);
            BPtr.reset(rawA);
        }
    }//辅助函数,完全交换两个节点在树上的逻辑地位
    //Credit: copilot


    size_t erase(const T& val){
        iterator iter = find(val);
        if (iter == end()) {
            return 0;
        }
        Node* currentNode = iter.holdingNode;
        if (currentNode -> leftChild && currentNode -> rightChild) {
            Node* suc = successor(currentNode);
            swapNode(currentNode, suc);
        }//情况1:当前节点有两个子节点,转情况2或3
        //Fix:不应该只是交换val,为了保持迭代器有效,交换节点
        if (currentNode -> leftChild || currentNode -> rightChild) {
            Node* father = currentNode -> parent;
            updateSubTreeSize(father, -1);//处理size的更新
            std::unique_ptr<Node> child = currentNode->leftChild ? 
                                  std::move(currentNode->leftChild) : 
                                  std::move(currentNode->rightChild);
            child -> parent = father;
            child -> isRed = false; 
            getUniquePtrOf(currentNode) = std::move(child); 
            return 1;
        }//情况2:当前节点有一个子节点,处理完就可以直接返回
        Node* vic = currentNode;
        //情况3:没有子节点,该情况是处理情况1,2后自然落到的
        if (!(currentNode -> leftChild) && !(currentNode -> rightChild)) {
            if (isRoot(currentNode)) {
                root.reset(nullptr);
                return 1;
            }
            if (isRed(currentNode)) {
                Node* father = currentNode -> parent;
                updateSubTreeSize(father, -1);
                getUniquePtrOf(currentNode).reset(nullptr);
                return 1;
            }//情况3.1:是红色或根节点
            while (!isRoot(currentNode) && !isRed(currentNode)) {
                Node* father = currentNode -> parent;
                if (currentNode -> isLeftChild()) {
                    Node* sibling = father -> rightChild.get();
                    if (isRed(sibling)) {
                        rotateLeft(getUniquePtrOf(father));
                        sibling -> isRed = false;
                        father -> isRed = true;
                        continue;
                    }//情况3.2.1: 兄弟是红色节点
                    if (!isRed(sibling) && !isRed(sibling -> leftChild.get()) 
                        && !isRed(sibling -> rightChild.get())){
                        sibling -> isRed = true;
                        currentNode = father;
                        if (currentNode -> isRed) {
                            currentNode -> isRed = false;
                            break;
                        }//特判:father是红
                        continue;
                    }//情况3.2.2: 兄弟和兄弟的孩子都是黑色节点
                    if (!isRed(sibling) && isRed(sibling -> leftChild.get())
                        && !isRed(sibling -> rightChild.get())){
                        rotateRight(getUniquePtrOf(sibling));
                        sibling -> isRed = true;
                        sibling -> parent -> isRed = false;
                        sibling = sibling -> parent;
                    }//情况3.2.3: 兄弟是黑色节点,兄弟的左孩子红右孩子黑,旋转并转到情况3.2.4
                    if (!isRed(sibling) && isRed(sibling -> rightChild.get())) {
                        rotateLeft(getUniquePtrOf(father));
                        sibling -> rightChild -> isRed = false;
                        bool colorOfFather = father -> isRed;
                        bool colorOfSibling = sibling -> isRed;
                        father -> isRed = colorOfSibling;
                        sibling -> isRed = colorOfFather;
                        break;
                    }//情况3.2.4: 兄弟是黑色节点,兄弟的右孩子红,改完终止
                }
                if (currentNode -> isRightChild()) {
                    Node* sibling = father -> leftChild.get();
                    if (isRed(sibling)) {
                        rotateRight(getUniquePtrOf(father));
                        sibling -> isRed = false;
                        father -> isRed = true;
                        continue;
                    }
                    if (!isRed(sibling) && !isRed(sibling -> leftChild.get()) 
                        && !isRed(sibling -> rightChild.get())){
                        sibling -> isRed = true;
                        currentNode = father;
                        if (currentNode -> isRed) {
                            currentNode -> isRed = false;
                            break;
                        }
                        continue;
                    }
                    if (!isRed(sibling) && isRed(sibling -> rightChild.get())
                        && !isRed(sibling -> leftChild.get())){
                        rotateLeft(getUniquePtrOf(sibling));
                        sibling -> isRed = true;
                        sibling -> parent -> isRed = false;
                        sibling = sibling -> parent;
                    }
                    if (!isRed(sibling) && isRed(sibling -> leftChild.get())) {
                        rotateRight(getUniquePtrOf(father));
                        sibling -> leftChild -> isRed = false;
                        bool colorOfFather = father -> isRed;
                        bool colorOfSibling = sibling -> isRed;
                        father -> isRed = colorOfSibling;
                        sibling -> isRed = colorOfFather;
                        break;
                    }
                }
            }
            Node* father = vic -> parent;
            updateSubTreeSize(father, -1);
            getUniquePtrOf(vic).reset(nullptr);
        }//维护平衡之后正式删除victim
        if (root) {root -> isRed = false;}
        return 1;
    }//返回0/1表示是否找到元素
};

}
#endif //RB_TREE_HPP