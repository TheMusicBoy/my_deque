#ifndef _TMB_DEQUE_
#define _TMB_DEQUE_

#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>

#include "allocator.hpp"
#include "values.hpp"

namespace tmb {

#ifndef TMB_DEQUE_BUF_SIZE
#define TMB_DEQUE_BUF_SIZE 1024
#endif

const size_t init_map_size = 8;

TMB_CONSTEXPR inline std::size_t __deque_buf_size(size_t __s) {
    return (__s < TMB_DEQUE_BUF_SIZE ? std::size_t(TMB_DEQUE_BUF_SIZE / __s)
                                     : std::size_t(1));
}

struct exc {
    class length_error {};
};

template <typename _Ty, typename _Ref, typename _Ptr>
class _Deque_iterator {
 private:
    template <typename _UTy>
    using __ptr_to = typename std::pointer_traits<_Ptr>::template rebind<_UTy>;
    template <typename _UTy>
    using __iter = _Deque_iterator<_Ty, _UTy &, __ptr_to<_UTy>>;

 public:
    using iterator       = __iter<_Ty>;
    using const_iterator = __iter<const _Ty>;
    using _El_pointer    = __ptr_to<_Ty>;
    using _Map_pointer   = __ptr_to<_El_pointer>;

    static constexpr std::size_t _buffer_size() TMB_NOEXCEPT {
        return __deque_buf_size(sizeof(_Ty));
    }

    using integer_category = std::random_access_iterator_tag;
    using difference_type  = std::ptrdiff_t;
    using size_type        = std::size_t;
    using _Self            = _Deque_iterator<_Ty, _Ref, _Ptr>;

    using value_type = _Ty;
    using reference  = _Ref;
    using pointer    = _Ptr;

    _El_pointer _cur;
    _El_pointer _first;
    _El_pointer _last;
    _Map_pointer _node;

    _Deque_iterator() TMB_NOEXCEPT {}

    _Deque_iterator(_El_pointer __c, _Map_pointer __n) TMB_NOEXCEPT
        : _cur(__c),
          _first(*__n),
          _last(*__n + _buffer_size()),
          _node(__n) {}

    _Deque_iterator(const _Deque_iterator &__x) TMB_NOEXCEPT
        : _Deque_iterator(__x._cur, __x._node) {}

    template <typename _Iter,
              typename = std::_Require<std::is_same<_Self, const_iterator>,
                                       std::is_same<_Iter, iterator>>>
    _Deque_iterator(const _Iter &__x)
        : _Deque_iterator(__x._cur, __x._first, __x._last) {}

    _Deque_iterator &operator=(const _Deque_iterator &__x)
        TMB_NOEXCEPT = default;

    template <typename _Iter,
              typename std::enable_if<std::is_same<_Self, const_iterator>::value ||
                                 std::is_same<_Iter, iterator>::value,
                             bool>::type = true>
    _Deque_iterator &operator=(const _Deque_iterator &__x) {
        return *this = _Self(__x);
    }

    ~_Deque_iterator() TMB_NOEXCEPT = default;

    reference operator*() const { return *_cur; }

    pointer operator->() const { return _cur; }

    _Self &operator++() TMB_NOEXCEPT {
        ++_cur;
        if (_cur == _last) {
            _set_node(_node + 1);
            _cur = _first;
        }
        return *this;
    }

    _Self operator++(int) TMB_NOEXCEPT {
        _Self _tmp = *this;
        ++*this;
        return _tmp;
    }

    _Self &operator--() TMB_NOEXCEPT {
        if (_cur == _first) {
            _set_node(_node - 1);
            _cur = _last;
        }
        --_cur;
        return *this;
    }

    _Self operator--(int) TMB_NOEXCEPT {
        _Self _tmp = *this;
        --*this;
        return _tmp;
    }

    _Self &operator+=(difference_type __n) TMB_NOEXCEPT {
        difference_type __offset = __n + (_cur - _first);
        if (__offset >= 0 && __offset < difference_type(_buffer_size()))
            _cur += __n;
        else {
            difference_type __node_offset;
            if (__offset < 0)
                __node_offset =
                    difference_type((-__offset - 1) / _buffer_size() - 1);
            else
                __node_offset = difference_type(__offset / _buffer_size());
            _set_node(_node + __node_offset);
            _cur = _first + (__offset -
                             __node_offset *
                                 static_cast<difference_type>(_buffer_size()));
        }
        return *this;
    }

