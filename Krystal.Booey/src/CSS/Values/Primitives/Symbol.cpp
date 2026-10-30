#include "Krystal.Booey/CSS/Values/Primitives/Symbol.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  void Serialiser<SymbolRaw>::operator()(StringBuilder &builder, const SerialisationContext &,
                                         const SymbolRaw &value) noexcept
  {
    builder.Append(ToLower(value.Value));
  }

  void Serialiser<Symbol>::operator()(StringBuilder &builder, const SerialisationContext &,
                                      const Symbol &value) noexcept
  {
    builder.Append(ToLower(value.Value));
  }
}
