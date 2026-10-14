#include "Krystal.Booey/Style/Values/Overflow/BlockEllipsis.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  BlockEllipsis CSSValueConversion<BlockEllipsis>::operator()(BuilderState &state,
                                                              const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::None {};
    }

    switch (primitiveValue->ValueId())
    {
      case css::ValueId::None: return css::keywords::None {};
      case css::ValueId::Auto: return css::keywords::Auto {};
      default:                 break;
    }

    return css::CSSOMStringAtom {primitiveValue->StringValue()};
  }
}
