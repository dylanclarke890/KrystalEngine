/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::css
{
#pragma region Angle

  KRYS_NODISCARD inline double CanonicalizeAngle(double value, AngleUnit unit) noexcept
  {
    return ConvertAngle<AngleUnit::Deg>(value, unit);
  }

  template <auto R, typename V>
  KRYS_NODISCARD double Canonicalize(AngleRaw<R, V> raw)
  {
    return CanonicalizeAngle(raw.Value, raw.Unit);
  }

#pragma endregion

#pragma region Time

  KRYS_NODISCARD inline double CanonicalizeTime(double value, TimeUnit unit) noexcept
  {
    return ConvertTime<TimeUnit::S>(value, unit);
  }

  template <auto R, typename V>
  KRYS_NODISCARD double Canonicalize(TimeRaw<R, V> raw)
  {
    return CanonicalizeTime(raw.Value, raw.Unit);
  }

#pragma endregion

#pragma region Frequency

  KRYS_NODISCARD inline double CanonicalizeFrequency(double value, FrequencyUnit unit) noexcept
  {
    return ConvertFrequency<FrequencyUnit::Hz>(value, unit);
  }

  template <auto R, typename V>
  KRYS_NODISCARD double Canonicalize(FrequencyRaw<R, V> raw)
  {
    return CanonicalizeFrequency(raw.Value, raw.Unit);
  }

#pragma endregion

#pragma region Resolution

  KRYS_NODISCARD inline double CanonicalizeResolution(double value, ResolutionUnit unit) noexcept
  {
    return ConvertResolution<ResolutionUnit::Dppx>(value, unit);
  }

  template <auto R, typename V>
  KRYS_NODISCARD double Canonicalize(ResolutionRaw<R, V> raw)
  {
    return CanonicalizeResolution(raw.Value, raw.Unit);
  }

#pragma endregion
}