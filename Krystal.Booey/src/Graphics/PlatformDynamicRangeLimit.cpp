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

#include "Krystal.Booey/Graphics/PlatformDynamicRangeLimit.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include <cmath>

namespace krys::boo
{
  PlatformDynamicRangeLimit::ValueType
    PlatformDynamicRangeLimit::NormalizedAverage(float standardPercent, float constrainedPercent,
                                                 float noLimitPercent) noexcept
  {
    if (!std::isfinite(standardPercent) || standardPercent < 0 || standardPercent > 100
        || !std::isfinite(constrainedPercent) || constrainedPercent < 0 || constrainedPercent > 100
        || !std::isfinite(noLimitPercent) || noLimitPercent < 0 || noLimitPercent > 100)
    {
      krys_debug_assert(std::isfinite(standardPercent));
      krys_debug_assert(standardPercent >= 0);
      krys_debug_assert(standardPercent <= 100);
      krys_debug_assert(std::isfinite(constrainedPercent));
      krys_debug_assert(constrainedPercent >= 0);
      krys_debug_assert(constrainedPercent <= 100);
      krys_debug_assert(std::isfinite(noLimitPercent));
      krys_debug_assert(noLimitPercent >= 0);
      krys_debug_assert(noLimitPercent <= 100);
      // Unexpected value -> Clamp HDR down to prevent misuses.
      return StandardValue;
    }

    float sum = standardPercent + constrainedPercent + noLimitPercent;
    if (!sum)
    {
      krys_debug_assert(sum);
      return StandardValue;
    }

    float weightedSum =
      standardPercent * StandardValue + constrainedPercent * ConstrainedValue + noLimitPercent * NoLimitValue;
    float weightedAverage = SafeFPDivision(weightedSum, sum);
    return std::clamp(weightedAverage, 0.0f, 1.0f);
  }
}
