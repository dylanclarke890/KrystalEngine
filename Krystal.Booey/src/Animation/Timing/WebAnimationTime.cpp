/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Animation/Timing/WebAnimationTime.hpp"
#include "Krystal.Core/Assert.hpp"
#include <cmath>
#include <limits>

namespace krys::boo
{
  WebAnimationTime::WebAnimationTime(Maybe<Seconds> time, Maybe<double> percentage) noexcept
  {
    krys_debug_assert(time || percentage);
    krys_debug_assert(!!time != !!percentage);
    if (time)
    {
      _type = Type::Time;
      _value = time->count();
    }
    else
    {
      _type = Type::Percentage;
      _value = *percentage;
    }
  }

  WebAnimationTime WebAnimationTime::FromMilliseconds(double milliseconds) noexcept
  {
    return {Type::Time, milliseconds / 1'000};
  }

  WebAnimationTime WebAnimationTime::FromPercentage(double percentage) noexcept
  {
    return {Type::Percentage, percentage};
  }

  Maybe<Seconds> WebAnimationTime::Time() const noexcept
  {
    if (_type == Type::Time)
    {
      return Seconds {_value};
    }
    return null;
  }

  Maybe<double> WebAnimationTime::Percentage() const noexcept
  {
    if (_type == Type::Percentage)
    {
      return _value;
    }
    return null;
  }

  bool WebAnimationTime::IsValid() const noexcept
  {
    return _type != Type::Unknown;
  }

  bool WebAnimationTime::IsInfinity() const noexcept
  {
    return std::isinf(_value);
  }

  bool WebAnimationTime::IsZero() const noexcept
  {
    return !_value;
  }

  bool WebAnimationTime::IsNaN() const noexcept
  {
    return std::isnan(_value);
  }

  WebAnimationTime WebAnimationTime::MatchingZero() const noexcept
  {
    return {_type, 0};
  }

  WebAnimationTime WebAnimationTime::MatchingInfinity() const noexcept
  {
    return {_type, std::numeric_limits<double>::infinity()};
  }

  WebAnimationTime WebAnimationTime::MatchingEpsilon() const noexcept
  {
    if (_type == Type::Percentage)
    {
      return WebAnimationTime::FromPercentage(0.000001);
    }
    return {TimeEpsilon};
  }

  bool WebAnimationTime::ApproximatelyEqualTo(const WebAnimationTime &other) const noexcept
  {
    krys_debug_assert(_type == other._type);
    if (_type == Type::Time)
    {
      return std::abs(_value - other._value) < TimeEpsilon.count();
    }
    return _value == other._value;
  }

  bool WebAnimationTime::ApproximatelyLessThan(const WebAnimationTime &other) const noexcept
  {
    krys_debug_assert(_type == other._type);
    if (_type == Type::Time)
    {
      return (_value + TimeEpsilon.count()) < other._value;
    }
    return _value < other._value;
  }

  bool WebAnimationTime::ApproximatelyGreaterThan(const WebAnimationTime &other) const noexcept
  {
    krys_debug_assert(_type == other._type);
    if (_type == Type::Time)
    {
      return (_value - TimeEpsilon.count()) > other._value;
    }
    return _value > other._value;
  }

  WebAnimationTime WebAnimationTime::operator+(const WebAnimationTime &other) const noexcept
  {
    krys_debug_assert(_type == other._type);
    return {_type, _value + other._value};
  }

  WebAnimationTime WebAnimationTime::operator-(const WebAnimationTime &other) const noexcept
  {
    krys_debug_assert(_type == other._type);
    return {_type, _value - other._value};
  }

  double WebAnimationTime::operator/(const WebAnimationTime &other) const noexcept
  {
    krys_debug_assert(_type == other._type);
    return _value / other._value;
  }

  WebAnimationTime &WebAnimationTime::operator+=(const WebAnimationTime &other) noexcept
  {
    krys_debug_assert(_type == other._type);
    _value += other._value;
    return *this;
  }

  WebAnimationTime &WebAnimationTime::operator-=(const WebAnimationTime &other) noexcept
  {
    krys_debug_assert(_type == other._type);
    _value -= other._value;
    return *this;
  }

  std::partial_ordering operator<=>(const WebAnimationTime &a, const WebAnimationTime &b) noexcept
  {
    krys_debug_assert(a._type == b._type);
    return a._value <=> b._value;
  }

  std::partial_ordering operator<=>(const WebAnimationTime &a, Seconds b) noexcept
  {
    krys_debug_assert(a._type == WebAnimationTime::Type::Time);
    return a._value <=> b.count();
  }

  WebAnimationTime WebAnimationTime::operator+(const Seconds &other) const noexcept
  {
    krys_debug_assert(_type == Type::Time);
    return {_type, _value + other.count()};
  }

  WebAnimationTime WebAnimationTime::operator-(const Seconds &other) const noexcept
  {
    krys_debug_assert(_type == Type::Time);
    return {_type, _value - other.count()};
  }

  bool WebAnimationTime::operator==(const Seconds &other) const noexcept
  {
    return _type == Type::Time && _value == other.count();
  }

  WebAnimationTime WebAnimationTime::operator*(double scalar) const noexcept
  {
    return {_type, _value * scalar};
  }

  WebAnimationTime WebAnimationTime::operator/(double scalar) const noexcept
  {
    return {_type, _value / scalar};
  }

  WebAnimationTime::operator Seconds() const noexcept
  {
    krys_debug_assert(_type == Type::Time);
    return Seconds(_value);
  }
}
