#include "Krystal.Booey/CSS/Values/Color/HexColor.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"

namespace krys::boo::css
{
  // MARK: - Serialise

  void Serialiser<HexColor>::operator()(CSSOMString &builder, const SerialisationContext &,
                                        const HexColor &value) noexcept
  {
    builder.append(boo::SerialisationForCSS(boo::Color {value.Value}));
  }
}
