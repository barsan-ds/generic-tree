#pragma once

#include <cassert> // assert
#include <memory>  // std::unique_ptr
#include <type_traits>
#include <utility> // std::move(), std::forward()

#include <TreeDS/allocator_utility.hpp>
#include <TreeDS/node/node.hpp>
#include <TreeDS/node/struct_node.hpp>
#include <TreeDS/utility.hpp>

namespace ds {

template <typename T>
class binary_node : public node<T, binary_node<T>> {

    template <typename, typename, bool>
    friend class tree_iterator;

    template <typename, template <typename> class, typename, typename>
    friend class tree;

    template <typename A>
    friend void deallocate(A&, typename A::value_type*);

    public:
    using super = node<T, binary_node>;

    protected:
    binary_node* left  = nullptr;
    binary_node* right = nullptr;

    public:
    using node<T, binary_node>::node;

    /*   ---   Wide acceptance Copy Constructor using allocator   ---   */
    template <
        typename ConvertibleT = T,
        typename Allocator    = std::allocator<binary_node>,
        CHECK_CONVERTIBLE(ConvertibleT, T)>
    explicit binary_node(
        const binary_node<ConvertibleT>& other,
        Allocator&& allocator = std::allocator<binary_node>()) :
            node<T, binary_node<T>>(static_cast<T>(other.value)),
            left(
                other.left
                    ? attach(allocate(allocator, *other.left, allocator).release())
                    : nullptr),
            right(
                other.right
                    ? attach(allocate(allocator, *other.right, allocator).release())
                    : nullptr) {
    }

    // Construct from struct_node using allocator
    template <
        typename ConvertibleT = T,
        typename... Nodes,
        typename Allocator = std::allocator<binary_node>,
        CHECK_CONVERTIBLE(ConvertibleT, T)>
    explicit binary_node(
        const struct_node<ConvertibleT, Nodes...>& other,
        Allocator&& allocator = std::allocator<binary_node>()) :
            node<T, binary_node<T>>(other.get_value()) {
        static_assert(sizeof...(Nodes) <= 2, "A binary node must have at most 2 children.");
        if constexpr (sizeof...(Nodes) >= 1) {
            const auto& left = get_child<0>(other);
            if constexpr (!std::is_same_v<decltype(left.get_value()), std::nullptr_t>) {
                static_assert(
                    std::is_convertible_v<std::decay_t<decltype(left.get_value())>, T>,
                    "The struct_node passed has a LEFT child with a value that is not compatible with T.");
                this->left = attach(allocate(allocator, left, allocator).release());
            }
        }
        if constexpr (sizeof...(Nodes) >= 2) {
            const auto& right = get_child<1>(other);
            if constexpr (!std::is_same_v<decltype(right.get_value()), std::nullptr_t>) {
                static_assert(
                    std::is_convertible_v<std::decay_t<decltype(right.get_value())>, T>,
                    "The struct_node passed has a RIGHT child with a value that is not compatible with T.");
                this->right = attach(allocate(allocator, right, allocator).release());
            }
        }
    }

    // Move constructor
    binary_node(binary_node&& other) :
            node<T, binary_node<T>>(other.value),
            left(other.left),
            right(other.right) {
        other.move_resources_to(*this);
    }

    ~binary_node() = default;

    protected:
    binary_node* attach(binary_node* node) {
        assert(node != nullptr);
        node->parent = this;
        return node;
    }

    void move_resources_to(binary_node& node) {
        if (this->parent) {
            if (this->parent->left == this) {
                this->parent->left = &node;
            }
            if (this->parent->right == this) {
                this->parent->right = &node;
            }
        }
        if (this->left) {
            this->left->parent = &node;
        }
        if (this->right) {
            this->right->parent = &node;
        }
        this->parent = nullptr;
        this->left   = nullptr;
        this->right  = nullptr;
    }

    public:
    const binary_node* get_left() const {
        return this->left;
    }

    const binary_node* get_right() const {
        return this->right;
    }

