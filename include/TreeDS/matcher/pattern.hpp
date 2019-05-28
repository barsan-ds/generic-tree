#pragma once

#include <cstddef> // std::size_t

#include <TreeDS/basic_tree.hpp>
#include <TreeDS/matcher/node/matcher.hpp>
#include <TreeDS/tree.hpp>

namespace md {

template <typename PatternTree>
class pattern {

    protected:
    PatternTree pattern_tree;

    public:
    pattern(PatternTree&& tree) :
            pattern_tree(tree) {
    }

    template <typename Node, typename Policy, typename Allocator>
    bool match(basic_tree<Node, Policy, Allocator>& tree) {
        pattern_tree.reset();
        return this->pattern_tree.match_node(tree.root, tree.get_node_allocator());
    }

    template <typename Node, typename Policy, typename Allocator>
    bool match(const basic_tree<Node, Policy, Allocator>& tree) {
        pattern_tree.reset();
        return this->pattern_tree.match_node(tree.root, tree.get_node_allocator());
    }

    template <typename Node, typename Policy, typename Allocator>
    void assign_result(tree<Node, Policy, Allocator>& tree) {
        tree = pattern_tree.get_matched_node(tree.allocator);
    }

    template <std::size_t Index, typename Node, typename Policy, typename Allocator>
    void assign_capture(capture_index<Index> index, tree<Node, Policy, Allocator>& tree) {
        tree = pattern_tree.get_captured_node(index, tree.allocator);
    }

    template <char... Name, typename Node, typename Policy, typename Allocator>
    void assign_capture(capture_name<Name...> name, tree<Node, Policy, Allocator>& tree) {
        tree = pattern_tree.get_captured_node(name, tree.allocator);
    }

    std::size_t size() const {
        return this->pattern_tree.capture_size();
    }
};

} // namespace md
