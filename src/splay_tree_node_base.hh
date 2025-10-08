#ifndef SPLAY_TREE_NODE_BASE_HH
#define SPLAY_TREE_NODE_BASE_HH

#include <array>
#include <cassert>

template <typename SplayTreeNodeT>
class SplayTreeNodeBase
{
    SplayTreeNodeBase *L;
    SplayTreeNodeBase *R;
    SplayTreeNodeBase *P;
    int Size;

    enum class Child { LEFT, RIGHT };
    [[nodiscard]] Child which() const { return P->L == this ? Child::LEFT : Child::RIGHT; }
    [[nodiscard]] bool is_root() const { return P == nullptr; }
    [[nodiscard]] bool is_left_child() const { return which() == Child::LEFT; }
    [[nodiscard]] bool is_right_child() const { return which() == Child::RIGHT; }

    [[nodiscard]] SplayTreeNodeT &underlying() { return static_cast<SplayTreeNodeT &>(*this); }
    [[nodiscard]] const SplayTreeNodeT &underlying() const
    {
        return static_cast<const SplayTreeNodeT &>(*this);
    }

    // CRTP reimplement
    void do_propagate() { }
    void do_update() { }

protected:
    // base_propagate() is called to propagate the update information to child(ren).
    // There is no need to update the information combined from child(ren)
    // which should be done in base_update().
    void base_propagate() { underlying().do_propagate(); }
    // base_update() is called to update the information combined from child(ren).
    void base_update()
    {
        Size = 1;
        if (L)
            Size += L->Size;
        if (R)
            Size += R->Size;
        underlying().do_update();
    }
    void base_rotate()
    {
        P->base_propagate();
        base_propagate();
        if (is_left_child()) {
            if ((P->L = R))
                R->P = P;
            if (!P->is_root()) {
                if (P->is_left_child())
                    P->P->L = this;
                else {
                    P->P->R = this;
                }
            }
            R = P, P = P->P, R->P = this;
            R->base_update();
        } else {
            if ((P->R = L))
                L->P = P;
            if (!P->is_root()) {
                if (P->is_left_child())
                    P->P->L = this;
                else {
                    P->P->R = this;
                }
            }
            L = P, P = P->P, L->P = this;
            L->base_update();
        }
    }
    void base_splay(SplayTreeNodeBase *guard = nullptr)
    {
        for (base_propagate(); P != guard; base_rotate()) {
            if (P->P != guard) {
                P->P->base_propagate();
                P->which() == which() ? P->base_rotate() : base_rotate();
            }
        }
        base_update();
    }

    static SplayTreeNodeBase *base_join(SplayTreeNodeBase *a, SplayTreeNodeBase *b)
    {
        assert(a == nullptr || a->is_root());
        assert(b == nullptr || b->is_root());
        if (a == nullptr)
            return b;
        if (b == nullptr)
            return a;
        a->base_propagate();
        while (a->R) {
            a = a->R;
            a->base_propagate();
        }
        a->base_splay();
        a->R = b, b->P = a;
        a->base_update();
        return a;
    }

    SplayTreeNodeBase() : L(), R(), P(), Size(1) { }

public:
    [[nodiscard]] int size() const { return Size; }

    [[nodiscard]] SplayTreeNodeT *left() const { return static_cast<SplayTreeNodeT *>(L); }
    [[nodiscard]] SplayTreeNodeT *right() const { return static_cast<SplayTreeNodeT *>(R); }
    [[nodiscard]] SplayTreeNodeT *parent() const { return static_cast<SplayTreeNodeT *>(P); }

    void update() { base_update(); }
    void splay(SplayTreeNodeT *guard = nullptr) { base_splay(guard); }

    static SplayTreeNodeT *select(SplayTreeNodeT *&root, int k)
    {
        root->base_propagate();
        while ((root->left() ? root->left()->size() : 0) != k) {
            if ((root->left() ? root->left()->size() : 0) < k) {
                k -= (root->left() ? root->left()->size() : 0) + 1;
                root = root->right();
            } else {
                root = root->left();
            }
            root->base_propagate();
        }
        root->base_splay();
        return root;
    }

    [[nodiscard]] static SplayTreeNodeT *join(SplayTreeNodeT *a, SplayTreeNodeT *b)
    {
        return static_cast<SplayTreeNodeT *>(base_join(a, b));
    }
    [[nodiscard]] static std::array<SplayTreeNodeT *, 2> split(SplayTreeNodeT *root, int k)
    {
        if (k == 0)
            return { nullptr, root };
        select(root, k - 1);
        SplayTreeNodeT *a = root->right();
        if (a) {
            root->R = a->P = nullptr;
            a->update();
        }
        return { root, a };
    }
};

#endif