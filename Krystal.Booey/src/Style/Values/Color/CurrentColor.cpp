#include "Krystal.Booey/Style/Values/Color/CurrentColor.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // MARK: - Serialisation

  void SerialisationForCSSTokenization(StringBuilder &builder, const css::SerialisationContext &,
                                       const CurrentColor &) noexcept
  {
    builder.Append(u8"currentcolor");
  }

  css::CSSOMString SerialisationForCSSTokenization(const css::SerialisationContext &,
                                                   const CurrentColor &) noexcept
  {
    return u8"currentcolor";
  }
}
