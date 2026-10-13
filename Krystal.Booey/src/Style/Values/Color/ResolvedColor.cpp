#include "Krystal.Booey/Style/Values/Color/ResolvedColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/ResolvedColor.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion.

  Color ToStyleColor(const css::ResolvedColor &unresolved, ColorResolutionState &) noexcept
  {
    return Color {ResolvedColor {unresolved.Value}};
  }

  // MARK: - Serialisation

  void SerialisationForCSSTokenization(css::CSSOMString &builder, const css::SerialisationContext &,
                                       const ResolvedColor &absoluteColor) noexcept
  {
    builder.append(boo::SerialisationForCSS(absoluteColor.Value));
  }

  css::CSSOMString SerialisationForCSSTokenization(const css::SerialisationContext &,
                                                   const ResolvedColor &absoluteColor) noexcept
  {
    return boo::SerialisationForCSS(absoluteColor.Value);
  }
}
