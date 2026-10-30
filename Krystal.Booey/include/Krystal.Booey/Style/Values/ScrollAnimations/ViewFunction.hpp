#pragma once

#include "Krystal.Booey/Animation/ScrollAxis.hpp"
#include "Krystal.Booey/Style/Values/ScrollAnimations/ViewTimelineInsets.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo
{
  namespace css
  {
    class ViewValue;
  }

  namespace style
  {
    // <view()> = view( [ <axis> || <'view-timeline-inset'> ]? )
    // https://www.w3.org/TR/scroll-animations-1/#funcdef-view
    struct ViewFunctionParameters
    {
      ScrollAxis Axis;
      ViewTimelineInsetItem Insets;

      KRYS_NODISCARD bool operator==(const ViewFunctionParameters &) const noexcept = default;
    };
    using ViewFunction = css::FunctionNotation<css::ValueId::View, ViewFunctionParameters>;

    // MARK: - Conversion

    template <>
    struct CSSValueConversion<ViewFunction>
    {
      KRYS_NODISCARD ViewFunction operator()(BuilderState &, const css::Value &) noexcept;
      KRYS_NODISCARD ViewFunction operator()(BuilderState &, const css::ViewValue &) noexcept;
    };

    template <>
    struct CSSValueCreation<ViewFunction>
    {
      KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                                const ViewFunction &) noexcept;
    };

    // MARK: - Serialization

    template <>
    struct Serialiser<ViewFunctionParameters>
    {
      void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                      const ViewFunctionParameters &) noexcept;
    };

  }
}
