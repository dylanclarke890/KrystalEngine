#pragma once

#include "Krystal.Booey/Animation/ScrollAxis.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo
{
  namespace css
  {
    class ScrollValue;
  }

  namespace style
  {
    // <scroll()> = scroll( [ <scroller> || <axis> ]? )
    // https://www.w3.org/TR/scroll-animations-1/#funcdef-scroll
    struct ScrollFunctionParameters
    {
      boo::Scroller Scroller;
      ScrollAxis Axis;

      KRYS_NODISCARD bool operator==(const ScrollFunctionParameters &) const noexcept = default;
    };
    using ScrollFunction = css::FunctionNotation<css::ValueId::Scroll, ScrollFunctionParameters>;

    // MARK: - Conversion

    template <>
    struct CSSValueConversion<ScrollFunction>
    {
      KRYS_NODISCARD ScrollFunction operator()(BuilderState &, const css::Value &) noexcept;
      KRYS_NODISCARD ScrollFunction operator()(BuilderState &, const css::ScrollValue &) noexcept;
    };

    template <>
    struct CSSValueCreation<ScrollFunction>
    {
      KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                                const ScrollFunction &) noexcept;
    };

    // MARK: - Serialization

    template <>
    struct Serialiser<ScrollFunctionParameters>
    {
      void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                      const ScrollFunctionParameters &) noexcept;
    };

  }
}
