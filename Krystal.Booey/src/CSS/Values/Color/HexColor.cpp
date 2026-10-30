#include "Krystal.Booey/CSS/Values/Color/HexColor.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  // MARK: - Serialise

  void Serialiser<HexColor>::operator()(StringBuilder &builder, const SerialisationContext &,
                                        const HexColor &value) noexcept
  {
    builder.Append(boo::SerialisationForCSS(boo::Color {value.Value}));
  }
}
