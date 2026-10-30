#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'scroll-snap-type'> = none | [ x | y | block | inline | both ] [ mandatory | proximity
  // ]?@(default=proximity) https://drafts.csswg.org/css-scroll-snap-1/#propdef-scroll-snap-type
  struct ScrollSnapType
  {
    struct ScrollSnapContainer
    {
      ScrollSnapAxis Axis;
      ScrollSnapStrictness Strictness;

      KRYS_NODISCARD constexpr bool operator==(const ScrollSnapContainer &) const noexcept = default;
    };

    constexpr ScrollSnapType(css::keywords::None) noexcept
    {
    }
    constexpr ScrollSnapType(ScrollSnapAxis axis,
                             ScrollSnapStrictness strictness = ScrollSnapStrictness::Proximity) noexcept
        : _value {ScrollSnapContainer {axis, strictness}}
    {
    }

    KRYS_NODISCARD constexpr bool IsNone() const noexcept
    {
      return !_value;
    }
    KRYS_NODISCARD constexpr bool IsContainer() const noexcept
    {
      return !!_value;
    }
    KRYS_NODISCARD constexpr Maybe<ScrollSnapContainer> TryContainer() const noexcept
    {
      return _value;
    }

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNone())
      {
        return visitor(css::keywords::None {});
      }

      switch (_value->Strictness)
      {
        case ScrollSnapStrictness::Proximity: return visitor(_value->Axis);
        case ScrollSnapStrictness::Mandatory:
          return visitor(css::SpaceSeparatedTuple {_value->Axis, css::keywords::Mandatory {}});
      }
      krys_unreachable();
    }

    KRYS_NODISCARD constexpr bool operator==(const ScrollSnapType &) const noexcept = default;

  private:
    Maybe<ScrollSnapContainer> _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<ScrollSnapType>
  {
    KRYS_NODISCARD ScrollSnapType operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ScrollSnapType)
