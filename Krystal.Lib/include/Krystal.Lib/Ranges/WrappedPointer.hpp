#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Utils/ToAddress.hpp"
#include "Krystal.Lib/Utils/Unwrap.hpp"
#include <iterator>
#include <utility>

namespace Krys::Ranges
{
  template <typename T>
  class WrappedPointer
  {
  private:
    template <typename>
    friend class ::Krys::Ranges::WrappedPointer;

    using TUnwrapped = decltype(::Krys::Unwrap(std::declval<T &>()));

  public:
    using iterator_category = std::contiguous_iterator_tag;
    using value_type = remove_cvref_t<TUnwrapped>;
    using element_type = remove_ref_t<TUnwrapped>;
    using reference = add_lvalue_ref_t<TUnwrapped>;
    using pointer = add_pointer_t<element_type>;
    using difference_type = decltype(std::declval<const pointer &>() - std::declval<const pointer &>());

    constexpr WrappedPointer() : WrappedPointer(nullptr)
    {
    }
    constexpr WrappedPointer(pointer ptr) : _ptr(ptr)
    {
    }
    template <typename TRight, std::enable_if_t<!SameType<WrappedPointer<T>, WrappedPointer<TRight>>
                                                  && Const<T> && !Const<TRight>,
                                                std::nullptr_t> = nullptr>
    constexpr WrappedPointer(const WrappedPointer<TRight> &right) noexcept : _ptr(right._ptr)
    {
    }
    template <typename TRight, std::enable_if_t<Const<T> && !Const<TRight>, std::nullptr_t> = nullptr>
    constexpr WrappedPointer(WrappedPointer<TRight> &&right) noexcept : _ptr(std::move(right._ptr))
    {
    }
    constexpr WrappedPointer(const WrappedPointer &) = default;
    constexpr WrappedPointer(WrappedPointer &&) = default;
    constexpr WrappedPointer &operator=(const WrappedPointer &) = default;
    constexpr WrappedPointer &operator=(WrappedPointer &&) = default;

    constexpr explicit operator bool() const noexcept
    {
      return this->_ptr != nullptr;
    }

    constexpr pointer base() const noexcept
    {
      return this->_ptr;
    }

    constexpr reference operator[](difference_type index) const noexcept
    {
      pointer ptr = this->_ptr + index;
      return Krys::Unwrap(*ptr);
    }

    constexpr reference operator*() const noexcept
    {
      return Krys::Unwrap(*this->_ptr);
    }

    constexpr pointer operator->() const noexcept
    {
      return std::addressof(Krys::Unwrap(*this->_ptr));
    }

    constexpr WrappedPointer &operator++() noexcept
    {
      ++(this->_ptr);
      return *this;
    }

    constexpr WrappedPointer operator++(int) noexcept
    {
      auto copy = *this;
      ++copy;
      return copy;
    }

    constexpr WrappedPointer &operator--() noexcept
    {
      --(this->_ptr);
      return *this;
    }

    constexpr WrappedPointer operator--(int) noexcept
    {
      auto copy = *this;
      --copy;
      return copy;
    }

    constexpr WrappedPointer &operator+=(difference_type right) noexcept
    {
      this->_ptr += right;
      return *this;
    }

    constexpr WrappedPointer &operator-=(difference_type right) noexcept
    {
      this->_ptr -= right;
      return *this;
    }

    constexpr WrappedPointer operator+(difference_type right) const noexcept
    {
      return WrappedPointer(this->_ptr + right);
    }

    constexpr WrappedPointer operator-(difference_type right) const noexcept
    {
      return WrappedPointer(this->_ptr - right);
    }

    friend constexpr pointer to_address(const WrappedPointer &wrapped) noexcept
    {
      return Krys::UnwrapIteratorValue(wrapped._ptr);
    }

  private:
    pointer _ptr;
  };

  template <typename _LeftType, typename TRight>
  constexpr bool operator==(const WrappedPointer<_LeftType> &left, const WrappedPointer<TRight> &right)
  {
    return left.base() == right.base();
  }

  template <typename TRight>
  constexpr bool operator==(std::nullptr_t left, const WrappedPointer<TRight> &right)
  {
    return left == right.base();
  }

  template <typename _LeftType>
  constexpr bool operator==(const WrappedPointer<_LeftType> &left, std::nullptr_t right)
  {
    return left.base() == right;
  }

  template <typename _LeftType, typename TRight>
  constexpr bool operator!=(const WrappedPointer<_LeftType> &left, const WrappedPointer<TRight> &right)
  {
    return left.base() != right.base();
  }

  template <typename TRight>
  constexpr bool operator!=(std::nullptr_t left, const WrappedPointer<TRight> &right)
  {
    return left != right.base();
  }

  template <typename _LeftType>
  constexpr bool operator!=(const WrappedPointer<_LeftType> &left, std::nullptr_t right)
  {
    return left.base() != right;
  }

  template <typename _LeftType, typename TRight>
  constexpr typename WrappedPointer<_LeftType>::difference_type
    operator-(const WrappedPointer<_LeftType> &left, const WrappedPointer<TRight> &right) noexcept
  {
    return left.base() - right.base();
  }

  template <typename T>
  constexpr auto ToMutableIter(const WrappedPointer<const T> &value) noexcept
  {
    using TPtr = typename WrappedPointer<T>::pointer;
    return WrappedPointer<T>(const_cast<TPtr>(value.base()));
  }

  template <typename T>
  constexpr auto ToMutableIter(WrappedPointer<const T> &value) noexcept
  {
    using TPtr = typename WrappedPointer<T>::pointer;
    return WrappedPointer<T>(const_cast<TPtr>(value.base()));
  }

}

namespace std
{
  template <typename T>
  struct pointer_traits<Krys::Ranges::WrappedPointer<T>>
  {
    using pointer = typename Krys::Ranges::WrappedPointer<T>::pointer;
    using element_type = typename Krys::Ranges::WrappedPointer<T>::element_type;
    using difference_type = typename Krys::Ranges::WrappedPointer<T>::difference_type;

    static constexpr pointer pointer_to(element_type &iter) noexcept
    {
      return std::addressof(iter);
    }

    static constexpr pointer to_address(const Krys::Ranges::WrappedPointer<T> &iter) noexcept(
      noexcept(Krys::to_address(iter.base())))
    {
      return Krys::to_address(iter.base());
    }
  };
}
