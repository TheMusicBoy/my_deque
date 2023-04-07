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

template <typename _Ty, typename _Alloc = tmb::allocator<_Ty>>
class deque : protected _Deque_base<_Ty, _Alloc> {
 private:
    // Checking for a const/volatile value_type
    static_assert(std::is_same<typename std::remove_cv<_Ty>::type, _Ty>::value,
                  "a value_type must be non-const, non-volatile");
    // Checking for an allocator value_type
    static_assert(
        std::is_same<typename _Alloc::value_type, _Ty>::value,
        "allocator::value_type and deque::value_type must be the same");

    using _Base         = _Deque_base<_Ty, _Alloc>;
    using _alloc_traits = typename _Base::_alloc_traits;
    using _Ty_alloc     = typename _Base::_Ty_alloc;
    using _Map_alloc    = typename _Base::_Map_alloc;

 public:
    using value_type             = _Ty;
    using pointer                = typename _alloc_traits::pointer;
    using const_pointer          = typename _alloc_traits::const_pointer;
    using reference              = decltype(*pointer());
    using const_reference        = decltype(*const_pointer());
    using iterator               = typename _Base::iterator;
    using const_iterator         = typename _Base::const_iterator;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using allocator_type         = _Alloc;

 protected:
    static constexpr size_t _buffer_size() TMB_NOEXCEPT {
        return __deque_buf_size(sizeof(_Ty));
    }

    using _Base::_allocate_map;
    using _Base::_allocate_node;
    using _Base::_create_nodes;
    using _Base::_deallocate_map;
    using _Base::_deallocate_node;
    using _Base::_destroy_nodes;
    using _Base::_get_Ty_allocator;
    using _Base::_init_map;
    using _Base::_reallocate_map;

    using _Base::_impl;

 public:
    deque() : _Base() {}

    explicit deque(const allocator_type &__a) : _Base(__a) {}

    explicit deque(size_type __n, const allocator_type &__a = allocator_type())
        : _Base(__a, _check_init_len(__n, __a)) {
        _default_init();
    }

    explicit deque(size_type __n, const value_type &__v,
                   const allocator_type &__a = allocator_type())
        : _Base(__a, _check_init_len(__n, __a)) {}

    deque(const deque &__x) : _Base(__x._get_Ty_allocator(), __x.size()) {
        _copy_construct_range(__x.begin(), __x.end(), this->_impl._start,
                              _get_Ty_allocator());
    }

    deque(deque &&__x) : _Base(std::move(__x)) {}

    deque(const deque &__x, const allocator_type &__a)
        : _Base(__a, __x.size()) {
        _copy_construct_range(__x.begin(), __x.end(), this->_impl._start,
                              _get_Ty_allocator());
    }

    deque(deque &&__x, const allocator_type &__a)
        : _Base(std::move(__x), __a, __x.size()) {
        if (__x.get_allocator() != __a) {
            _move_construct_range(__x.begin(), __x.end(), this->_impl._start,
                                  _get_Ty_allocator());
            __x.clear();
        }
    }

    deque(std::initializer_list<value_type> __l,
          const allocator_type &__a = allocator_type())
        : _Base(__a) {
        _range_init(__l.begin(), __l.end());
    }

    template <typename _Iter, typename = std::_RequireInputIter<_Iter>>
    deque(_Iter __f, _Iter __l, const allocator_type &__a = allocator_type())
        : _Base(__a) {
        _range_init(__f, __l);
    }

    ~deque() { _destroy_data(); }

    deque &operator=(const deque &__x) {
        _destroy_data();
        _init_map(__x.size());
        _copy_construct_range(__x.begin(), __x.end(), this->_impl._start,
                              _get_Ty_allocator());
        return *this;
    }

    deque &operator=(deque &&__x) {
        _destroy_data();
        this->_impl.swap_data(__x._impl);
        return *this;
    }

    deque &operator=(std::initializer_list<value_type> __l) {
        _destroy_data();
        _range_init(__l.begin(), __l.end());
        return *this;
    }

    void assign(size_type __n, const value_type &__v) {
        _fill_assign(__n, __v);
    }

    template <typename _Iter, typename = std::_RequireInputIter<_Iter>>
    void assign(_Iter __f, _Iter __l) {
        _range_assign(__f, __l);
    }

    void assign(std::initializer_list<value_type> __l) {
        _range_assign(__l.begin(), __l.end());
    }

