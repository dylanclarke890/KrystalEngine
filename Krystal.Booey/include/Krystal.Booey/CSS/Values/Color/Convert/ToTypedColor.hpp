/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/CSS/Values/Color/ColorDescriptors.hpp"
#include "Krystal.Booey/CSS/Values/Color/Convert/Normalise.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <limits>

namespace krys::boo::css
{
  // This file implements support for converting style resolved parsed values (e.g. tuple
  // of `Variant<style::Number<>, style::Percentage<>, ...>`) into typed colors (e.g. `SRGBA<float>`).

  template <typename Descriptor, size_t Index>
  KRYS_NODISCARD float ConvertToTypeColorComponent(style::Number<> number) noexcept
  {
    constexpr auto info = std::get<Index>(Descriptor::Components);
    constexpr auto multiplier = info.NumberMultiplier;
    constexpr auto min = info.Min * info.NumberMultiplier;
    constexpr auto max = info.Max * info.NumberMultiplier;

    if constexpr (info.Type == ColourComponentType::Angle)
    {
      return static_cast<float>(NormalizeHue(number.Value));
    }
    else if constexpr (info.Min == -std::numeric_limits<double>::infinity()
                       && info.Max == std::numeric_limits<double>::infinity())
    {
      return static_cast<float>(number.Value * multiplier);
    }
    else if constexpr (info.Min == -std::numeric_limits<double>::infinity())
    {
      return static_cast<float>(std::min(number.Value * multiplier, max));
    }
    else if constexpr (info.Max == std::numeric_limits<double>::infinity())
    {
      return static_cast<float>(std::max(number.Value * multiplier, min));
    }
    else
    {
      return static_cast<float>(std::clamp(number.Value * multiplier, min, max));
    }
  }

  template <typename Descriptor, size_t Index>
  KRYS_NODISCARD float ConvertToTypeColorComponent(style::Percentage<> percent) noexcept
  {
    constexpr auto info = std::get<Index>(Descriptor::Components);
    constexpr auto multiplier = info.PercentMultiplier * info.NumberMultiplier;
    constexpr auto min = info.Min * info.NumberMultiplier;
    constexpr auto max = info.Max * info.NumberMultiplier;

    if constexpr (info.Min == -std::numeric_limits<double>::infinity()
                  && info.Max == std::numeric_limits<double>::infinity())
    {
      return static_cast<float>(percent.Value * multiplier);
    }
    else if constexpr (info.Min == -std::numeric_limits<double>::infinity())
    {
      return static_cast<float>(std::min(percent.Value * multiplier, max));
    }
    else if constexpr (info.Max == std::numeric_limits<double>::infinity())
    {
      return static_cast<float>(std::max(percent.Value * multiplier, min));
    }
    else
    {
      return static_cast<float>(std::clamp(percent.Value * multiplier, min, max));
    }
  }

  template <typename Descriptor, size_t Index>
  KRYS_NODISCARD float ConvertToTypeColorComponent(style::Angle<> angle) noexcept
  {
    constexpr auto info = std::get<Index>(Descriptor::Components);
    static_assert(info.Type == ColourComponentType::Angle);

    return static_cast<float>(NormalizeHue(angle.Value));
  }

  template <typename Descriptor, size_t Index>
  KRYS_NODISCARD float ConvertToTypeColorComponent(keywords::None) noexcept
  {
    return std::numeric_limits<float>::quiet_NaN();
  }

  template <typename Descriptor, size_t Index, typename... Ts>
  KRYS_NODISCARD float ConvertToTypeColorComponent(const Variant<Ts...> &variant) noexcept
  {
    return krys::SwitchOn(variant,
                          [](auto value) { return ConvertToTypeColorComponent<Descriptor, Index>(value); });
  }

  template <typename Descriptor, size_t Index, typename T>
  KRYS_NODISCARD float ConvertToTypeColorComponent(const Maybe<T> &optional, float defaultValue) noexcept
  {
    return optional ? ConvertToTypeColorComponent<Descriptor, Index>(*optional) : defaultValue;
  }

  template <typename Descriptor>
  KRYS_NODISCARD GetColorType<Descriptor> ConvertToTypedColor(StyleColorParseType<Descriptor> parsed,
                                                              double defaultAlpha) noexcept
  {
    return {
      ConvertToTypeColorComponent<Descriptor, 0>(std::get<0>(parsed)),
      ConvertToTypeColorComponent<Descriptor, 1>(std::get<1>(parsed)),
      ConvertToTypeColorComponent<Descriptor, 2>(std::get<2>(parsed)),
      ConvertToTypeColorComponent<Descriptor, 3>(std::get<3>(parsed), static_cast<float>(defaultAlpha))};
  }
}
