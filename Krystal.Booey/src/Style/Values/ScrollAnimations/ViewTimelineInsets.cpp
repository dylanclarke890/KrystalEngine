#include "Krystal.Booey/Style/Values/ScrollAnimations/ViewTimelineInsets.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::style
{
  const ViewTimelineInsetItem &ViewTimelineInsetDefaulter::operator()() const noexcept
  {
    static NeverDestroyed<ViewTimelineInsetItem> staticValue {css::keywords::Auto {}};
    return staticValue.get();
  }

  // MARK: - Conversion

  ViewTimelineInsetItem
    CSSValueConversion<ViewTimelineInsetItem>::operator()(BuilderState &state,
                                                          const css::PrimitiveValue &value) noexcept
  {
    return ConvertCSSValueToStyle<ViewTimelineInsetItem::Length>(state, value);
  }

  ViewTimelineInsetItem
    CSSValueConversion<ViewTimelineInsetItem>::operator()(BuilderState &state,
                                                          const css::ValuePair &value) noexcept
  {
    return {
      ConvertCSSValueToStyle<ViewTimelineInsetItem::Length>(state, value.First()),
      ConvertCSSValueToStyle<ViewTimelineInsetItem::Length>(state, value.Second()),
    };
  }

  ViewTimelineInsetItem
    CSSValueConversion<ViewTimelineInsetItem>::operator()(BuilderState &state,
                                                          const css::Value &value) noexcept
  {
    if (const auto *pair = DynamicDowncast<css::ValuePair>(value))
    {
      return this->operator()(state, *pair);
    }

    const auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Auto {};
    }

    return this->operator()(state, *primitiveValue);
  }

  ViewTimelineInsets CSSValueConversion<ViewTimelineInsets>::operator()(BuilderState &state,
                                                                        const css::Value &value) noexcept
  {
    if (const auto *pair = DynamicDowncast<css::ValuePair>(value))
    {
      return {ConvertCSSValueToStyle<ViewTimelineInsetItem>(state, *pair)};
    }

    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      return {ConvertCSSValueToStyle<ViewTimelineInsetItem>(state, *primitiveValue)};
    }

    auto list = RequiredListDowncast<css::ValueList, css::Value>(state, value);
    if (!list)
    {
      return css::keywords::Auto {};
    }

    return ViewTimelineInsetList::Map(
      *list, [&](const auto &element) -> ViewTimelineInsetItem
      { return ConvertCSSValueToStyle<ViewTimelineInsetItem>(state, element); });
  }
}
