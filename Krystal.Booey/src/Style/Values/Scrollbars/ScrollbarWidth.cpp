#include "Krystal.Booey/Style/Values/Scrollbars/ScrollbarWidth.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  ScrollbarWidth CSSValueConversion<ScrollbarWidth>::operator()(BuilderState &state,
                                                                const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return ScrollbarWidth::Auto;
    }

    switch (primitiveValue->ValueId())
    {
      case css::ValueId::Auto: return ScrollbarWidth::Auto;
      case css::ValueId::Thin: return ScrollbarWidth::Thin;
      case css::ValueId::None: return ScrollbarWidth::None;
      default:                 state.SetCurrentPropertyInvalidAtComputedValueTime(); return ScrollbarWidth::Auto;
    }
  }
}
