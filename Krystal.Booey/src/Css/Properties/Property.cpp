#include "Krystal.Booey/Css/Properties/Property.hpp"
#include "Krystal.Booey/Css/Properties/PropertyShorthand.hpp"

namespace krys::boo::css
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