    allocator_type get_allocator() const TMB_NOEXCEPT {
        return _Base::get_allocator();
    }

    iterator begin() TMB_NOEXCEPT { return this->_impl._start; }

    const_iterator begin() const TMB_NOEXCEPT { return this->_impl._start; }

    iterator end() TMB_NOEXCEPT { return this->_impl._finish; }

    const_iterator end() const TMB_NOEXCEPT { return this->_impl._finish; }

    reverse_iterator rbegin() TMB_NOEXCEPT {
        return reverse_iterator(this->_impl._finish);
    }

    const_reverse_iterator rbegin() const TMB_NOEXCEPT {
        return const_reverse_iterator(this->_impl._finish);
    }

    reverse_iterator rend() TMB_NOEXCEPT {
        return reverse_iterator(this->_impl._start);
    }

    const_reverse_iterator rend() const TMB_NOEXCEPT {
        return const_reverse_iterator(this->_impl._start);
    }

    const_iterator cbegin() const TMB_NOEXCEPT { return this->_impl._start; }

    const_iterator cend() const TMB_NOEXCEPT { return this->_impl._finish; }

    const_reverse_iterator crbegin() const TMB_NOEXCEPT {
        return const_reverse_iterator(this->_impl._finish);
    }

    const_reverse_iterator crend() const TMB_NOEXCEPT {
        return const_reverse_iterator(this->_impl._start);
    }

    size_type size() const TMB_NOEXCEPT {
        return this->_impl._finish - this->_impl._start;
    }

    size_type max_size() const TMB_NOEXCEPT {
        return _max_size(_get_Ty_allocator());
    }

    void resize(size_type __n) {
        size_type __len = size();
        if (__n > __len)
            _default_append(__n - __len);
        else if (__n < __len)
            _erase_at_end(this->_impl._start +
                          static_cast<difference_type>(__n));
    }

    void resize(size_type __n, const value_type &__v) {
        size_type __len = size();
        if (__n > __len)
            _fill_insert(this->_impl._finish, __n - __len, __v);
        else if (__n < __len)
            _erase_at_end(this->_impl._start + difference_type(__n));
    }

    TMB_NODISCARD bool empty() const TMB_NOEXCEPT {
        return this->_impl._start == this->_impl._finish;
    }

    reference operator[](size_type __n) TMB_NOEXCEPT {
        return this->_impl._start[static_cast<difference_type>(__n)];
    }

    const_reference operator[](size_type __n) const TMB_NOEXCEPT {
        return this->_impl._start[static_cast<difference_type>(__n)];
    }

 protected:
    void _range_check(size_type __n) const {
        if (__n >= size() || __n < 0) throw(exceptions::out_of_range);
    }

 public:
    reference at(size_type __n) {
        _range_check(__n);
        return (*this)[__n];
    }

    const_reference at(size_type __n) const {
        _range_check(__n);
        return (*this)[__n];
    }

    reference front() TMB_NOEXCEPT { return *begin(); }

    const_reference front() const TMB_NOEXCEPT { return *begin(); }

    reference back() TMB_NOEXCEPT { return *(end() - 1); }

    const_reference back() const TMB_NOEXCEPT { return *(end() - 1); }

    void push_front(const value_type &__x) {
        this->_impl._start = _reserve_elements_at_front(1);
        _alloc_traits::construct(this->_impl, this->_impl._start, __x);
    }

    void push_front(value_type &&__x) { emplace_front(std::move(__x)); }

    void push_back(const value_type &__x) {
        _reserve_elements_at_back(1);
        _alloc_traits::construct(this->_impl, this->_impl._finish._cur, __x);
        ++this->_impl._finish;
    }

    void push_back(value_type &&__x) { emplace_back(std::move(__x)); }

    void pop_front() {
        _requires_nonempty();
        if (this->_impl._start._cur != this->_impl._start._last - 1) {
            _alloc_traits::destroy(_get_Ty_allocator(),
                                   this->_impl._finish._cur);
            ++this->_impl._start._cur;
        } else {
            _alloc_traits::destroy(_get_Ty_allocator(),
                                   this->_impl._start._cur);
            ++this->_impl._start;
            _deallocate_node(this->_impl._start._node - 1);
        }
    }

