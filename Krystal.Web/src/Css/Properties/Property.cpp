#include "Krystal.Web/Css/Properties/Property.hpp"
#include "Krystal.Web/Css/Properties/PropertyShorthand.hpp"

namespace krys::web::css
{
  PropertyId PropertyMetadata::ShorthandId() const noexcept
  {
    if (!Flags.contains(PropertyFlag::SetFromShorthand))
    {
      return PropertyId::Invalid;
    }
    auto shorthands = MatchingShorthandsForLonghand(Id);
    krys_debug_assert(ShorthandIndex < shorthands.size());
    return shorthands[ShorthandIndex].Id();
  }
}
