#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Hash.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/Concepts.hpp"
#include <memory>

namespace Krys
{
  /// @brief Owns the memory it points to.
  /// @tparam T The underlying type of the data being pointed to.
  template <typename T, typename Deleter = std::default_delete<T>>
  using UniquePtr = std::unique_ptr<T, Deleter>;

  template <typename T, typename... Args>
  requires(Constructible<T, Args...>)
  KRYS_NODISCARD constexpr UniquePtr<T>
    CreateUnique(Args &&...args) noexcept(NoThrowConstructible<T, Args...>)
  {
    return std::make_unique<T>(std::forward<Args>(args)...);
  }

  template <typename TExpected, typename TArg, typename Deleter>
  KRYS_NODISCARD constexpr inline bool Is(const UniquePtr<TArg, Deleter> &source) noexcept
  {
    return Is<TExpected>(source.get());
  }

  template <typename T>
  constexpr bool IsSmartPtr<UniquePtr<T>> = true;

  template <typename T>
  constexpr bool IsNullableSmartPtr<UniquePtr<T>> = true;

  template <typename P, typename Deleter>
  struct DefaultHash<UniquePtr<P, Deleter>> : public PtrHash<UniquePtr<P, Deleter>>
  {
  };
}