    void pop_back() {
        _requires_nonempty();
        if (this->_impl._finish._cur != this->_impl._finish._first) {
            --this->_impl._finish._cur;
            _alloc_traits::destroy(_get_Ty_allocator(),
                                   this->_impl._finish._cur);
        } else {
            --this->_impl._finish;
            _alloc_traits::destroy(_get_Ty_allocator(),
                                   this->_impl._finish._cur);
            _deallocate_node(this->_impl._finish._node + 1);
        }
    }

    template <typename... _Args>
    reference emplace_front(_Args &&...__args) {
        this->_impl._start = _reserve_elements_at_front(1);
        _alloc_traits::construct(this->_impl, this->_impl._start,
                                 std::forward(__args...));
        return front();
    }

    template <typename... _Args>
    reference emplace_back(_Args &&...__args) {
        this->_impl._finish = _reserve_elements_at_back(1);
        _alloc_traits::construct(this->_impl, this->_impl._finish - 1,
                                 std::forward(__args...));
        return back();
    }

    template <typename... _Args>
    iterator emplace(const_iterator __pos, _Args &&...__args) {
        const size_type __el_before = __pos - this->_impl._start;
        _insert(__pos, std::forward(__args...));
        return this->_impl._start + __el_before;
    }

    iterator insert(const_iterator __pos, const value_type &__x) {
        const size_type __el_before = __pos - this->_impl._start;
        _insert(__pos, __x);
        return this->_impl._start + __el_before;
    }

    iterator insert(const_iterator __pos, value_type &&__x) {
        return emplace(__pos, std::move(__x));
    }

    iterator insert(const_iterator __pos,
                    std::initializer_list<value_type> __list) {
        auto __offset = __pos - cbegin();
        _insert(__pos, __list.begin(), __list.end());
        return begin() + __offset;
    }

    iterator insert(const_iterator __pos, size_type __n,
                    const value_type &__x) {
        auto __offset = __pos - cbegin();
        _insert(__pos, __n, __x);
        return begin() + __offset;
    }

    template <typename _Iter, typename = std::_RequireInputIter<_Iter>>
    iterator insert(const_iterator __pos, _Iter __f, _Iter __l) {
        auto __offset = __pos - cbegin();
        _insert(__pos, __f, __l);
        return begin() + __offset;
    }

    iterator erase(const_iterator __pos) { return _erase(__pos); }

    iterator erase(const_iterator __first, const_iterator __last) {
        return _erase(__first, __last);
    }

    void swap(deque &__x) TMB_NOEXCEPT {
        this->_impl._swap_data(__x._impl);
        _alloc_traits::_S_on_swap(_get_Ty_allocator(), __x._get_Ty_allocator());
    }

    void clear() TMB_NOEXCEPT { _erase_at_end(begin()); }

    struct exceptions {
        struct invalid_init_size {};
        struct out_of_range {};
    };

 protected:
    static size_type _max_size(const _Ty_alloc &__a) TMB_NOEXCEPT {
        return _alloc_traits::max_size(__a);
    }

    static size_t _check_init_len(size_t __n, const allocator_type &__a) {
        if (__n > _max_size(__a) || __n < 0)
            throw(exceptions::invalid_init_size);
        return __n;
    }

    void _default_init() {
        _default_construct_range(this->_impl._start, this->_impl._finish,
                                 _get_Ty_allocator());
    }

    void _fill_init(const value_type &__v) {
        _fill_construct_range(this->_impl._start, this->_impl._finish, __v,
                              _get_Ty_allocator());
    }

    template <typename _Iter>
    void _range_init(_Iter __f, _Iter __l) {
        size_type __n = std::distance(__f, __l);
        _init_map(_check_init_len(__n, _get_Ty_allocator()));
        _copy_construct_range(__f, __l, this->_impl._start,
                              _get_Ty_allocator());
    }

    template <typename _Iter>
    void _range_assign(_Iter __f, _Iter __l) {
        const size_type __len = std::distance(__f, __l);
        if (__len > size())
            _new_elements_at_back(__len - this->_impl._finish +
                                  this->_impl._start);
        else
            _erase_at_end(this->_impl._start + __len);
        _destroy_range(this->_impl._start, this->_impl._finish,
                       _get_Ty_allocator());
        _copy_construct_range(__f, __l, this->_impl._start,
                              _get_Ty_allocator());
    }

    void _reserve_map_at_front(size_type __nodes_to_add = 1) {
        if (__nodes_to_add >
            size_type(this->_impl._start._node - this->_impl._map))
            _reallocate_map(__nodes_to_add, true);
    }

