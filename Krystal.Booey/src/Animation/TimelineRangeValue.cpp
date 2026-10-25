#include "Krystal.Booey/Animation/TimelineRangeValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Timeline.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/Values/Animations/SingleAnimationRange.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo
{
  RefPtr<css::Value> ConvertToCSSValue(TimelineRangeValue &&value, RefPtr<dom::Element> element,
                                       style::SingleAnimationRangeType type) noexcept
  {
    if (!element)
    {
      return {};
    }

    Ref document = ShareRef(element->NodeDocument());
    auto parserType = type == style::SingleAnimationRangeType::Start ? css::SingleAnimationRangeType::Start
                                                                     : css::SingleAnimationRangeType::End;

    return krys::SwitchOn(
      value,
      [&](dom::DOMString &rangeString) -> RefPtr<css::Value>
      {
        return css::PropertyParserHelpers::ParseSingleAnimationRange(
          rangeString, document->CssParserContext(), parserType);
      },
      [&](TimelineRangeOffset &rangeOffset) -> RefPtr<css::Value>
      {
        if (auto consumedRangeName = css::PropertyParserHelpers::ParseSingleAnimationRange(
              rangeOffset.RangeName, document->CssParserContext(), parserType))
        {
          if (RefPtr offset = rangeOffset.Offset)
          {
            if (auto offsetValue = offset->ToCSSValue())
            {
              return css::ValuePair::CreateNonCoalescing(Ref<css::Value> {krys::move(consumedRangeName)},
                                                         Ref<css::Value> {krys::move(offsetValue)});
            }
          }
          return consumedRangeName;
        }
        if (RefPtr offset = rangeOffset.Offset)
        {
          return offset->ToCSSValue();
        }
        return nullptr;
      },
      [&](RefPtr<css::CSSKeywordValue> &rangeKeyword) -> RefPtr<css::Value>
      { return rangeKeyword ? rangeKeyword->ToCSSValue() : nullptr; },
      [&](RefPtr<css::CSSNumericValue> &rangeValue) -> RefPtr<css::Value>
      { return rangeValue ? rangeValue->ToCSSValue() : nullptr; });
  }
}
