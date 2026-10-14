#pragma once

#include "Krystal.Booey/Graphics/FontMetrics.hpp"
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