    void _reserve_map_at_back(size_type __nodes_to_add = 1) {
        if (__nodes_to_add + 1 >
            this->_impl._map_size -
                (this->_impl._finish._node - this->_impl._map))
            _reallocate_map(__nodes_to_add, false);
    }

    void _new_elements_at_front(size_type __n) {
        if (max_size() - size() < __n) throw(exceptions::invalid_init_size());

        const size_type __new_nodes =
            ((__n + _buffer_size() - 1) / _buffer_size());
        _reserve_map_at_front(__new_nodes);

        size_type __i;
        try {
            for (__i = 1; __i <= __new_nodes; ++__i)
                *(this->_impl._start._node - __i) = this->_allocate_node();
        } catch (...) {
            for (size_type __r = 1; __r < __i; ++__r)
                _deallocate_node(*(this->_impl._start._node - __r));
            throw;
        }
    }

    void _new_elements_at_back(size_type __n) {
        if (max_size() - size() < __n) throw(typename exceptions::invalid_init_size());

        const size_type __new_nodes =
            ((__n + _buffer_size() - 1) / _buffer_size());
        _reserve_map_at_back(__new_nodes);

        size_type __i;
        try {
            for (__i = 1; __i <= __new_nodes; ++__i)
                *(this->_impl._finish._node + __i) = this->_allocate_node();
        } catch (...) {
            for (size_type __r = 1; __r < __i; ++__r)
                _deallocate_node(*(this->_impl._finish._node + __r));
            throw;
        }
    }

    iterator _reserve_elements_at_front(size_type __n) {
        const size_type __vacancies =
            this->_impl._start._cur - this->_impl._start._first;

        if (__n > __vacancies) _new_elements_at_front(__n - __vacancies);
        return this->_impl._start - difference_type(__n);
    }

    iterator _reserve_elements_at_back(size_type __n) {
        const size_type __vacancies =
            (this->_impl._finish._last - this->_impl._finish._cur) - 1;
        if (__n > __vacancies) _new_elements_at_back(__n - __vacancies);
        return this->_impl._finish + difference_type(__n);
    }

    void _default_append(size_type __n) {
        if (__n) {
            iterator __new_finish = _reserve_elements_at_back(__n);
            try {
                _default_construct_range(this->_impl._finish, __new_finish,
                                         _get_Ty_allocator());
                this->_impl._finish = __new_finish;
            } catch (...) {
                _destroy_nodes(this->_impl._finish._node + 1,
                               __new_finish._node + 1);
                throw;
            }
        }
    }

    void _requires_nonempty() { assert(!this->empty()); }

    void _erase_at_end(iterator __pos) {
        _destroy_range(__pos, end(), _get_Ty_allocator());
        _destroy_nodes(__pos._node + 1, this->_impl._finish + 1);
        this->_impl._finish = __pos;
    }

    void _erase_at_begin(iterator __pos) {
        _destroy_range(begin(), __pos, _get_Ty_allocator());
        _destroy_nodes(this->_impl._start._node, __pos._node);
        this->_impl._start = __pos;
    }

    iterator _erase(iterator __pos) {
        iterator __next               = std::next(__pos);
        const difference_type __index = __pos - begin();
        if (static_cast<size_type>(__index) < (size() >> 1)) {
            if (__pos != begin()) std::move_backward(begin(), __pos, __next);
            pop_front();
        } else {
            if (__next != end()) std::move(__next, end(), __pos);
            pop_back();
        }
        return begin() + __index;
    }

    iterator _erase(iterator __first, iterator __last) {
        if (__first == __last) {
            return __first;
        } else if (__first == begin() && __last == end()) {
            clear();
            return end();
        } else {
            const difference_type __n         = __last - __first;
            const difference_type __el_before = __first - begin();

            if (static_cast<size_type>(__el_before) <= (size() - __n) / 2) {
                if (__first != begin())
                    std::move_backward(begin(), __first, __last);
                _erase_at_begin(begin() + __n);
            } else {
                if (__last != end()) std::move(__last, end(), __first);
                _erase_at_end(end() - __n);
            }
            return begin() + __el_before;
        }
    }

