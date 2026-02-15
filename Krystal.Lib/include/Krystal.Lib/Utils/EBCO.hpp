#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include <memory>
#include <utility>

namespace Krys
{
  /// @brief A class for optimizing the amount of space a certain member of type `T` might use.
  /// @tparam T The type of the member.
  /// @tparam TTag A differentiating tag to separate this member from others when there are multiple bases of
  /// the same `T`.
  /// @remarks The only reason this class continues to be necessary is because of Microsoft Visual C++. Every
  /// other compiler respects the new C++20 attribute [[no_unique_address]] - it is only Microsoft that
  /// explicitly decided that our opt-in indication that we care more about the object's size is not
  /// important.
  template <typename T, std::size_t TTag = 0, typename = void>
  class alignas(T) EBCO
  {
  private:
    T _value;

  public:
    /// @brief Default construction.
    EBCO() = default;

    /// @brief Copy construction.
    EBCO(const EBCO &) = default;

    /// @brief Move construction.
    EBCO(EBCO &&) = default;

    /// @brief Copy assignment operator.
    EBCO &operator=(const EBCO &) = default;

    /// @brief Move assignment operator.
    EBCO &operator=(EBCO &&) = default;

    /// @brief Copies the object into storage.
    constexpr EBCO(const T &value) noexcept(NoThrowCopyConstructible<T>) : _value(value) {};

    /// @brief Moves the object into storage.
    constexpr EBCO(T &&value) noexcept(NoThrowMoveConstructible<T>) : _value(std::move(value)) {};

    /// @brief Copy assigns into the previous object into storage.
    constexpr EBCO &operator=(const T &value) noexcept(NoThrowCopyAssignable<T>)
    {
      this->_value = value;
      return *this;
    }

    /// @brief Move assigns into the previous object into storage.
    constexpr EBCO &operator=(T &&value) noexcept(NoThrowMoveAssignable<T>)
    {
      this->_value = std::move(value);
      return *this;
    };

    /// @brief Constructs the object in storage from the given arguments.
    template <typename TArg, typename... TArgs>
    requires(!SameType<remove_ref_t<remove_cv_t<TArg>>, EBCO>
             && !SameType<remove_ref_t<remove_cv_t<TArg>>, T>)
    constexpr EBCO(TArg &&arg, TArgs &&...args) noexcept(NoThrowConstructible<T, TArg, TArgs...>)
        : _value(std::forward<TArg>(arg), std::forward<TArgs>(args)...)
    {
    }

    /// @brief Gets the wrapped value.
    constexpr T &GetValue() & noexcept
    {
      return static_cast<T &>(this->_value);
    }

    /// @brief Gets the wrapped value.
    constexpr T const &GetValue() const & noexcept
    {
      return static_cast<T const &>(this->_value);
    }

    /// @brief Gets the wrapped value.
    constexpr T &&GetValue() && noexcept
    {
      return static_cast<T &&>(this->_value);
    }
  };

  /// @brief A partial template specialization for types which can be stored as a base class, enabling more of
  /// the optimization potential.
  template <typename T, std::size_t TTag>
  requires(Class<T> && !IsFinal<T> && !Reference<T>)
  class alignas(T) EBCO<T, TTag> : private T
  {
  public:
    /// @brief Default construction.
    EBCO() = default;

    /// @brief Copy construction.
    EBCO(const EBCO &) = default;

    /// @brief Move construction.
    EBCO(EBCO &&) = default;

    /// @brief Copy constructs the object in storage.
    constexpr EBCO(const T &value) noexcept(std::is_nothrow_copy_constructible_v<T>) : T(value) {};

    /// @brief Move constructs the object in storage.
    constexpr EBCO(T &&value) noexcept(std::is_nothrow_move_constructible_v<T>) : T(std::move(value)) {};

    /// @brief Constructs the object in storage from the given arguments.
    template <typename TArg, typename... TArgs>
    requires(!SameType<remove_ref_t<remove_cv_t<TArg>>, EBCO>
             && !SameType<remove_ref_t<remove_cv_t<TArg>>, T>)
    constexpr EBCO(TArg &&arg, TArgs &&...args) noexcept(NoThrowConstructible<T, TArg, TArgs...>)
        : T(std::forward<TArg>(arg), std::forward<TArgs>(args)...)
    {
    }

    /// @brief Copy assignment operator.
    EBCO &operator=(const EBCO &) = default;

    /// @brief Move assignment operator.
    EBCO &operator=(EBCO &&) = default;

    /// @brief Copy assigns into the previous object into storage.
    constexpr EBCO &operator=(const T &value) noexcept(NoThrowCopyAssignable<T>)
    {
      static_cast<T &>(*this) = value;
      return *this;
    }

    /// @brief Move assigns into the previous object into storage.
    constexpr EBCO &operator=(T &&value) noexcept(NoThrowMoveAssignable<T>)
    {
      static_cast<T &>(*this) = std::move(value);
      return *this;
    }

    /// @brief Gets the wrapped value.
    constexpr T &&GetValue() && noexcept
    {
      return static_cast<T &&>(*this);
    }

    /// @brief Gets the wrapped value.
    constexpr T &GetValue() & noexcept
    {
      return static_cast<T &>(*this);
    }

    /// @brief Gets the wrapped value.
    constexpr T const &GetValue() const & noexcept
    {
      return static_cast<T const &>(*this);
    }
  };

  /// @brief A partial specialization for l-value reference types.
  template <typename T, std::size_t TTag>
  class alignas(T *) EBCO<T &, TTag>
  {
  private:
    T *_value;

  public:
    /// @brief Default construction.
    EBCO() = default;

    /// @brief Default construction.
    EBCO(const EBCO &) = default;

    /// @brief Default construction.
    EBCO(EBCO &&) = default;

    /// @brief Copy assignment.
    constexpr EBCO &operator=(const EBCO &value) noexcept
    {
      *(this->_value) = *(value._value);
      return *this;
    }

    /// @brief Move assignment.
    constexpr EBCO &operator=(EBCO &&value) noexcept
    {
      *(this->_value) = std::move(*(value._value));
      return *this;
    }

    /// @brief Holds onto @p value's reference
    constexpr EBCO(T &value) noexcept : _value(std::addressof(value)) {};

    /// @brief Assigns into the underlying stored reference.
    constexpr EBCO &operator=(T &value) noexcept
    {
      *(this->_value) = value;
      return *this;
    }

    /// @brief Gets the wrapped value.
    constexpr T &GetValue() & noexcept
    {
      return *(this->_value);
    }

    /// @brief Gets the wrapped value.
    constexpr T const &GetValue() const & noexcept
    {
      return *(this->_value);
    }

    /// @brief Gets the wrapped value.
    constexpr T &&GetValue() && noexcept
    {
      return std::move(*(this->_value));
    }
  };
}