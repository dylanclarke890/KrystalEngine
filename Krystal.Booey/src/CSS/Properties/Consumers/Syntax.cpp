#include "Krystal.Booey/CSS/Properties/Consumers/Syntax.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Parser/VariableParser.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<Value> ConsumeDeclarationValue(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // https://drafts.csswg.org/css-syntax-3/#typedef-declaration-value
    return VariableParser::ParseDeclarationValue(CSSOMStringAtom::Null(), tokens.ConsumeAll(), state.Context);
  }
}
