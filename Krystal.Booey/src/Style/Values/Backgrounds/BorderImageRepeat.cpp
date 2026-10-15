#include "Krystal.Booey/Style/Values/Backgrounds/BorderImageRepeat.hpp"

#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  BorderImageRepeat CSSValueConversion<BorderImageRepeat>::operator()(BuilderState &state,
                                                                      const css::Value &value) noexcept
  {
    if (auto *pairValue = DynamicDowncast<css::ValuePair>(value))
    {
      return BorderImageRepeat {
        ConvertCSSValueToStyle<NinePieceImageRule>(state, pairValue->First()),
        ConvertCSSValueToStyle<NinePieceImageRule>(state, pairValue->Second()),
      };
    }

    // Values coming from CSS Typed OM may not have been converted to a css::ValuePair.
    return ConvertCSSValueToStyle<NinePieceImageRule>(state, value);
  }
}
