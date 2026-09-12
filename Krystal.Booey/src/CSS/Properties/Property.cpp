#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/CSSPropertyShorthand.hpp"

namespace krys::boo::css
{
  CSSPropertyId PropertyMetadata::ShorthandId() const noexcept
  {
    if (!HasFlag(Flags, CSSPropertyFlag::SetFromShorthand))
    {
      return CSSPropertyId::Invalid;
    }

    auto shorthands = MatchingShorthandsForLonghand(Id);
    assert(shorthands.size() && ShorthandIndex < shorthands.size());

    return shorthands[ShorthandIndex].Id();
  }
}