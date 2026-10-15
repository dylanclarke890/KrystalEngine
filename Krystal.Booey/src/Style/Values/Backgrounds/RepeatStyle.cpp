#include "Krystal.Booey/Style/Values/Backgrounds/RepeatStyle.hpp"

#include "Krystal.Booey/CSS/Values/BackgroundRepeatValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  RepeatStyle CSSValueConversion<RepeatStyle>::operator()(BuilderState &state,
                                                          const css::Value &value) noexcept
  {
    if (auto *repeatValue = DynamicDowncast<css::BackgroundRepeatValue>(value))
    {
      return RepeatStyle {.Values {
        FromValueId<FillRepeat>(repeatValue->XValue()),
        FromValueId<FillRepeat>(repeatValue->YValue()),
      }};
    }

    // Values coming from CSS Typed OM may not have been converted to a css::BackgroundRepeatValue.
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return RepeatStyle {.Values {
        FillRepeat::Repeat,
        FillRepeat::Repeat,
      }};
    }

    if (primitiveValue->ValueId() == css::ValueId::RepeatX)
    {
      return RepeatStyle {.Values {
        FillRepeat::Repeat,
        FillRepeat::NoRepeat,
      }};
    }
    if (primitiveValue->ValueId() == css::ValueId::RepeatY)
    {
      return RepeatStyle {.Values {
        FillRepeat::NoRepeat,
        FillRepeat::Repeat,
      }};
    }

    auto repeat = ConvertCSSValueToStyle<FillRepeat>(state, *primitiveValue);
    return RepeatStyle {.Values {
      repeat,
      repeat,
    }};
  }
}
