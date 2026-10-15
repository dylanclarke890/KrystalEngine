#pragma once

#include "Krystal.Booey/CSS/Values/TextDecoration/TextShadow.hpp"
#include "Krystal.Booey/Style/Values/Borders/Shadow.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::style
{
  struct TextShadow
  {
    style::Color Color;
    css::SpaceSeparatedPoint<Length<css::AllUnzoomed>> Location;
    Length<css::NonNegativeUnzoomed> Blur;

    KRYS_NODISCARD bool operator==(const TextShadow &) const noexcept = default;
  };

  template <size_t I>
  const auto &get(const TextShadow &value)
  {
    if constexpr (!I)
    {
      return value.Color;
    }
    else if constexpr (I == 1)
    {
      return value.Location;
    }
    else if constexpr (I == 2)
    {
      return value.Blur;
    }
  }

  // <text-shadow-list> = <single-text-shadow>#
  using TextShadowList = ShadowList<TextShadow>;

  // <'text-shadow'> = none | <text-list>
  // https://www.w3.org/TR/css-text-decor-3/#propdef-text-shadow
  using TextShadows = Shadows<TextShadow>;

  // MARK: - Conversions

  template <>
  struct ToCSS<TextShadow>
  {
    KRYS_NODISCARD css::TextShadow operator()(const TextShadow &, const RenderStyle &) noexcept;
  };
  template <>
  struct ToStyle<css::TextShadow>
  {
    KRYS_NODISCARD TextShadow operator()(const css::TextShadow &, const BuilderState &) noexcept;
  };

  // `TextShadowList` is special-cased to return a `css::TextShadowPropertyValue`.
  template <>
  struct CSSValueCreation<TextShadowList>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const TextShadowList &) noexcept;
  };
  template <>
  struct CSSValueConversion<TextShadows>
  {
    KRYS_NODISCARD TextShadows operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<TextShadowList>
  {
    void operator()(css::CSSOMString &, const css::SerialisationContext &, const RenderStyle &,
                    const TextShadowList &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<TextShadow>
  {
    KRYS_NODISCARD TextShadow Blend(const TextShadow &, const TextShadow &, const RenderStyle &,
                                    const RenderStyle &, const BlendingContext &) noexcept;
  };

  template <>
  struct Blending<TextShadows>
  {
    KRYS_NODISCARD bool CanBlend(const TextShadows &, const TextShadows &, CompositeOperation) noexcept;
    KRYS_NODISCARD constexpr bool RequiresInterpolationForAccumulativeIteration(const TextShadows &,
                                                                                const TextShadows &) noexcept
    {
      return true;
    }
    KRYS_NODISCARD TextShadows Blend(const TextShadows &, const TextShadows &, const RenderStyle &,
                                     const RenderStyle &, const BlendingContext &) noexcept;
  };

  // MARK: - Shadow-specific Interfaces

  KRYS_NODISCARD constexpr ShadowStyle GetShadowStyle(const TextShadow &) noexcept
  {
    return ShadowStyle::Normal;
  }

  KRYS_NODISCARD constexpr bool IsInset(const TextShadow &) noexcept
  {
    return false;
  }

  KRYS_NODISCARD constexpr LayoutUnit PaintingSpread(const TextShadow &, const style::ZoomFactor &) noexcept
  {
    return LayoutUnit();
  }
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::TextShadow, 3)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextShadows)
