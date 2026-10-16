#pragma once

#include "Krystal.Booey/Graphics/FontSizeAdjust.hpp"
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
