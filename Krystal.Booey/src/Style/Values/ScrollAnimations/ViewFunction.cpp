#include "Krystal.Booey/Style/Values/ScrollAnimations/ViewFunction.hpp"

#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Values/ViewValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"

#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"

#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  ViewFunction CSSValueConversion<ViewFunction>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    const auto *viewValue = RequiredDowncast<css::ViewValue>(state, value);
    if (!viewValue)
    {
      return ViewFunction {ViewFunctionParameters {
        .Axis = ScrollAxis::Block,
        .Insets = {css::keywords::Auto {}},
      }};
    }

    return this->operator()(state, *viewValue);
  }

  ViewFunction CSSValueConversion<ViewFunction>::operator()(BuilderState &state,
                                                            const css::ViewValue &value) noexcept
  {
    auto axis = [&]
    {
      if (const auto &axisValue = value.Axis())
      {
        return FromValueId<ScrollAxis>(axisValue->ValueId());
      }
      return ScrollAxis::Block;
    }();

    auto startInset = [&]
    {
      if (const auto &startInsetValue = value.StartInset())
      {
        return ConvertCSSValueToStyle<ViewTimelineInsetItem::Length>(state, *startInsetValue);
      }
      return ViewTimelineInsetItem::Length {css::keywords::Auto {}};
    }();

    auto endInset = [&]
    {
      if (const auto &endInsetValue = value.EndInset())
      {
        return ConvertCSSValueToStyle<ViewTimelineInsetItem::Length>(state, *endInsetValue);
      }
      return startInset;
    }();

    return ViewFunction {ViewFunctionParameters {
      .Axis = axis,
      .Insets = {krys::move(startInset), krys::move(endInset)},
    }};
  }

  Ref<css::Value> CSSValueCreation<ViewFunction>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                             const ViewFunction &value) noexcept
  {
    return css::ViewValue::Create(CreateCSSValue(pool, style, value->Axis),
                                  CreateCSSValue(pool, style, value->Insets.Start()),
                                  CreateCSSValue(pool, style, value->Insets.End()));
  }

  // MARK: - Serialization

  void Serialiser<ViewFunctionParameters>::operator()(css::CSSOMString &builder,
                                                      const css::SerialisationContext &context,
                                                      const RenderStyle &style,
                                                      const ViewFunctionParameters &value) noexcept
  {
    bool needsSpace = false;
    if (value.Axis != ScrollAxis::Block)
    {
      Serialise(builder, context, style, value.Axis);
      needsSpace = true;
    }

    if (value.Insets.Start() == value.Insets.End())
    {
      if (value.Insets.Start().IsAuto())
      {
        return;
      }

      if (needsSpace)
      {
        builder.push_back(u8' ');
      }
      Serialise(builder, context, style, value.Insets.Start());
    }
    else
    {
      if (needsSpace)
      {
        builder.push_back(u8' ');
      }

      Serialise(builder, context, style, value.Insets.Start());
      builder.push_back(u8' ');
      Serialise(builder, context, style, value.Insets.End());
    }
  }
}
