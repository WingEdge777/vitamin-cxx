#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace vita {
template <typename key_t, typename compare_t = std::less<key_t>>
struct RedBlackTree {
public:
    struct Node {
        Node *fa;
        Node *ch[2];
        key_t data;
        size_t sz;
        bool red;
        bool child_dir() { return this == fa->ch[1]; }
        Node(const key_t &data) : data(data), sz(1), red(true) { fa = ch[0] = ch[1] = nullptr; }
    };
    RedBlackTree() : compare{}, root{nullptr} {}
    ~RedBlackTree() {
        post_order([](auto it) { delete it; });
    }

    // 前/中/后序遍历
    template <typename F>
    void pre_order(F callback) {
        auto f = [&](auto &&f, Node *p) {
            if (!p) return;
            callback(p), f(f, p->ch[0]), f(f, p->ch[1]);
        };
        f(f, root);
    }

    template <typename F>
    void in_order(F callback) {
        auto f = [&](auto &&f, Node *p) {
            if (!p) return;
            f(f, p->ch[0]), callback(p), f(f, p->ch[1]);
        };
        f(f, root);
    }

    template <typename F>
    void post_order(F callback) {
        auto f = [&](auto &&f, Node *p) {
            if (!p) return;
            f(f, p->ch[0]), f(f, p->ch[1]), callback(p);
        };
        f(f, root);
    }
    Node *leftmost(const Node *p) { return most(p, 0); }
    Node *rightmost(const Node *p) { return most(p, 1); }
    Node *prev(const Node *p) { return neighbour(p, 0); }
    Node *next(const Node *p) { return neighbour(p, 1); }
    Node *lower_bound(const key_t &key) {
        Node *now = root, *ans = nullptr;
        while (now) {
            if (!compare(now->data, key))
                ans = now, now = now->ch[0];
            else
                now = now->ch[1];
        }
        return ans;
    }
    Node *upper_bound(const key_t &key) {
        Node *now = root, *ans = nullptr;
        while (now) {
            if (compare(key, now->data))
                ans = now, now = now->ch[0];
            else
                now = now->ch[1];
        }
        return ans;
    }
    // start from 0
    size_t order_of_key(const key_t &key) {
        size_t ans = 0;
        auto now = root;
        while (now) {
            if (!compare(now->data, key))
                now = now->ch[0];
            else {
                ans += size(now->ch[0]) + 1;
                now = now->ch[1];
            }
        }
        return ans;
    }
    Node *find_by_order(size_t order) {
        Node *now = root, *ans = nullptr;
        while (now && now->sz >= order) {
            auto lsz = size(now->ch[0]);
            if (order < lsz)
                now = now->ch[0];
            else {
                ans = now;
                if (order == lsz) break;
                now = now->ch[1];
                order -= lsz + 1;
            }
        }
        return ans;
    }
    Node *insert(const key_t &data) {
        Node n = new Node(data);
        Node *now = root, *p = nullptr;
        bool dir = 0;
        while (now) {
            p = now;
            dir = compare(now->data, data);
            now = now->ch[dir];
        }
        insert_fixup_leaf(p, n, dir);
        return n;
    }
    bool erase(const key_t &key) {
        auto p = lower_bound(key);
        if (!p || compare(p->data, key) && compare(key, p->data)) return false;
        erase(p);
        return true;
    }
    Node *erase(Node *p) {
        if (!p) return nullptr;
        Node *res;
        if (p->ch[0] && p->ch[1]) {
            auto s = leftmost(p->ch[1]);
            std::swap(s->data, p->data);
            res = p, p = s;
        } else {
            res = next(p);
        }
        erase_fixup_branch_or_leaf(p);
        delete p;
        return res;
    }

private:
    size_t size(const Node *p) { return p ? p->sz : 0; }
    bool is_red(const Node *p) { return p ? p->red : false; }
    Node *most(Node *p, bool dir) {
        if (!p) return nullptr;
        while (p->ch[dir]) p = p->ch[dir];
        return p;
    }
    Node *neighbour(Node *p, bool dir) {
        if (!p) return nullptr;
        if (p->ch[dir]) return most(p->ch[dir], !dir); // 非叶子节点：左/右子树的最右/左子节点
        if (p == root) return nullptr;
        while (p && p->fa && p->child_dir() == dir) p = p->fa; // 叶子节点
        return p ? p->fa : nullptr;
    }
    // TODO
    void insert_fixup_leaf(Node *p, Node *n, bool dir) {}
    void erase_fixup_branch_or_leaf(Node *n) {}

    Node *root;
    const compare_t compare;
};

} // namespace vita
