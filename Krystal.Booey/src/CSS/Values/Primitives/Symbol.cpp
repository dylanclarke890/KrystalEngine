#include "Krystal.Booey/CSS/Values/Primitives/Symbol.hpp"

namespace krys::boo::css
{
  void Serialiser<SymbolRaw>::operator()(CSSOMString &builder, const SerialisationContext &,
                                         const SymbolRaw &value) noexcept
  {
    builder.append(ToLower(value.Value));
  }

  void Serialiser<Symbol>::operator()(CSSOMString &builder, const SerialisationContext &,
                                      const Symbol &value) noexcept
  {
    builder.append(ToLower(value.Value));
  }
}
