#ifndef _TMB_VALUES_
#define _TMB_VALUES_

#ifndef TMB_NOEXCEPT
#if __cplusplus >= 201103L
   #define TMB_NOEXCEPT noexcept
   #define TMB_NOEXCEPT_IF(_COND) noexcept(_COND)
   #define TMB_USE_NOEXCEPT noexcept
   #define TMB_THROW(_EXC)
#else
   #define TMB_NOEXCEPT
   #define TMB_NOEXCEPT_IF(_COND)
   #define TMB_USE_NOEXCEPT throw()
   #define TMB_THROW(_EXC) throw(_EXC)
#endif
#endif // TMB_NOEXCEPT

#ifndef TMB_NODISCARD
#if __cplusplus >= 201700L
   #define TMB_NODISCARD [[nodiscard]]
#elif __has_cpp_attribute(gnu::warn_unused_result)
   #define TMB_NODISCARD [[gnu::warn_unused_result]]
#else
   #define TMB_NODISCARD
#endif
#endif // TMB_NODISCARD

#ifndef TMB_CONSTEXPR
#if __cplusplus > 201703L
   #define TMB_CONSTEXPR constexpr
#else
   #define TMB_CONSTEXPR
#endif
#endif

#endif