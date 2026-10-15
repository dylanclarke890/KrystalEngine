#include "Krystal.Booey/Style/Values/ScrollAnimations/ProgressTimelineName.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  ProgressTimelineName CSSValueConversion<ProgressTimelineName>::operator()(BuilderState &state,
                                                                            const css::Value &value) noexcept
  {
    // FIXME: This should really be checking for `value.valueId() == css::ValueId::None` with
    // `ProgressTimelineName` modeling that state - https://bugs.webkit.org/show_bug.cgi?id=295467.

    const auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return ProgressTimelineName {css::CustomIdentifier {css::CSSOMStringAtom::Empty()}};
    }

    return ProgressTimelineName {
      css::CustomIdentifier {css::CSSOMStringAtom {primitiveValue->StringValue()}}};
  }
}
