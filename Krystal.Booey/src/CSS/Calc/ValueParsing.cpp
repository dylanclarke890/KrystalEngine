#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Calc/TreeParser.hpp"
#include "Krystal.Booey/CSS/Calc/TreeSimplification.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"

namespace krys::boo::css::calc
{
  RefPtr<Value> Value::Parse(TokenRange &tokens, PropertyParserState &state, css::Category category,
                             css::Range range, AllowedSymbols symbolsAllowed,
                             PropertyParserOptions propertyOptions) noexcept
  {
    auto parserOptions = ParserOptions {
      .Category = category,
      .Range = range,
      .AllowedSymbols = krys::move(symbolsAllowed),
      .PropertyOptions = propertyOptions,
    };
    auto simplificationOptions = SimplificationOptions {
      .Category = category,
      .Range = range,
      .ConversionData = null,
      .SymbolTable = {},
      .AllowZeroValueLengthRemovalFromSum = false,
    };

    auto tree = ParseAndSimplify(tokens, state, parserOptions, simplificationOptions);
    if (!tree)
    {
      return nullptr;
    }

    return AdoptRefPtr(new Value(category, range, krys::move(*tree)));
  }
}
