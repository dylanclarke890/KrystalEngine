#include "Krystal.Booey/Style/Values/ScrollAnimations/ScrollFunction.hpp"

#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Values/ScrollValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  ScrollFunction CSSValueConversion<ScrollFunction>::operator()(BuilderState &state,
                                                                const css::Value &value) noexcept
  {
    const auto *scrollValue = RequiredDowncast<css::ScrollValue>(state, value);
    if (!scrollValue)
    {
      return ScrollFunction {ScrollFunctionParameters {Scroller::Nearest, ScrollAxis::Block}};
    }
    return this->operator()(state, *scrollValue);
  }

  ScrollFunction CSSValueConversion<ScrollFunction>::operator()(BuilderState &,
                                                                const css::ScrollValue &value) noexcept
  {
    return ScrollFunction {ScrollFunctionParameters {
      value.Scroller() ? FromValueId<Scroller>(value.Scroller()->ValueId()) : Scroller::Nearest,
      value.Axis() ? FromValueId<ScrollAxis>(value.Axis()->ValueId()) : ScrollAxis::Block}};
  }

  Ref<css::Value> CSSValueCreation<ScrollFunction>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                               const ScrollFunction &value) noexcept
  {
    return css::ScrollValue::Create(CreateCSSValue(pool, style, value->Scroller),
                                    CreateCSSValue(pool, style, value->Axis));
  }

  // MARK: - Serialization

  void Serialiser<ScrollFunctionParameters>::operator()(css::CSSOMString &builder,
                                                        const css::SerialisationContext &context,
                                                        const RenderStyle &style,
                                                        const ScrollFunctionParameters &value) noexcept
  {
    auto hasScroller = value.Scroller != Scroller::Nearest;
    auto hasAxis = value.Axis != ScrollAxis::Block;

    bool needsSpace = false;
    if (hasScroller)
    {
      Serialise(builder, context, style, value.Scroller);
      needsSpace = true;
    }

    if (hasAxis)
    {
      if (needsSpace)
      {
        builder.push_back(u8' ');
      }
      Serialise(builder, context, style, value.Axis);
    }
  }
}
