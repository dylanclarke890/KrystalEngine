/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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
#include <algorithm>
#include <compare>

namespace krys::boo::style
{
  struct DynamicRangeLimit;
}

namespace krys::boo
{
  /// @brief A `dynamic-range-limit` as a single value in [0, 1]: 0 is `standard`, 0.5 `constrained` and 1
  /// `no-limit`.
  class PlatformDynamicRangeLimit
  {
    friend style::DynamicRangeLimit;

  public:
    using ValueType = float;

  private:
    constexpr static ValueType StandardValue = 0;
    constexpr static ValueType ConstrainedValue = 0.5;
    constexpr static ValueType NoLimitValue = 1;

    ValueType _value {NoLimitValue};

  public:
    KRYS_NODISCARD constexpr static PlatformDynamicRangeLimit Standard() noexcept
    {
      return PlatformDynamicRangeLimit(StandardValue);
    }

    KRYS_NODISCARD constexpr static PlatformDynamicRangeLimit Constrained() noexcept
    {
      return PlatformDynamicRangeLimit(ConstrainedValue);
    }

    KRYS_NODISCARD constexpr static PlatformDynamicRangeLimit NoLimit() noexcept
    {
      return PlatformDynamicRangeLimit(NoLimitValue);
    }

    KRYS_NODISCARD constexpr static PlatformDynamicRangeLimit InitialValue() noexcept
    {
      return NoLimit();
    }

    KRYS_NODISCARD constexpr static PlatformDynamicRangeLimit InitialValueForVideos() noexcept
    {
      return NoLimit();
    }

    KRYS_NODISCARD constexpr static PlatformDynamicRangeLimit DefaultWhenSuppressingHDR() noexcept
    {
      return Standard();
    }

    KRYS_NODISCARD constexpr static PlatformDynamicRangeLimit DefaultWhenSuppressingHDRInVideos() noexcept
    {
      return Constrained();
    }

    // `dynamic-range-limit` mapped to PlatformDynamicRangeLimit.Value():
    // ["standard", "constrained"] -> [Standard().Value(), Constrained().Value()],
    // ["constrained", "noLimit"] -> [Constrained().Value(), NoLimit().Value()]
    KRYS_NODISCARD constexpr ValueType Value() const noexcept
    {
      return _value;
    }

    KRYS_NODISCARD constexpr auto operator<=>(const PlatformDynamicRangeLimit &) const noexcept = default;

  private:
    explicit constexpr PlatformDynamicRangeLimit(ValueType value) noexcept
        : _value(std::clamp(value, 0.0f, 1.0f))
    {
    }

    PlatformDynamicRangeLimit(float standardPercent, float constrainedPercent, float noLimitPercent) noexcept
        : _value(NormalizedAverage(standardPercent, constrainedPercent, noLimitPercent))
    {
    }

    KRYS_NODISCARD static ValueType NormalizedAverage(float standardPercent, float constrainedPercent,
                                                      float noLimitPercent) noexcept;
  };
}
