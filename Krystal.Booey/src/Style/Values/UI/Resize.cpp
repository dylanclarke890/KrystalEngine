#include "Krystal.Booey/Style/Values/UI/Resize.hpp"

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
      // FIXME: Read `textAreasAreResizable` from the document's settings once `Settings` is ported; until
      // then this uses its default (false).
      return Resize::None;
    }

    return FromCSSValue<Resize>(value);
  }
}
