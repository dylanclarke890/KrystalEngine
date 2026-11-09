/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include <cfloat>
#include <cmath>
#include <compare>

namespace krys::boo::svg
{
  /// @brief A time in seconds on a SMIL time container's timeline, which can also be unresolved or
  /// indefinite.
  class SMILTime
  {
  private:
    static constexpr double UnresolvedValue = DBL_MAX;
    // Just a big value smaller than DBL_MAX. Our times are relative to 0, we don't really need the full
    // range.
    static constexpr double IndefiniteValue = FLT_MAX;

    double _time {0};

  public:
    constexpr SMILTime() noexcept = default;

    constexpr SMILTime(double time) noexcept : _time(time)
    {
      krys_debug_assert(!std::isnan(time));
    }

    KRYS_NODISCARD static constexpr SMILTime Unresolved() noexcept
    {
      return UnresolvedValue;
    }

    KRYS_NODISCARD static constexpr SMILTime Indefinite() noexcept
    {
      return IndefiniteValue;
    }

    KRYS_NODISCARD constexpr double Value() const noexcept
    {
      return _time;
    }

    KRYS_NODISCARD constexpr bool IsFinite() const noexcept
    {
      return _time < IndefiniteValue;
    }

    KRYS_NODISCARD constexpr bool IsIndefinite() const noexcept
    {
      return _time == IndefiniteValue;
    }

    KRYS_NODISCARD constexpr bool IsUnresolved() const noexcept
    {
      return _time == UnresolvedValue;
    }

    KRYS_NODISCARD friend constexpr bool operator==(const SMILTime &a, const SMILTime &b) noexcept
    {
      return a.IsFinite() && a.Value() == b.Value();
    }

    KRYS_NODISCARD friend constexpr bool operator!(const SMILTime &a) noexcept
    {
      return !a.IsFinite() || !a.Value();
    }

    KRYS_NODISCARD friend constexpr auto operator<=>(const SMILTime &a, const SMILTime &b) noexcept
    {
      return a.Value() <=> b.Value();
    }
  };

  /// @brief An instance time, with whether it came from the document or from a script (or another
  /// dynamic source), whose times are cleared when an interval ends.
  class SMILTimeWithOrigin
  {
  public:
    enum class Origin : bool
    {
      Parser,
      Script
    };

  private:
    SMILTime _time;
    Origin _origin {Origin::Parser};

  public:
    constexpr SMILTimeWithOrigin() noexcept = default;

    constexpr SMILTimeWithOrigin(const SMILTime &time, Origin origin) noexcept : _time(time), _origin(origin)
    {
    }

    KRYS_NODISCARD constexpr const SMILTime &Time() const noexcept
    {
      return _time;
    }

    KRYS_NODISCARD constexpr bool OriginIsScript() const noexcept
    {
      return _origin == Origin::Script;
    }

    KRYS_NODISCARD friend constexpr auto operator<=>(const SMILTimeWithOrigin &a,
                                                     const SMILTimeWithOrigin &b) noexcept
    {
      return a.Time() <=> b.Time();
    }
  };

  KRYS_NODISCARD SMILTime operator+(const SMILTime &, const SMILTime &) noexcept;
  KRYS_NODISCARD SMILTime operator-(const SMILTime &, const SMILTime &) noexcept;
  // So multiplying times does not make too much sense but SMIL defines it for duration * repeatCount
  KRYS_NODISCARD SMILTime operator*(const SMILTime &, const SMILTime &) noexcept;
}
