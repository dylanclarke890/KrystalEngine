#include "Krystal.Booey/Style/Values/Text/TextAlignLast.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"

namespace krys::boo::style
{
  TextAlignLast CSSValueConversion<TextAlignLast>::operator()(BuilderState &state,
                                                              const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return TextAlignLast::Auto;
    }

    if (!primitiveValue->IsValueId())
    {
      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return TextAlignLast::Auto;
    }

    if (primitiveValue->ValueId() == css::ValueId::MatchParent)
    {
      auto &parentStyle = state.ParentStyle();

      if (parentStyle.TextAlignLast() == TextAlignLast::Start)
      {
        return parentStyle.GetWritingMode().IsBidiLTR() ? TextAlignLast::Left : TextAlignLast::Right;
      }
      if (parentStyle.TextAlignLast() == TextAlignLast::End)
      {
        return parentStyle.GetWritingMode().IsBidiLTR() ? TextAlignLast::Right : TextAlignLast::Left;
      }
      return parentStyle.TextAlignLast();
    }

    return FromCSSValue<TextAlignLast>(value);
  }
}
