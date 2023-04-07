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

}  // namespace tmb

#endif
