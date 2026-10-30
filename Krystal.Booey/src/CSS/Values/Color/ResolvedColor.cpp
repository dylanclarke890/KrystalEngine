#include "Krystal.Booey/CSS/Values/Color/ResolvedColor.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  void Serialiser<ResolvedColor>::operator()(StringBuilder &builder, const SerialisationContext &,
                                             const ResolvedColor &value) noexcept
  {
    builder.Append(boo::SerialisationForCSS(value.Value));
  }
}
