#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Config.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Detection/Compiler.hpp"
#include "Krystal.Lib/Ranges/Impl/ContiguousIterator.hpp"
#include <xutility>

namespace Krys
{
  namespace Impl
  {
    template <typename T, typename = void>
    struct ArrowOperator
    {
    };

    template <typename T>
    struct ArrowOperator<T, void_t<decltype(std::declval<T &>().operator->())>>
    {
      using type = typename ArrowOperator<decltype(std::declval<T &>().operator->())>::type;
    };

    template <typename T>
    struct ExtractFirstParameter
    {
    };

    template <template <class, class...> typename T, typename TFirst, typename... TArgs>
    struct ExtractFirstParameter<T<TFirst, TArgs...>>
    {
      using type = TFirst;
    };

    template <typename T, typename = void>
    struct ExtractElementType : public ExtractFirstParameter<T>
    {
    };

    template <typename T>
    struct ExtractElementType<T, void_t<typename remove_cvref_t<T>::element_type>>
    {
      using type = typename remove_cvref_t<T>::element_type;
    };

    template <typename T>
    concept IsMaybeStdPointerTraitable = requires { typename ExtractElementType<remove_cvref_t<T>>::type; };

    template <typename T>
    concept HasStdPointerTraitsToAddress =
      requires(T &t) { std::pointer_traits<remove_ref_t<T>>::to_address(t); };
  }

  /// @brief The type of the operator arrow function.
  /// @tparam T  The pointer or iterator to attempt to operator arrow type within.
  template <typename T>
  using operator_arrow_t = typename Impl::ArrowOperator<T>::type;

  template <typename T>
  concept OperatorArrowable = requires(T t) {
    { t.operator->() };
  };

  /// @brief Whether or not the given type can have to_address (std::pointer_traits<Type>::to_address)
  /// called on it.
  template <typename T>
  concept ToAddressable = (!Impl::IsMaybeStdPointerTraitable<T>
                           && ((IsPointer<T> && !IsFunction<remove_ref_t<remove_pointer_t<T>>>)
                               || OperatorArrowable<remove_ref_t<T>>))
                          || (Impl::IsMaybeStdPointerTraitable<T>
                              && (Impl::HasStdPointerTraitsToAddress<T>
                                  || (!IsFunction<remove_ref_t<T>> && OperatorArrowable<remove_ref_t<T>>)));

  namespace Impl
  {
    struct ToAddressFn
    {
#if KRYS_CONFIG(STD_LIBRARY_TO_ADDRESS)
      template <typename T>
      constexpr auto operator()(T &&ptrLike) const
        noexcept(noexcept(std::to_address(std::forward<T>(ptrLike))))
          -> decltype(std::to_address(std::forward<T>(ptrLike)))
      {
        return std::to_address(std::forward<T>(ptrLike));
      }
#else

      /// @brief Identity: returns the pointer value that was passed in.
      template <typename T>
      constexpr T *operator()(T *ptr) const noexcept
      {
        static_assert(!IsFunction<T>, "the pointer shall not be function pointer type");
        return ptr;
      }

      /// @brief Calls to_address if it's available, or falls back to other means for pointers and other
      /// potentially-contiguous types.
      template <typename TPointer>
      requires(!IsPointer<TPointer>)
      constexpr auto operator()(TPointer &p) const noexcept
      {
        if constexpr (Krys::Ranges::IsContiguousIterator<TPointer>)
        {
  #if KRYS_COMPILER_STL(MSVC)
          return (*this)(p._Unwrapped());
  #else
          return (*this)(p.operator->());
  #endif
        }
        else
        {
          if constexpr (Impl::HasStdPointerTraitsToAddress<TPointer>)
          {
            return std::pointer_traits<TPointer>::to_address(p);
          }
          else
          {
            return (*this)(p.operator->());
          }
        }
      }
#endif
    };
  }

  /// @brief Calls to_address if it's available, or falls back to other means for pointers and other
  /// potentially-contiguous types. This is an identity function for pointer types.
  /// @returns A pointer type representing the pointer or iterator passed in, if at all possible.
  inline constexpr Impl::ToAddressFn to_address = {};
}
