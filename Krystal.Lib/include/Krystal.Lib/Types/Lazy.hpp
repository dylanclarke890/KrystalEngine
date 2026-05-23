#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include <cassert>

namespace Krys
{
  template <typename T>
  struct DefaultLazyCreate
  {
    KRYS_NODISCARD T operator()() const noexcept(NoThrowDefaultConstructible<T>)
    {
      return T {};
    }
  };

  template <typename T>
  struct DefaultLazyUniqueCreate
  {
    KRYS_NODISCARD UniquePtr<T> operator()() const noexcept(NoThrowDefaultConstructible<T>)
    {
      return CreateUnique<T>();
    }
  };

  /// @brief Thin wrapper class that allows for lazy initialization.
  /// @tparam T Any type. Usually one that has an expensive constructor that doesn't need to be invoked
  /// immediately.
  /// @tparam CreateFn A callable type that returns an instance of T.
  template <typename T, typename CreateFn = DefaultLazyCreate<T>>
  class LazyValue
  {
  private:
    mutable Maybe<T> _value;

    constexpr static inline bool IsNoThrow =
      noexcept(std::declval<CreateFn>()()) && NoThrowMoveConstructible<T>;

  public:
    KRYS_NODISCARD const T &get() const noexcept(IsNoThrow)
    {
      EnsureCreated();
      return *_value;
    }

    KRYS_NODISCARD T &get() noexcept(IsNoThrow)
    {
      EnsureCreated();
      return *_value;
    }

    KRYS_NODISCARD const T &operator*() const noexcept(IsNoThrow)
    {
      EnsureCreated();
      return *_value;
    }

    KRYS_NODISCARD T &operator*() noexcept(IsNoThrow)
    {
      EnsureCreated();
      return *_value;
    }

    KRYS_NODISCARD bool HasValue() const noexcept
    {
      return _value.has_value();
    }

    void Reset() noexcept(NoThrowDestructible<T>)
    {
      _value.reset();
    }

  private:
    void EnsureCreated() const noexcept(IsNoThrow)
    {
      if (!_value.has_value())
      {
        _value = CreateFn {}();
      }
    }
  };

  /// @brief Thin wrapper class that allows for lazy allocation of an object.
  /// @tparam T Any type. Usually one that is large/rarely used and thus should be allocated on the heap.
  /// @tparam CreateFn A callable type that returns an instance of UniquePtr<T>.
  /// @tparam TDeleter The deleter type for the UniquePtr.
  template <typename T, typename CreateFn = DefaultLazyUniqueCreate<T>,
            typename TDeleter = std::default_delete<T>>
  class LazyUnique
  {
  protected:
    mutable UniquePtr<T, TDeleter> _value;
    constexpr static inline bool IsNoThrow = noexcept(std::declval<CreateFn>()());

  public:
    KRYS_NODISCARD const T &get() const noexcept(IsNoThrow)
    {
      EnsureCreated();
      return *_value;
    }

    KRYS_NODISCARD T &get() noexcept(IsNoThrow)
    {
      EnsureCreated();
      return *_value;
    }

    KRYS_NODISCARD const T &operator*() const noexcept(IsNoThrow)
    {
      EnsureCreated();
      return *_value;
    }

    KRYS_NODISCARD T &operator*() noexcept(IsNoThrow)
    {
      EnsureCreated();
      return *_value;
    }

    KRYS_NODISCARD T *operator->() noexcept(IsNoThrow)
    {
      EnsureCreated();
      return _value.get();
    }

    KRYS_NODISCARD const T *operator->() const noexcept(IsNoThrow)
    {
      EnsureCreated();
      return _value.get();
    }

    KRYS_NODISCARD bool HasValue() const noexcept
    {
      return _value != nullptr;
    }

    void Reset() noexcept(NoThrowDestructible<T>)
    {
      _value.reset();
    }

  private:
    void EnsureCreated() const noexcept(IsNoThrow)
    {
      if (_value == nullptr)
      {
        _value = CreateFn {}();
      }
    }
  };
}