    _Self operator+(difference_type __n) TMB_NOEXCEPT {
        _Self _tmp(*this);
        return _tmp += __n;
    }

    _Self &operator-=(difference_type __n) TMB_NOEXCEPT {
        return *this += -__n;
    }

    _Self operator-(difference_type __n) TMB_NOEXCEPT {
        _Self _tmp(*this);
        return _tmp += __n;
    }

    reference operator[](difference_type __n) TMB_NOEXCEPT {
        return *(this + __n);
    }

    void _set_node(_Map_pointer __new) TMB_NOEXCEPT {
        _node  = __new;
        _first = *__new;
        _last  = _first + difference_type(_buffer_size());
    }
};

template <typename _Ty, typename _RefL, typename _PtrL, typename _RefR,
          typename _PtrR>
inline bool operator==(const _Deque_iterator<_Ty, _RefL, _PtrL> &__x,
                       const _Deque_iterator<_Ty, _RefR, _PtrR> &__y)
    TMB_NOEXCEPT {
    return __x._cur == __y._cur;
}

template <typename _Ty, typename _RefL, typename _PtrL, typename _RefR,
          typename _PtrR>
inline bool operator!=(const _Deque_iterator<_Ty, _RefL, _PtrL> &__x,
                       const _Deque_iterator<_Ty, _RefR, _PtrR> &__y)
    TMB_NOEXCEPT {
    return __x._cur != __y._cur;
}

template <typename _Ty, typename _RefL, typename _PtrL, typename _RefR,
          typename _PtrR>
inline bool operator<(const _Deque_iterator<_Ty, _RefL, _PtrL> &__x,
                      const _Deque_iterator<_Ty, _RefR, _PtrR> &__y)
    TMB_NOEXCEPT {
    return (__x._node == __y._node) ? (__x._cur < __y._cur)
                                    : (__x._node < __y._node);
}

template <typename _Ty, typename _RefL, typename _PtrL, typename _RefR,
          typename _PtrR>
inline bool operator>(const _Deque_iterator<_Ty, _RefL, _PtrL> &__x,
                      const _Deque_iterator<_Ty, _RefR, _PtrR> &__y)
    TMB_NOEXCEPT {
    return __y < __x;
}

template <typename _Ty, typename _RefL, typename _PtrL, typename _RefR,
          typename _PtrR>
inline bool operator>=(const _Deque_iterator<_Ty, _RefL, _PtrL> &__x,
                       const _Deque_iterator<_Ty, _RefR, _PtrR> &__y)
    TMB_NOEXCEPT {
    return !(__x < __y);
}

template <typename _Ty, typename _RefL, typename _PtrL, typename _RefR,
          typename _PtrR>
inline bool operator<=(const _Deque_iterator<_Ty, _RefL, _PtrL> &__x,
                       const _Deque_iterator<_Ty, _RefR, _PtrR> &__y)
    TMB_NOEXCEPT {
    return !(__y < __x);
}

template <typename _Ty, typename _RefL, typename _PtrL, typename _RefR,
          typename _PtrR>
inline typename _Deque_iterator<_Ty, _RefL, _PtrL>::difference_type operator-(
    const _Deque_iterator<_Ty, _RefL, _PtrL> &__x,
    const _Deque_iterator<_Ty, _RefR, _PtrR> &__y) TMB_NOEXCEPT {
    return static_cast<
               typename _Deque_iterator<_Ty, _RefL, _PtrL>::difference_type>(
               _Deque_iterator<_Ty, _RefL, _PtrL>::_buffer_size()) *
               (__x._node - __y._node) +
           (__x._cur - __x._first) + (__y._first - __y._cur);
}

template <typename _Ty, typename _Ref, typename _Ptr>
inline _Deque_iterator<_Ty, _Ref, _Ptr> operator+(
    std::ptrdiff_t __n,
    const _Deque_iterator<_Ty, _Ref, _Ptr> __x) TMB_NOEXCEPT {
    return __x + __n;
}

/**
 * Deque_base has implementation of some operations.
 */
template <typename _Ty, typename _Alloc>
class _Deque_base {
 protected:
    template <typename _OTy>
    using __alloc_of =
        typename std::allocator_traits<_Alloc>::template rebind_alloc<_OTy>;

    using _Ty_alloc     = __alloc_of<_Ty>;
    using _alloc_traits = std::allocator_traits<_Ty_alloc>;