    const binary_node* first_child() const {
        return this->left
            ? this->left
            : this->right;
    }

    const binary_node* last_child() const {
        return this->right
            ? this->right
            : this->left;
    }

    const binary_node* get_next_sibling() const {
        auto parent = this->parent;
        if (parent) {
            if (this == parent->left) return parent->right;
        }
        return nullptr;
    }

    bool is_left_child() const {
        return this->parent
            ? this == this->parent->left
            : false;
    }

    bool is_right_child() const {
        return this->parent
            ? this == this->parent->right
            : false;
    }

    long hash_code() const;

    bool operator==(const binary_node& other) const {
        // Trivial case exclusion.
        if ((this->left == nullptr) != (other.left == nullptr)
            || (this->right == nullptr) != (other.right == nullptr)) {
            return false;
        }
        // Test value for inequality.
        if (!(this->value == other.value)) {
            return false;
        }
        // Deep comparison (at this point both are either null or something).
        if (this->left && !this->left->operator==(*other.left)) {
            return false;
        }
        // Deep comparison (at this point both are either null or something).
        if (this->right && !this->right->operator==(*other.right)) {
            return false;
        }
        // All the possible false cases were tested, then it's true.
        return true;
    }

    template <
        typename ConvertibleT = T,
        typename... Nodes,
        CHECK_CONVERTIBLE(ConvertibleT, T)>
    bool operator==(const struct_node<ConvertibleT, Nodes...>& other) const {
        // Too large tree
        if (other.children_count() > 2) {
            return false;
        }
        // Test value for inequality.
        if (!(this->value == other.get_value())) {
            return false;
        }
        if constexpr (sizeof...(Nodes) >= 1) {
            const auto& left = get_child<0>(other);
            if constexpr (!std::is_same_v<decltype(left.get_value()), std::nullptr_t>) {
                static_assert(
                    std::is_convertible_v<std::decay_t<decltype(left.get_value())>, T>,
                    "The struct_node passed has a LEFT child with a value that is not compatible with T.");
                if (!this->left || !this->left->operator==(left)) {
                    return false;
                }
            } else if (this->left) {
                return false;
            }
        } else if (this->left) {
            return false;
        }
        if constexpr (sizeof...(Nodes) >= 2) {
            const auto& right = get_child<1>(other);
            if constexpr (!std::is_same_v<decltype(right.get_value()), std::nullptr_t>) {
                static_assert(
                    std::is_convertible_v<std::decay_t<decltype(right.get_value())>, T>,
                    "The struct_node passed has a RIGHT child with a value that is not compatible with T.");
                if (!this->right || !this->right->operator==(right)) {
                    return false;
                }
            } else if (this->right) {
                return false;
            }
        } else if (this->right) {
            return false;
        }
        // All the possible false cases were tested, then it's true.
        return true;
    }
}; // namespace ds

/*   ---   Expected equality properties  ---   */
template <typename T>
bool operator!=(const binary_node<T>& lhs, const binary_node<T>& rhs) {
    return !lhs.operator==(rhs);
}

template <
    typename T,
    typename ConvertibleT,
    typename... Children,
    CHECK_CONVERTIBLE(ConvertibleT, T)>
bool operator==(
    const struct_node<ConvertibleT, Children...>& lhs,
    const binary_node<T>& rhs) {
    return rhs.operator==(lhs);
}

template <
    typename T,
    typename ConvertibleT,
    typename... Children,
    CHECK_CONVERTIBLE(ConvertibleT, T)>
bool operator!=(
    const binary_node<T>& lhs,
    const struct_node<ConvertibleT, Children...>& rhs) {
    return !lhs.operator==(rhs);
}

template <
    typename T,
    typename ConvertibleT,
    typename... Children,
    CHECK_CONVERTIBLE(ConvertibleT, T)>
bool operator!=(
    const struct_node<ConvertibleT, Children...>& lhs,
    const binary_node<T>& rhs) {
    return !rhs.operator==(lhs);
}

} // namespace ds
