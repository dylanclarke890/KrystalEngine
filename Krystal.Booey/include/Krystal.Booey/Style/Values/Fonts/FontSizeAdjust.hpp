/*
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

#include "Krystal.Booey/Graphics/Fonts/FontSizeAdjust.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // <'font-size-adjust'> = none | [ [ ex-height | cap-height | ch-width | ic-width | ic-height ]? [ from-font
  // | <number [0,inf]> ] ]
  // FIXME: Current spec grammar is `none | <number [0,∞]>`
  // https://drafts.csswg.org/css-fonts-4/#propdef-font-size-adjust
  struct FontSizeAdjust
  {
    using Number = style::Number<css::NonNegative>;
    using Metric = boo::FontSizeAdjust::Metric;
    using ValueType = boo::FontSizeAdjust::ValueType;

    FontSizeAdjust(css::keywords::None) noexcept : _platform {Metric::ExHeight}
    {
    }
    FontSizeAdjust(Metric metric, css::keywords::FromFont) noexcept
        : _platform {metric, ValueType::FromFont, std::nullopt}
    {
    }
    FontSizeAdjust(Metric metric, Number metricValue) noexcept
        : _platform {metric, ValueType::Number, static_cast<float>(metricValue.Value)}
    {
    }

    FontSizeAdjust(boo::FontSizeAdjust platform) noexcept : _platform {platform}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return _platform.IsNone();
    }
    KRYS_NODISCARD bool IsFromFont() const noexcept
    {
      return _platform.IsFromFont();
    }

    KRYS_NODISCARD Metric GetMetric() const noexcept
    {
      return _platform.MetricType;
    }
    KRYS_NODISCARD Maybe<float> MetricValue() const noexcept
    {
      return _platform.Value ? Maybe<float> {*_platform.Value} : std::nullopt;
    }
    KRYS_NODISCARD Maybe<float> ResolvedMetricValue(const RenderStyle &) const noexcept;

    KRYS_NODISCARD boo::FontSizeAdjust Platform() const noexcept
    {
      return _platform;
    }

    KRYS_NODISCARD bool operator==(const FontSizeAdjust &) const noexcept = default;

  private:
    boo::FontSizeAdjust _platform;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<FontSizeAdjust>
  {
    KRYS_NODISCARD FontSizeAdjust operator()(BuilderState &, const css::Value &) noexcept;
  };
  // NOTE: `FontSizeAdjust` is special-cased to allow resolution of `from-font`.
  template <>
  struct CSSValueCreation<FontSizeAdjust>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const FontSizeAdjust &) noexcept;
  };

  // MARK: - Serialization

  // NOTE: `FontSizeAdjust` is special-cased to allow resolution of `from-font`.
  template <>
  struct Serialiser<FontSizeAdjust>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const FontSizeAdjust &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<FontSizeAdjust>
  {
    KRYS_NODISCARD bool CanBlend(const FontSizeAdjust &, const FontSizeAdjust &) noexcept;
    KRYS_NODISCARD FontSizeAdjust Blend(const FontSizeAdjust &, const FontSizeAdjust &,
                                        const BlendingContext &) noexcept;
  };
}
