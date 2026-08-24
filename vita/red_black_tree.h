#pragma once

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
        bool is_right_child() { return this == fa->ch[1]; }
    };
    RedBlackTree() : compare{}, root{nullptr} {}
    ~RedBlackTree() {
        post_order([](auto it) { delete it; });
    }

    template <typename F>
    void pre_order(F callback) {}

    template <typename F>
    void in_order(F callback) {}

    template <typename F>
    void post_order(F callback) {}

private:
    size_t size(const Node *p) { return p ? p->sz : 0; }
    bool is_red(const Node *p) { return p ? p->red : false; }
    Node *root;
    const compare_t compare;
};

} // namespace vita
