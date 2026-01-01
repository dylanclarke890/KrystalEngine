#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include <cstddef>
#include <iterator>
#include <tuple>

namespace Krys::Ranges
{
  namespace Impl
  {
    // A more aggressive form of std::ignore, this form includes a constructor which takes everything and does
    // nothing, making it truly the devourer of all things.
    struct Blackhole
    {
      constexpr Blackhole() = default;
      constexpr Blackhole(const Blackhole &) = default;
      constexpr Blackhole(Blackhole &&) = default;

      template <typename TRight>
      requires(!SameType<remove_cvref_t<TRight>, Blackhole>)
      constexpr Blackhole(TRight &&) noexcept
      {
      }

      constexpr Blackhole &operator=(const Blackhole &) = default;
      constexpr Blackhole &operator=(Blackhole &&) = default;

      template <typename TRight>
      requires(!SameType<remove_cvref_t<TRight>, Blackhole>)
      constexpr Blackhole &operator=(TRight &&) noexcept
      {
        return *this;
      }
    };
  }

  /// @brief An output iterator that does absolutely nothing and swallows up all output operations into the
  /// void.
  class BlackholeIterator
  {
  public:
    /// @brief The iterator category. All blackhole iterators are the weakest kind: output iterators.
    using iterator_category = std::output_iterator_tag;

    /// @brief Standard `std::ptrdiff_t` `difference_type` definition.
    using difference_type = std::ptrdiff_t;

    /// @brief Mostly-useless pointer definition. Points to implementation-defined black hole type.
    using pointer = Impl::Blackhole *;

    /// @brief An implementation-defined blackhole type that simply consumes everything and does nothing.
    using value_type = Impl::Blackhole;

    /// @brief An implementation-defined blackhole type that simply consumes everything and does nothing.
    using reference = Impl::Blackhole;

    /// @brief Does absolutely nothing.
    constexpr BlackholeIterator operator++(int) const
    {
      return *this;
    }

    /// @brief Does absolutely nothing.
    constexpr BlackholeIterator &operator+=(difference_type)
    {
      return *this;
    }

    /// @brief Does absolutely nothing.
    constexpr BlackholeIterator &operator-=(difference_type)
    {
      return *this;
    }

    /// @brief Does absolutely nothing.
    constexpr BlackholeIterator operator+(difference_type)
    {
      return *this;
    }

    /// @brief Does absolutely nothing.
    constexpr BlackholeIterator operator-(difference_type)
    {
      return *this;
    }

    /// @brief Does absolutely nothing.
    constexpr difference_type operator-(BlackholeIterator)
    {
      return 0;
    }

    /// @brief Does absolutely nothing.
    constexpr BlackholeIterator &operator++()
    {
      return *this;
    }

    /// @brief Does absolutely nothing.
    constexpr BlackholeIterator operator--(int) const
    {
      return *this;
    }

    /// @brief Does absolutely nothing.
    constexpr BlackholeIterator &operator--()
    {
      return *this;
    }

    /// @brief Returns the unspeakable blackhole reference type.
    constexpr reference operator*() const
    {
      return reference {};
    }
  };
}
