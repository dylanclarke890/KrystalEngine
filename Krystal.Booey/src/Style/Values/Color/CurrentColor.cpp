#include "Krystal.Booey/Style/Values/Color/CurrentColor.hpp"

namespace krys::boo::style
{
  // MARK: - Serialisation

  void SerialisationForCSSTokenization(css::CSSOMString &builder, const css::SerialisationContext &,
                                       const CurrentColor &) noexcept
  {
    builder.append(u8"currentcolor");
  }

  css::CSSOMString SerialisationForCSSTokenization(const css::SerialisationContext &,
                                                   const CurrentColor &) noexcept
  {
    return u8"currentcolor";
  }
}
