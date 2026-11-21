/*
 * Copyright (C) 2018 Yusuke Suzuki <yusukesuzuki@slowstart.org>.
 * Copyright (C) 2016-2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
// The idea of Markable<T> is derived from markable<T> at
// https://github.com/akrzemi1/markable.
//
// Copyright (C) 2015-2018 Andrzej Krzemienski.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Null.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include <cmath>
#include <limits>

namespace krys
{
#pragma region MarkableTraits

  namespace detail_markable
  {
    /// @example
    ///     enum class Type { Value1, Value2, Value3 };
    ///     Markable<Type, EnumMarkableTraits<Type, 42>> optional;
    template <typename EnumType,
              underlying_t<EnumType> Constant = std::numeric_limits<underlying_t<EnumType>>::max()>
    struct EnumMarkableTraits
    {
      static_assert(IsEnum<EnumType>, "EnumMarkableTraits can only be used with enum types");

      using UnderlyingType = underlying_t<EnumType>;

      KRYS_NODISCARD constexpr static bool IsEmptyValue(EnumType value) noexcept
      {
        return static_cast<UnderlyingType>(value) == Constant;
      }

      KRYS_NODISCARD constexpr static EnumType EmptyValue() noexcept
      {
        return static_cast<EnumType>(Constant);
      }
    };

    template <typename IntegralType, IntegralType Constant>
    struct IntegralMarkableTraits
    {
      static_assert(Integral<IntegralType>);

      KRYS_NODISCARD constexpr static bool IsEmptyValue(IntegralType value) noexcept
      {
        return value == Constant;
      }

      KRYS_NODISCARD constexpr static IntegralType EmptyValue() noexcept
      {
        return Constant;
      }
    };
  }

  template <typename T>
  struct MarkableTraits
  {
    static_assert(DependentFalse<T>, "MarkableTraits must be specialized for this type");
  };

  template <IsEnum T>
  struct MarkableTraits<T> : public detail_markable::EnumMarkableTraits<T>
  {
  };

  template <Integral T>
  struct MarkableTraits<T> : public detail_markable::IntegralMarkableTraits<T, static_cast<T>(-1)>
  {
  };

  template <typename T>
  requires(FloatingPoint<T>)
  struct MarkableTraits<T>
  {
    KRYS_NODISCARD constexpr static bool IsEmptyValue(T value) noexcept
    {
      return std::isnan(value);
    }

    KRYS_NODISCARD constexpr static T EmptyValue() noexcept
    {
      return std::numeric_limits<T>::quiet_NaN();
    }
  };

#pragma endregion

  /// @brief The goal of Markable is offering a Maybe-style type without sacrificing storage efficiency.
  /// Markable takes Traits, which should have IsEmptyValue and EmptyValue functions. By using
  /// one value of T as an empty value, we can remove the required flag in Maybe. This class is intended to be
  /// used as a member of a class to compact the size of the class. Otherwise, you should use Maybe.
  /// @note This strategy is similar to krys::HashTable, which uses two values of T as an empty value and a
  /// deleted value.
  template <typename T, typename Traits = MarkableTraits<T>>
  class Markable
  {
  private:
    T _value;

  public:
    constexpr Markable() noexcept : _value(Traits::EmptyValue())
    {
    }

    constexpr Markable(null_t) noexcept : Markable()
    {
    }

    constexpr Markable(T &&value) noexcept : _value(krys::move(value))
    {
    }

    constexpr Markable(const T &value) noexcept : _value(value)
    {
    }

    template <typename... Args>
    constexpr explicit Markable(std::in_place_t, Args &&...args) noexcept
        : _value(std::forward<Args>(args)...)
    {
    }

    constexpr Markable(Maybe<T> value) noexcept
        : _value(bool(value) ? krys::move(*value) : Traits::EmptyValue())
    {
    }

    constexpr explicit operator bool() const noexcept
    {
      return !Traits::IsEmptyValue(_value);
    }

    void reset() noexcept
    {
      _value = Traits::EmptyValue();
    }

    KRYS_NODISCARD constexpr const T &value() const & noexcept KRYS_LIFETIME_BOUND
    {
      krys_debug_assert(bool(*this));
      return _value;
    }

    KRYS_NODISCARD constexpr T &value() & noexcept KRYS_LIFETIME_BOUND
    {
      krys_debug_assert(bool(*this));
      return _value;
    }

    KRYS_NODISCARD constexpr T &&value() && noexcept
    {
      krys_debug_assert(bool(*this));
      return krys::move(_value);
    }

    KRYS_NODISCARD constexpr const T *operator->() const noexcept KRYS_LIFETIME_BOUND
    {
      krys_debug_assert(bool(*this));
      return std::addressof(_value);
    }

    KRYS_NODISCARD constexpr T *operator->() noexcept KRYS_LIFETIME_BOUND
    {
      krys_debug_assert(bool(*this));
      return std::addressof(_value);
    }

    KRYS_NODISCARD constexpr const T &operator*() const & noexcept KRYS_LIFETIME_BOUND
    {
      krys_debug_assert(bool(*this));
      return _value;
    }

    KRYS_NODISCARD constexpr T &operator*() & noexcept KRYS_LIFETIME_BOUND
    {
      krys_debug_assert(bool(*this));
      return _value;
    }

    template <class U>
    KRYS_NODISCARD constexpr T value_or(U &&fallback) const noexcept
    {
      if (bool(*this))
      {
        return _value;
      }

      return static_cast<T>(std::forward<U>(fallback));
    }

    KRYS_NODISCARD operator Maybe<T>() && noexcept
    {
      if (bool(*this))
      {
        return krys::move(_value);
      }

      return null;
    }

    KRYS_NODISCARD operator Maybe<T>() const & noexcept
    {
      if (bool(*this))
      {
        return _value;
      }

      return null;
    }

    KRYS_NODISCARD Maybe<T> AsMaybe() const noexcept
    {
      return Maybe<T>(*this);
    }
  };

  template <typename T, typename Traits>
  KRYS_NODISCARD constexpr bool operator==(const Markable<T, Traits> &x,
                                           const Markable<T, Traits> &y) noexcept
  {
    if (bool(x) != bool(y))
    {
      return false;
    }

    if (!bool(x))
    {
      return true;
    }

    return x.value() == y.value();
  }

  template <typename T, typename Traits>
  KRYS_NODISCARD constexpr bool operator==(const Markable<T, Traits> &x, const T &v) noexcept
  {
    return bool(x) && x.value() == v;
  }
}
