#include "Krystal.Booey/CSS/Values/Color/ResolvedColor.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"

namespace krys::boo::css
{
  void Serialiser<ResolvedColor>::operator()(CSSOMString &builder, const SerialisationContext &,
                                             const ResolvedColor &value) noexcept
  {
    builder.append(boo::SerialisationForCSS(value.Value));
  }
}
