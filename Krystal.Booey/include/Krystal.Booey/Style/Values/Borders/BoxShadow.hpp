#pragma once

#include "Krystal.Booey/CSS/Values/Borders/BoxShadow.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/PrimitiveNumeric.hpp"
#include "Krystal.Booey/Style/Values/Borders/Shadow.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::style
{
  struct BoxShadow
  {
    style::Color Color;
    css::SpaceSeparatedPoint<Length<css::AllUnzoomed>> Location;
    Length<css::NonNegativeUnzoomed> Blur;
    Length<css::AllUnzoomed> Spread;
    Maybe<css::keywords::Inset> Inset;
    bool IsWebkitBoxShadow {false};

    KRYS_NODISCARD bool operator==(const BoxShadow &) const noexcept = default;
  };

  template <size_t I>
  const auto &get(const BoxShadow &value)
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
    else if constexpr (I == 3)
    {
      return value.Spread;
    }
    else if constexpr (I == 4)
    {
      return value.Inset;
    }
  }

  // <box-shadow-list> = <single-box-shadow>#
  using BoxShadowList = ShadowList<BoxShadow>;

  // <'box-shadow'> = none | <box-shadow-list>
  // https://www.w3.org/TR/css-backgrounds-3/#propdef-box-shadow
  using BoxShadows = Shadows<BoxShadow>;

  // MARK: - Conversions

  template <>
  struct ToCSS<BoxShadow>
  {
    KRYS_NODISCARD css::BoxShadow operator()(const BoxShadow &, const RenderStyle &) noexcept;
  };
  template <>
  struct ToStyle<css::BoxShadow>
  {
    KRYS_NODISCARD BoxShadow operator()(const css::BoxShadow &, const BuilderState &) noexcept;
  };

  // `BoxShadowList` is special-cased to return a `css::BoxShadowPropertyValue`.
  template <>
  struct CSSValueCreation<BoxShadowList>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const BoxShadowList &) noexcept;
  };
  template <>
  struct CSSValueConversion<BoxShadows>
  {
    KRYS_NODISCARD BoxShadows operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<BoxShadowList>
  {
    void operator()(css::CSSOMString &, const css::SerialisationContext &, const RenderStyle &,
                    const BoxShadowList &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<BoxShadow>
  {
    KRYS_NODISCARD BoxShadow Blend(const BoxShadow &, const BoxShadow &, const RenderStyle &,
                                   const RenderStyle &, const BlendingContext &) noexcept;
  };

  template <>
  struct Blending<BoxShadows>
  {
    KRYS_NODISCARD bool CanBlend(const BoxShadows &, const BoxShadows &, CompositeOperation) noexcept;
    KRYS_NODISCARD constexpr bool RequiresInterpolationForAccumulativeIteration(const BoxShadows &,
                                                                                const BoxShadows &) noexcept
    {
      return true;
    }
    KRYS_NODISCARD BoxShadows Blend(const BoxShadows &, const BoxShadows &, const RenderStyle &,
                                    const RenderStyle &, const BlendingContext &) noexcept;
  };

  // MARK: - Shadow-specific Interfaces

  KRYS_NODISCARD inline ShadowStyle GetShadowStyle(const BoxShadow &shadow) noexcept
  {
    return shadow.Inset.has_value() ? ShadowStyle::Inset : ShadowStyle::Normal;
  }

  KRYS_NODISCARD inline bool IsInset(const BoxShadow &shadow) noexcept
  {
    return shadow.Inset.has_value();
  }

  KRYS_NODISCARD inline LayoutUnit PaintingSpread(const BoxShadow &shadow,
                                                  const style::ZoomFactor &zoomFactor) noexcept
  {
    return LayoutUnit {shadow.Spread.ResolveZoom(zoomFactor)};
  }
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::BoxShadow, 5)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::BoxShadows)
