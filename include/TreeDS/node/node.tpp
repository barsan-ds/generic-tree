#include <utility> // std::move()
#include <bitset> // std::bitset

namespace ds {

template <typename T>
node<T>::node() :
		_value(), _parent(nullptr), _prev_sibling(nullptr), _next_sibling(nullptr), _first_child(nullptr),
				_last_child(nullptr) {
}

template <typename T>
node<T>::node(node &&other) :
		_value(std::move(other._value)), _parent(other._parent), _prev_sibling(other._prev_sibling),
				_next_sibling(other._next_sibling), _first_child(other._first_child), _last_child(other._last_child) {
	other.nullify();
}

template <typename T>
node<T>::node(const T &value) :
		_value(value), _parent(nullptr), _prev_sibling(nullptr), _next_sibling(nullptr), _first_child(nullptr),
				_last_child(nullptr) {
}

template <typename T>
node<T>::node(T &&value) :
		_value(std::move(value)), _parent(nullptr), _prev_sibling(nullptr), _next_sibling(nullptr),
				_first_child(nullptr), _last_child(nullptr) {
}

template <typename T>
template <typename ...Args> node<T>::node(Args&& ...args) :
		_value(args...), _parent(nullptr), _prev_sibling(nullptr), _next_sibling(nullptr), _first_child(nullptr),
				_last_child(nullptr) {
}

template <typename T>
node<T>::node(tree_base<T> &&tree) :
		_value(std::move(tree._root->_value)), _parent(nullptr), _prev_sibling(nullptr), _next_sibling(nullptr),
				_first_child(nullptr), _last_child(nullptr) {

}

template <typename T>
node<T>& node<T>::operator =(node<T> &&other) {
	_value = std::move(other._value);
	_parent = other._parent;
	_prev_sibling = other._prev_sibling;
	_next_sibling = other._next_sibling;
	_first_child = other._first_child;
	_last_child = other._last_child;
	other.nullify();
}

template <typename T>
void node<T>::nullify() {
	_parent = nullptr;
	_prev_sibling = nullptr;
	_next_sibling = nullptr;
	_first_child = nullptr;
	_last_child = nullptr;
}

template <typename T>
void node<T>::unlink() {
	// Unlink tree => node
	if (_prev_sibling) {
		_prev_sibling->_next_sibling = _next_sibling;
	} else {
		_parent->first_child = _next_sibling;
	}
	if (_next_sibling) {
		_next_sibling->_prev_sibling = _prev_sibling;
	} else {
		_parent->_last_child = _next_sibling;
	}
	// Unlink node => tree
	_parent = nullptr;
	_prev_sibling = nullptr;
	_next_sibling = nullptr;
	return {this};
}

template <typename T>
void node<T>::insert(node &n) {
	// Vertical link
	n._parent = _parent;
	n._first_child = this;
	n._last_child = this;
	_parent = &n;
	// Horizontal link
	if (_prev_sibling) {
		n._prev_sibling = _prev_sibling;
		n._prev_sibling->_next_sibling = &n;
		_prev_sibling = nullptr;
	} else if (_parent) {
		_parent->_first_child = &n;
	}
	if (_next_sibling) {
		n._next_sibling = _next_sibling;
		n._next_sibling->_prev_sibling = &n;
		_next_sibling = nullptr;
	} else if (_parent) {
		_parent->_last_child = &n;
	}
}

template <typename T>
void node<T>::append_child(node &n) {
	// Link node => tree
	n._parent = this;
	n._prev_sibling = _last_child;
	n._next_sibling = nullptr;
	// Link tree => node
	if (_last_child) {
		_last_child->_next_sibling = &n;
	} else {
		// If there is no last child, then there is no first child so we have to set it
		_first_child = &n;
	}
	_last_child = &n;
}

template <typename T>
void node<T>::prepend_child(node &n) {
	// Link node => tree
	n._parent = this;
	n._prev_sibling = nullptr;
	n._next_sibling = _first_child;
	// Link tree => node
	if (_first_child) {
		_first_child->prev_sibling = &n;
	} else {
		// If there is no last child, then there is no first child so we have to set it
		_last_child = &n;
	}
	_first_child = &n;
}

template <typename T>
void node<T>::append_sibling(node &n) {
	// Link node => tree
	n._parent = _parent;
	n._prev_sibling = this;
	n._next_sibling = _next_sibling;
	// Link tree => node
	if (_next_sibling) {
		_next_sibling->_prev_sibling = &n;
	} else {
		_parent->last_child = &n;
	}
	_next_sibling = &n;
}

template <typename T>
node<T>::~node() {
	if (_next_sibling) delete _next_sibling;
	if (_first_child) delete _first_child;
}

template <typename T>
const node<T>* node<T>::parent() const {
	return _parent;
}

template <typename T>
const node<T>* node<T>::prev_sibling() const {
	return _prev_sibling;
}

template <typename T>
const node<T>* node<T>::next_sibling() const {
	return _next_sibling;
}

template <typename T>
const node<T>* node<T>::first_child() const {
	return _first_child;
}

template <typename T>
const node<T>* node<T>::last_child() const {
	return _last_child;
}

template <typename T>
template <typename Archive>
void node<T>::save(Archive &ar) const {
	// {value, has next_sibling?, has children?}
	ar << _value << static_cast<bool>(_next_sibling) << static_cast<bool>(_last_child);
}

template <typename T>
template <typename Archive>
typename node<T>::info node<T>::load(Archive &ar) {
	ar >> _value;
	bool siblings, children;
	ar >> siblings;
	ar >> children;
	return {siblings, children};
}

template <typename T>
node<T>& node<T>::operator ()(const node &n) {
	append_child(*new node(n));
	return *this;
}

template <typename T>
node<T>& node<T>::operator ,(const node &n) {
	append_sibling(*new node(n));
	return this;
}

} /* namespace ds */
