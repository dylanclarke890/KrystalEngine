#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyShorthand.hpp"

namespace krys::boo::css
{
  PropertyId PropertyMetadata::ShorthandId() const noexcept
  {
    if (!HasFlag(Flags, PropertyFlag::SetFromShorthand))
    {
      return PropertyId::Invalid;
    }

    auto shorthands = MatchingShorthandsForLonghand(Id);
    krys_debug_assert(shorthands.size() && ShorthandIndex < shorthands.size());

    return shorthands[ShorthandIndex].Id();
  }
}