    using _Ptr       = typename _alloc_traits::pointer;
    using _Ptr_const = typename _alloc_traits::const_pointer;

    using _Map_alloc        = __alloc_of<_Ptr>;
    using _Map_alloc_traits = std::allocator_traits<_Map_alloc>;

 public:
    using allocator_type = _Alloc;

    using iterator       = _Deque_iterator<_Ty, _Ty &, _Ptr>;
    using const_iterator = _Deque_iterator<_Ty, const _Ty &, _Ptr_const>;

 protected:
    using _Map_pointer = typename iterator::_Map_pointer;

    struct _Deque_impl : public _Ty_alloc {
        _Map_pointer _map;
        std::size_t _map_size;
        iterator _start;
        iterator _finish;

        _Deque_impl() TMB_NOEXCEPT : _Ty_alloc(),
                                     _map(),
                                     _map_size(0),
                                     _start(),
                                     _finish() {}

        _Deque_impl(const _Ty_alloc &__a) TMB_NOEXCEPT : _Ty_alloc(__a),
                                                         _map(),
                                                         _map_size(0),
                                                         _start(),
                                                         _finish() {}

        _Deque_impl(_Deque_impl &&) = default;

        _Deque_impl(_Ty_alloc &&__a) TMB_NOEXCEPT : _Ty_alloc(std::move(__a)),
                                                    _map(),
                                                    _map_size(0),
                                                    _start(),
                                                    _finish() {}

        void swap_data(_Deque_impl &__x) TMB_NOEXCEPT {
            std::swap(_map, __x._map);
            std::swap(_map_size, __x._map_size);
            std::swap(_start, __x._start);
            std::swap(_finish, __x._finish);
        }
    };

    _Deque_impl _impl;

 public:
    _Deque_base() : _impl() { _init_map(0); }

    _Deque_base(size_t __n) : _impl() { _init_map(__n); }

    _Deque_base(const allocator_type &__a, size_t __n = 0) : _impl(__a) {
        _init_map(__n);
    }

    _Deque_base(_Deque_base &&__x) : _impl(std::move(__x._impl)) {}

    // This constructor need for implementation.
    _Deque_base(_Deque_base &&__x, const allocator_type &__a, size_t __n)
        : _impl(__a) {
        if (__x._get_allocator() == __a) {
            if (__x._impl._map) this->_impl.swap_data(__x._impl);
        } else {
            _init_map(__n);
        }
    }

    _Ty_alloc &_get_Ty_allocator() TMB_NOEXCEPT {
        return *static_cast<_Ty_alloc *>(&this->_impl);
    }

    const _Ty_alloc &_get_Ty_allocator() const TMB_NOEXCEPT {
        return *static_cast<const _Ty_alloc *>(&this->_impl);
    }

    _Map_alloc _get_Map_allocator() const TMB_NOEXCEPT {
        return static_cast<_Map_alloc>(_get_Ty_allocator());
    }

    _Ptr _allocate_node() {
        return _alloc_traits::allocate(this->_impl,
                                       __deque_buf_size(sizeof(_Ty)));
    }

    void _deallocate_node(_Ptr __p) TMB_NOEXCEPT {
        _alloc_traits::deallocate(this->_impl, __p,
                                  __deque_buf_size(sizeof(_Ty)));
    }

    _Map_pointer _allocate_map(size_t __n) {
        auto _map_alloc = _get_Map_allocator();
        return _Map_alloc_traits::allocate(_map_alloc, __n);
    }

    void _deallocate_map(_Map_pointer __p, std::size_t __n) TMB_NOEXCEPT {
        auto _map_alloc = _get_Map_allocator();
        _Map_alloc_traits::deallocate(_map_alloc, __p, __n);
    }

    void _reallocate_map(size_t __nodes_to_add, bool __add_at_front) {
        const size_t __old_count =
            this->_impl._finish._node - this->_impl._start._node + 1;

        const size_t __new_count = __old_count + __nodes_to_add;

        _Map_pointer __new_start;
        if (this->_impl._map_size > 2 * __new_count) {
            __new_start = this->_impl._map +
                          (this->_impl._map_size - __new_count) / 2 +
                          (__add_at_front ? __nodes_to_add : 0);

            if (__new_start < this->_impl._start._node)
                std::copy(this->_impl._start, this->_impl._finish + 1,
                          __new_start);

            else
                std::copy_backward(this->_impl._start, this->_impl._finish + 1,
                                   __new_start + __old_count);
        } else {
            size_t __new_map_size =
                this->_impl._map_size +
                std::max(this->_impl._map_size, __nodes_to_add) + 2;

            _Map_pointer __new_map = this->_allocate_map(__new_map_size);

            __new_start = __new_map + (__new_map_size - __new_count) / 2 +
                          (__add_at_front ? __nodes_to_add : 0);

            std::copy(this->_impl._start._node, this->_impl._finish._node + 1,
                      __new_start);
            _deallocate_map(this->_impl._map, this->_impl._map_size);

            this->_impl._map      = __new_map;
            this->_impl._map_size = __new_map_size;
        }

        this->_impl._start._set_node(__new_start);
        this->_impl._finish._set_node(__new_start + __old_count - 1);
    }

