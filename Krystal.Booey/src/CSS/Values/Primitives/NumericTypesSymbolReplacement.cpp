#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSymbolReplacement.hpp"
#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::css
{
  // MARK: - Symbol replacement

  Number<> ReplaceSymbol(Symbol symbol, const SymbolTable &symbolTable) noexcept
  {
    auto result = symbolTable.Get(symbol.Value);

    // We should only get here if the symbol was previously looked up in the symbol table.
    krys_assert(result);
    return NumberRaw<> {result->Value};
  }
}