    void _fill_insert(iterator __pos, size_type __n, const value_type &__x) {
        if (__pos._cur == this->_impl._start._cur) {
            iterator __new_start = _reserve_elements_at_front(__n);

            try {
                std::uninitialized_fill(__new_start, this->_impl._start, __x,
                                        _get_Ty_allocator());
                this->_impl._start = __new_start;
            } catch (...) {
                _destroy_nodes(__new_start._node, this->_impl._start._node);
                throw;
            }
        } else if (__pos._cur == this->_impl._finish._cur) {
            iterator __new_finish = _reserve_elements_at_back(__n);

            try {
                std::uninitialized_fill(this->_impl._finish, __new_finish, __x,
                                        _get_Ty_allocator());
                this->_impl._finish = __new_finish;
            } catch (...) {
                _destroy_nodes(this->_impl._finish._node + 1,
                               __new_finish._node + 1);
                throw;
            }
        } else {
            _insert(__pos, __n, __x);
        }
    }

    template <typename... _Args>
    void _insert(iterator __pos, _Args &&...__args) {
        const difference_type __el_before = __pos - this->_impl._start;
        const size_type __len             = this->size();

        if (__el_before < difference_type(__len / 2)) {
            iterator __new_start = _reserve_elements_at_front(1);
            iterator __old_start = this->_impl._start;

            __pos = this->_impl._start + __el_before;
            try {
                if (__el_before) {
                    iterator __start_n = (this->_impl._start + 1);
                    _alloc_traits::construct(this->_impl, __new_start,
                                             std::move(__old_start));

                    std::move(__start_n, __pos, __old_start);
                }

                this->_impl._start = __new_start;
                _alloc_traits::construct(this->_impl, __pos - 1,
                                         std::forward(__args...));
            } catch (...) {
                _destroy_nodes(__new_start._node, this->_impl._start._node);
                throw;
            }
        } else {
            iterator __new_finish = _reserve_elements_at_back(1);
            iterator __old_finish = this->_impl._finish;
            const difference_type __el_after =
                difference_type(__len) - __el_before;

            __pos = this->_impl._finish - __el_after;
            try {
                if (__el_after) {
                    iterator __finish_n = (this->_impl._finish - 1);

                    _alloc_traits::construct(
                        this->_impl, this->_impl._finish,
                        std::move(this->_impl._finish - 1));

                    std::move_backward(__pos, __finish_n, __old_finish);
                }
                this->_impl._finish = __new_finish;
                _alloc_traits::construct(this->_impl, __pos,
                                         std::forward(__args...));
            } catch (...) {
                _destroy_nodes(this->_impl._finish._node + 1,
                               __new_finish._node + 1);
                throw;
            }
        }
    }

    void _insert(iterator __pos, size_type __n, const value_type &__x) {
        const difference_type __el_before = __pos - this->_impl._start;
        const size_type __len             = this->size();

        if (__el_before < difference_type(__len / 2)) {
            iterator __new_start = _reserve_elements_at_front(__n);
            iterator __old_start = this->_impl._start;

            __pos = this->_impl._start + __el_before;
            try {
                if (__el_before >= difference_type(__n)) {
                    iterator __start_n =
                        (this->_impl._start + difference_type(__n));
                    std::uninitialized_move(this->_impl._start, __start_n,
                                            __new_start, _get_Ty_allocator());

                    this->_impl._start = __new_start;
                    std::move(__start_n, __pos, __old_start);
                    std::fill(__pos - difference_type(__n), __pos, __x);
                } else {
                    std::uninitialized_move(this->_impl._start, __pos,
                                            __new_start, _get_Ty_allocator());
                    std::uninitialized_fill(__new_start + __el_before,
                                            __old_start, __x,
                                            _get_Ty_allocator());

                    this->_impl._start = __new_start;

                    std::fill(__old_start, __pos, __x);
                }
            } catch (...) {
                _destroy_nodes(__new_start._node, this->_impl._start._node);
                throw;
            }
        } else {
            iterator __new_finish = _reserve_elements_at_back(__n);
            iterator __old_finish = this->_impl._finish;
            const difference_type __el_after =
                difference_type(__len) - __el_before;

            __pos = this->_impl._finish - __el_after;
            try {
                if (__el_after > difference_type(__n)) {
                    iterator __finish_n =
                        (this->_impl._finish - difference_type(__n));

                    std::uninitialized_move(__finish_n, this->_impl._finish,
                                            this->_impl._finish,
                                            _get_Ty_allocator());

                    this->_impl._finish = __new_finish;
                    std::move_backward(__pos, __finish_n, __old_finish);
                    std::fill(__pos, __pos + difference_type(__n), __x);
                } else {
                    std::uninitialized_move(__pos, __old_finish,
                                            __pos + difference_type(__n),
                                            _get_Ty_allocator());
                    std::uninitialized_fill(__old_finish + 1,
                                            __pos + difference_type(__n), __x,
                                            _get_Ty_allocator());

                    this->_impl._finish = __new_finish;
                    std::fill(__pos, __old_finish, __x);
                }
            } catch (...) {
                _destroy_nodes(this->_impl._finish._node + 1,
                               __new_finish._node + 1);
                throw;
            }
        }
    }

