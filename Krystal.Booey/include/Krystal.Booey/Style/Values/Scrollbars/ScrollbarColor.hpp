#pragma once

#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Core/Types/Markable.hpp"

namespace krys::boo::style
{
  // <scrollbar-color-parts> = <color>{2}
  struct ScrollbarColorParts
  {
    Color Thumb;
    Color Track;

    KRYS_NODISCARD bool operator==(const ScrollbarColorParts &) const noexcept = default;
  };
  template <size_t I>
  KRYS_NODISCARD const auto &get(const ScrollbarColorParts &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Thumb;
    }
    else if constexpr (I == 1)
    {
      return value.Track;
    }
  }

  struct ScrollbarColorMarkableTraits
  {
    KRYS_NODISCARD static bool IsEmptyValue(const ScrollbarColorParts &value) noexcept
    {
      return krys::MarkableTraits<Color>::IsEmptyValue(value.Thumb);
    }
    KRYS_NODISCARD static ScrollbarColorParts EmptyValue() noexcept
    {
      return {krys::MarkableTraits<Color>::EmptyValue(), krys::MarkableTraits<Color>::EmptyValue()};
    }
  };

  // <'scrollbar-color'> = auto | <color>{2}
  // https://www.w3.org/TR/css-scrollbars/#propdef-scrollbar-color
  struct ScrollbarColor
      : css::ValueOrKeyword<ScrollbarColorParts, css::keywords::Auto, ScrollbarColorMarkableTraits>
  {
    using Base::Base;
    using Parts = typename Base::Value;

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return IsKeyword();
    }
    KRYS_NODISCARD bool IsParts() const noexcept
    {
      return IsValue();
    }
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<ScrollbarColor>
  {
    KRYS_NODISCARD ScrollbarColor operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<ScrollbarColor>
  {
    bool Equals(const ScrollbarColor &, const ScrollbarColor &, const RenderStyle &,
                const RenderStyle &) noexcept;
    KRYS_NODISCARD bool CanBlend(const ScrollbarColor &, const ScrollbarColor &) noexcept;
    ScrollbarColor Blend(const ScrollbarColor &, const ScrollbarColor &, const RenderStyle &,
                         const RenderStyle &, const BlendingContext &) noexcept;
  };
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::ScrollbarColorParts, 2)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ScrollbarColor)