    void _destroy_nodes(_Map_pointer __s, _Map_pointer __f) TMB_NOEXCEPT {
        for (_Map_pointer __n = __s; __n < __f; ++__n) _deallocate_node(*__n);
    }

    void _create_nodes(_Map_pointer __s, _Map_pointer __f) {
        _Map_pointer __c;
        try {
            for (__c = __s; __c < __f; ++__c) *__c = _allocate_node();
        } catch (...) {
            _destroy_nodes(__s, __f);
            throw;
        }
    }

    void _init_map(size_t __n) {
        size_t __nodes = (__n / __deque_buf_size(sizeof(_Ty)) + 1);

        this->_impl._map_size = std::max(static_cast<size_t>(init_map_size),
                                         static_cast<size_t>(__nodes + 2));
        this->_impl._map      = _allocate_map(this->_impl._map_size);

        _Map_pointer __start =
            (this->_impl._map + (this->_impl._map_size - __nodes) / 2);
        _Map_pointer __finish = __start + __nodes;

        try {
            _create_nodes(__start, __finish);
        } catch (...) {
            _deallocate_map(this->_impl._map, this->_impl._map_size);
            this->_impl._map      = _Map_pointer();
            this->_impl._map_size = 0;
            throw;
        }

        this->_impl._start = iterator(*__start, __start);
        this->_impl._finish =
            iterator(*__finish + __n % __deque_buf_size(sizeof(_Ty)), __finish);
    }
};

template <typename _Iter, typename _Alloc>
void _default_construct_range(_Iter __f, _Iter __l, _Alloc &__a) {
    _Iter __c = __f;
    try {
        using _alloc_traits = std::allocator_traits<_Alloc>;
        for (; __c != __l; ++__c)
            _alloc_traits::construct(__a, std::__addressof(*__c));
    } catch (...) {
        _destroy_range(__f, __c, __a);
        throw;
    }
}

template <typename _Iter, typename _Ty, typename _Alloc>
void _fill_construct_range(_Iter __f, _Iter __l, const _Ty &__v, _Alloc &__a) {
    _Iter __c = __f;
    try {
        using _alloc_traits = std::allocator_traits<_Alloc>;
        for (; __c != __l; ++__c)
            _alloc_traits::construct(__a, std::__addressof(*__c), __v);
    } catch (...) {
        _destroy_range(__f, __c, __a);
        throw;
    }
}

template <typename _Iter, typename _Alloc>
void _copy_construct_range(_Iter __f, _Iter __l, _Iter __r, _Alloc &__a) {
    _Iter __c = __r;
    try {
        using _alloc_traits = std::allocator_traits<_Alloc>;
        for (; __f != __l; ++__c, ++__f)
            _alloc_traits::construct(__a, std::__addressof(*__c), *__f);
    } catch (...) {
        _destroy_range(__r, __c);
        throw;
    }
}

template <typename _Iter, typename _Alloc>
void _move_construct_range(_Iter __f, _Iter __l, _Iter __r, _Alloc &__a) {
    _Iter __c = __r;
    try {
        using _alloc_traits = std::allocator_traits<_Alloc>;
        for (; __f != __l; ++__c, ++__f)
            _alloc_traits::construct(__a, std::__addressof(*__c),
                                     std::move(*__f));
    } catch (...) {
        _destroy_range(__r, __c);
        throw;
    }
}

template <typename _Iter, typename _Alloc>
void _destroy_range(_Iter __f, _Iter __l, _Alloc &__a) {
    using _alloc_traits = std::allocator_traits<_Alloc>;
    for (auto __c = __f; __c != __l; ++__c)
        _alloc_traits::destroy(__a, std::__addressof(*__c));
}

}  // namespace tmb

#endif
