#ifndef SPLAY_TREE_HH
#define SPLAY_TREE_HH

#include <utility>
#include <list>
#include <iterator>
#include <memory>
#include <cstddef>

#include "splay_tree_node_base.hh"

template <typename NodeT>
class DynamicSizeNodePool
{
    struct Wrapped : public NodeT
    {
        using NodeT::NodeT;
        typename std::list<Wrapped>::iterator i;
    };
    std::list<Wrapped> used_list;
    std::list<Wrapped> free_list;

public:
    template <typename... Args>
    NodeT *make(Args &&...arg)
    {
        if (free_list.empty()) {
            auto &&node = used_list.emplace_back(std::forward<Args>(arg)...);
            node.i = std::prev(used_list.end());
            return std::addressof(node);
        }
        used_list.splice(used_list.end(), free_list, std::prev(free_list.end()));
        auto &&node = used_list.back();
        node.~NodeT(); // i remains unchanged
        new (std::addressof(node)) NodeT(std::forward<Args>(arg)...);
        return std::addressof(node);
    }
    // this is lazy, if sth. relies on the order of dtor, do NOT use
    void retrieve(NodeT *node)
    {
        free_list.splice(free_list.end(), used_list, ((Wrapped *)node)->i);
    }
};

template <typename Tp>
class SplayTreeArray
{
public:
    struct Node : SplayTreeNodeBase<Node>
    {
        Tp v;

        explicit Node(Tp v) : v(std::move(v)) { }
        Tp &val() { return v; }
        const Tp &val() const { return v; }
    };

    struct Proxy
    {
        friend SplayTreeArray;
        explicit Proxy() : p_() { }

    private:
        explicit Proxy(Node *p) : p_(p) { }
        Node *p_;
    };

    SplayTreeArray() : root_() { }

    Proxy pushBack(const Tp &v)
    {
        Node *t = pool_.make(v);
        root_ = Node::join(root_, t);
        return Proxy(t);
    }

    std::size_t size() const { return root_ ? root_->size() : 0; }
    std::size_t count() const { return size(); }

    void removeAt(std::size_t n)
    {
        auto [a, b] = Node::split(root_, n);
        auto [c, d] = Node::split(b, 1);
        pool_.retrieve(c);
        root_ = Node::join(a, d);
    }

    void remove(Proxy x)
    {
        x.p_->splay();
        root_ = x.p_;
        removeAt(root_->left() ? root_->left()->size() : 0);
    }

    Tp &at(std::size_t n)
    {
        Node *a = Node::select(root_, n);
        return a->val();
    }

    const Tp &at(std::size_t n) const
    {
        Node *a = Node::select(root_, n);
        return a->val();
    }

private:
    mutable Node *root_;
    DynamicSizeNodePool<Node> pool_;
};

#endif