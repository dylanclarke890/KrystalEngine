#include "Krystal.Booey/Style/Values/Text/TextAlign.hpp"

#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"

namespace krys::boo::style
{
  TextAlign CSSValueConversion<TextAlign>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return TextAlign::Start;
    }

    if (!primitiveValue->IsValueId())
    {
      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return TextAlign::Start;
    }

    auto &parentStyle = state.ParentStyle();

    // User agents are expected to have a rule in their user agent stylesheet that matches 'th' elements that
    // have a parent node whose computed value for the 'text-align' property is its initial value, whose
    // declaration block consists of just a single declaration that sets the 'text-align' property to the
    // value 'center'. https://html.spec.whatwg.org/multipage/rendering.html#rendering
    if (primitiveValue->ValueId() == css::ValueId::InternalThCenter)
    {
      if (parentStyle.TextAlign() == TextAlign::Start)
      {
        return TextAlign::Center;
      }
      return parentStyle.TextAlign();
    }

    if (primitiveValue->ValueId() == css::ValueId::WebkitMatchParent
        || primitiveValue->ValueId() == css::ValueId::MatchParent)
    {
      const auto *element = state.GetElement();

      if (element && element == state.GetDocument().DocumentElement().get())
      {
        return TextAlign::Start;
      }
      if (parentStyle.TextAlign() == TextAlign::Start)
      {
        return parentStyle.GetWritingMode().IsBidiLTR() ? TextAlign::Left : TextAlign::Right;
      }
      if (parentStyle.TextAlign() == TextAlign::End)
      {
        return parentStyle.GetWritingMode().IsBidiLTR() ? TextAlign::Right : TextAlign::Left;
      }

      return parentStyle.TextAlign();
    }

    return FromCSSValue<TextAlign>(value);
  }
}
