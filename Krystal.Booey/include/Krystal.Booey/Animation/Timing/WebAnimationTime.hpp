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

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <chrono>
#include <compare>

namespace krys::boo
{
  /// @brief A duration in seconds, which may be infinite or NaN.
  using Seconds = std::chrono::duration<double>;

  /// @brief The precision animation times are compared with: a microsecond.
  constexpr Seconds TimeEpsilon {0.000001};

  /// @brief An animation time: a time (for time-based timelines) or a percentage (for progress-based
  /// timelines).
  class WebAnimationTime
  {
  private:
    enum class Type : uint8
    {
      Unknown,
      Time,
      Percentage
    };

    Type _type {Type::Unknown};
    double _value {0};

    constexpr WebAnimationTime(Type type, double value) noexcept : _type(type), _value(value)
    {
    }

  public:
    constexpr WebAnimationTime() noexcept = default;
    WebAnimationTime(Maybe<Seconds> time, Maybe<double> percentage) noexcept;
    constexpr WebAnimationTime(const Seconds &value) noexcept : _type(Type::Time), _value(value.count())
    {
    }

    KRYS_NODISCARD static WebAnimationTime FromMilliseconds(double milliseconds) noexcept;
    KRYS_NODISCARD static WebAnimationTime FromPercentage(double percentage) noexcept;

    KRYS_NODISCARD Maybe<Seconds> Time() const noexcept;
    KRYS_NODISCARD Maybe<double> Percentage() const noexcept;

    KRYS_NODISCARD bool IsValid() const noexcept;
    KRYS_NODISCARD bool IsInfinity() const noexcept;
    KRYS_NODISCARD bool IsZero() const noexcept;
    KRYS_NODISCARD bool IsNaN() const noexcept;

    KRYS_NODISCARD WebAnimationTime MatchingZero() const noexcept;
    KRYS_NODISCARD WebAnimationTime MatchingEpsilon() const noexcept;
    KRYS_NODISCARD WebAnimationTime MatchingInfinity() const noexcept;

    KRYS_NODISCARD bool ApproximatelyEqualTo(const WebAnimationTime &other) const noexcept;
    KRYS_NODISCARD bool ApproximatelyLessThan(const WebAnimationTime &other) const noexcept;
    KRYS_NODISCARD bool ApproximatelyGreaterThan(const WebAnimationTime &other) const noexcept;

    KRYS_NODISCARD WebAnimationTime operator+(const WebAnimationTime &other) const noexcept;
    KRYS_NODISCARD WebAnimationTime operator-(const WebAnimationTime &other) const noexcept;
    KRYS_NODISCARD double operator/(const WebAnimationTime &other) const noexcept;
    WebAnimationTime &operator+=(const WebAnimationTime &other) noexcept;
    WebAnimationTime &operator-=(const WebAnimationTime &other) noexcept;

    KRYS_NODISCARD friend bool operator==(const WebAnimationTime &,
                                          const WebAnimationTime &) noexcept = default;
    KRYS_NODISCARD friend std::partial_ordering operator<=>(const WebAnimationTime &a,
                                                            const WebAnimationTime &b) noexcept;
    KRYS_NODISCARD friend std::partial_ordering operator<=>(const WebAnimationTime &a, Seconds b) noexcept;

    KRYS_NODISCARD WebAnimationTime operator+(const Seconds &other) const noexcept;
    KRYS_NODISCARD WebAnimationTime operator-(const Seconds &other) const noexcept;
    KRYS_NODISCARD bool operator==(const Seconds &other) const noexcept;
    KRYS_NODISCARD WebAnimationTime operator*(double scalar) const noexcept;
    KRYS_NODISCARD WebAnimationTime operator/(double scalar) const noexcept;

    KRYS_NODISCARD explicit operator Seconds() const noexcept;
  };
}
