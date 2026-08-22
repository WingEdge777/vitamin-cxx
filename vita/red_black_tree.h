#pragma once

#include <cstdint>
#include <functional>

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
    void post_order(F callback) {}

private:
    Node *root;
    const compare_t compare;
};
