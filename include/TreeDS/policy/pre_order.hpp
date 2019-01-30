#pragma once

#include <functional>

#include <TreeDS/utility.hpp>

namespace md {

namespace detail {

    class pre_order_impl final {

        public:
        constexpr pre_order_impl() = default;

        template <typename Node>
        const Node* increment(const Node& from) {
            const Node* result = from.get_first_child();
            if (result != nullptr) {
                return result;
            }
            // cross to another branch (on the right)
            result = keep_calling(
                // from
                from,
                // keep calling
                std::mem_fn(&Node::get_parent),
                // until
                [](const Node& child, const Node&) {
                    return child.get_next_sibling() != nullptr;
                },
                // then return
                [](const Node& child, const Node&) {
                    return child.get_next_sibling();
                });
            if (result->is_root()) {
                return nullptr;
            } else {
                return result;
            }
        }

        template <typename Node>
        const Node* decrement(const Node& from) {
            const Node* next = from.get_parent();
            const Node* prev = &from;
            /*
         * The parent is the next node if (REMEMBER: we traverse tree in pre-order and decrement the iterator):
         *   1) The passed node is root (its parent is nullptr so the previous value is the end of the reverse iterator)
         *   2) The node is the unique child of its parent
         */
            if (!next || prev == next->get_first_child()) {
                return next;
            }
            const Node* prev_sibling = prev->get_prev_sibling();
            return prev_sibling
                ? keep_calling(*prev_sibling, std::mem_fn(&Node::get_last_child))
                : nullptr;
        }

        template <typename Node>
        const Node* go_first(const Node& root) {
            return &root;
        }

        template <typename Node>
        const Node* go_last(const Node& root) {
            return keep_calling(root, std::mem_fn(&Node::get_last_child));
        }
    };

} // namespace detail

struct pre_order {
    template <typename Node, typename Allocator>
    detail::pre_order_impl get_instance(const Allocator& = Allocator()) const {
        return detail::pre_order_impl();
    }
};

} // namespace md