    template <typename _ForwardIter>
    void _insert(iterator __pos, _ForwardIter __first, _ForwardIter __last) {
        const difference_type __el_count  = std::distance(__first, __last);
        const difference_type __el_before = __pos - this->_impl._start;
        const size_type __len             = this->size();

        if (__el_before < difference_type(__len / 2)) {
            iterator __new_start = _reserve_elements_at_front(__el_count);
            iterator __old_start = this->_impl._start;

            __pos = this->_impl._start + __el_before;
            try {
                if (__el_before >= difference_type(__el_count)) {
                    iterator __start_n =
                        (this->_impl._start + difference_type(__el_count));
                    std::uninitialized_move(this->_impl._start, __start_n,
                                            __new_start, _get_Ty_allocator());

                    this->_impl._start = __new_start;
                    std::move(__start_n, __pos, __old_start);
                    std::copy(__first, __last,
                              __pos - difference_type(__el_count));
                } else {
                    difference_type __uninit_copy = __el_count - __el_before;

                    std::uninitialized_move(this->_impl._start, __pos,
                                            __new_start, _get_Ty_allocator());
                    std::uninitialized_copy_n(__new_start + __el_before,
                                              __uninit_copy,
                                              _get_Ty_allocator());

                    this->_impl._start = __new_start;

                    std::copy(std::advance(__first, __uninit_copy), __last,
                              __old_start);
                }
            } catch (...) {
                _destroy_nodes(__new_start._node, this->_impl._start._node);
                throw;
            }
        } else {
            iterator __new_finish = _reserve_elements_at_back(__el_count);
            iterator __old_finish = this->_impl._finish;
            const difference_type __el_after =
                difference_type(__len) - __el_before;

            __pos = this->_impl._finish - __el_after;
            try {
                if (__el_after > difference_type(__el_count)) {
                    iterator __finish_n =
                        (this->_impl._finish - difference_type(__el_count));

                    std::uninitialized_move(__finish_n, this->_impl._finish,
                                            this->_impl._finish,
                                            _get_Ty_allocator());

                    this->_impl._finish = __new_finish;
                    std::move_backward(__pos, __finish_n, __old_finish);
                    std::copy(__first, __last, __pos);
                } else {
                    difference_type __uninit_copy = __el_count - __el_after;
                    std::uninitialized_move(__pos, __old_finish,
                                            __pos + difference_type(__el_count),
                                            _get_Ty_allocator());
                    // TODO
                    std::uninitialized_copy_n(__first,
                                              __el_count - __uninit_copy,
                                              _get_Ty_allocator());

                    this->_impl._finish = __new_finish;
                }
            } catch (...) {
                _destroy_nodes(this->_impl._finish._node + 1,
                               __new_finish._node + 1);
                throw;
            }
        }
    }

    void _fill_assign(size_type __n, const value_type &__v) {
        if (__n > size())
            _new_elements_at_back(__n - this->_impl._finish +
                                  this->_impl._start);
        else
            _erase_at_end(this->_impl._start + __n);
        std::fill(this->_impl._start, this->_impl._finish, __v);
    }

    void _fill_append(size_type __n, const value_type &__v) {
        iterator __end = this->_impl._finish;
        _new_elements_at_back(__n);
        std::uninitialized_fill(__end, this->_impl._finish, __v,
                                _get_Ty_allocator());
    }

    void _destroy_data() {
        if (this->_impl._map) {
            _destroy_range(this->_impl._start, this->_impl._finish,
                           _get_Ty_allocator());
            _destroy_nodes(this->_impl._start._node, this->_impl._finish._node);
            _deallocate_map(this->_impl._map, this->_impl._map_size);
            this->_impl._map_size = 0;
        }
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
