/*
 * Copyright (C) 2023 ChangSeok Oh <changseok@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Fonts/FontMetrics.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief The used value of `font-size-adjust`: the metric and its aspect value (or `from-font`).
  struct FontSizeAdjust
  {
    enum class ValueType : bool
    {
      Number,
      FromFont
    };
    enum class Metric : uint8
    {
      ExHeight,
      CapHeight,
      ChWidth,
      IcWidth,
      IcHeight
    };

    Metric MetricType {Metric::ExHeight};
    ValueType Type {ValueType::Number};
    Markable<float> Value {};

    KRYS_NODISCARD bool operator==(const FontSizeAdjust &) const noexcept = default;

    KRYS_NODISCARD Maybe<float> Resolve(float computedSize, const FontMetrics &fontMetrics) const noexcept
    {
      Maybe<float> metricValue;
      switch (MetricType)
      {
        case FontSizeAdjust::Metric::CapHeight: metricValue = fontMetrics.CapHeight(); break;
        case FontSizeAdjust::Metric::ChWidth:   metricValue = fontMetrics.ZeroWidth(); break;
        // FIXME: Are ic-height and ic-width the same? Gecko treats them the same.
        case FontSizeAdjust::Metric::IcWidth:
        case FontSizeAdjust::Metric::IcHeight:  metricValue = fontMetrics.IdeogramWidth(); break;
        case FontSizeAdjust::Metric::ExHeight:
        default:                                metricValue = fontMetrics.XHeight();
      }

      return metricValue.has_value() && computedSize ? Maybe<float> {*metricValue / computedSize}
                                                     : std::nullopt;
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return !Value && Type != ValueType::FromFont;
    }
    KRYS_NODISCARD bool IsFromFont() const noexcept
    {
      return Type == ValueType::FromFont;
    }
    KRYS_NODISCARD bool ShouldResolveFromFont() const noexcept
    {
      return IsFromFont() && !Value;
    }
  };
}

namespace krys
{
  template <>
  struct DefaultHash<boo::FontSizeAdjust>
  {
    KRYS_NODISCARD static size_t Hash(const boo::FontSizeAdjust &fontSizeAdjust) noexcept
    {
      return krys::Hash::Combine(static_cast<uint8>(fontSizeAdjust.MetricType),
                                 static_cast<bool>(fontSizeAdjust.Type), fontSizeAdjust.Value.value_or(0.f));
    }

    KRYS_NODISCARD static bool Equal(const boo::FontSizeAdjust &a, const boo::FontSizeAdjust &b) noexcept
    {
      return a == b;
    }
  };
}
