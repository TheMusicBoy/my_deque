#ifndef _TMB_ALLOCATOR_
#define _TMB_ALLOCATOR_

#include <memory>
#include <type_traits>
#include <iostream>

#include "values.hpp"

namespace tmb {

template <typename _Ty>
class allocator {
 public:
   using size_type         = std::size_t;
   using difference_type   = std::ptrdiff_t;

   using value_type        = _Ty;
   using pointer           = _Ty*;
   using const_pointer     = const _Ty*;
   using reference         = _Ty&;
   using const_reference   = const _Ty&;


#if __cplusplus >= 201100L
   using is_always_equal = std::true_type;
#endif


   template<typename _Ty1>
   struct rebind { allocator<_Ty1> other; };
   
   TMB_CONSTEXPR allocator() TMB_NOEXCEPT  { }

   template<typename _Ty1>
   TMB_CONSTEXPR allocator(const allocator<_Ty1>& __val) TMB_NOEXCEPT {};

   template<typename _Ty1>
   allocator& operator=(const allocator<_Ty1>&) TMB_NOEXCEPT {};

   ~allocator() TMB_NOEXCEPT {}


   TMB_NODISCARD pointer allocate(size_type __n) {
      if(__n > max_size())
         throw("invalid allocation");

      std::cout << "Allocated " << __n << " elements" << std::endl;

      return static_cast<_Ty*>(::operator new(__n * sizeof(_Ty)));
   }

   void deallocate(pointer __ptr, size_type) TMB_NOEXCEPT {
      ::operator delete(__ptr);
   }

   size_type max_size() const {
#if __PTRDIFF_MAX__ < __SIZE_MAX__
	return size_t(__PTRDIFF_MAX__) / sizeof(_Ty);
#else
	return size_t(-1) / size_t(_Ty);
#endif
   } // class allocator

};

}  // namespace tmb

#endif