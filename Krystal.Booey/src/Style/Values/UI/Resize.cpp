#include "Krystal.Booey/Style/Values/UI/Resize.hpp"

#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"

namespace krys::boo::style
{
  Resize CSSValueConversion<Resize>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return Resize::None;
    }

    if (!primitiveValue->IsValueId())
    {
      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return Resize::None;
    }

    if (primitiveValue->ValueId() == css::ValueId::InternalTextareaAuto)
    {
      return state.GetDocument().GetSettings().TextAreasAreResizable ? Resize::Both : Resize::None;
    }

    return FromCSSValue<Resize>(value);
  }
}
