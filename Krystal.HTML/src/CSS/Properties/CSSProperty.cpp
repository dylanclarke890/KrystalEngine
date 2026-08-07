#include "Krystal.HTML/CSS/Properties/CSSProperty.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyShorthand.hpp"

namespace Krys::HTML
{
  CSSPropertyId CSSPropertyMetadata::ShorthandId() const noexcept
  {
    if (!HasFlag(Flags, CSSPropertyFlag::SetFromShorthand))
    {
      return CSSPropertyId::Invalid;
    }

    auto shorthands = MatchingShorthandsForLonghand(Id);
    assert(shorthands.size() && ShorthandIndex < shorthands.size());

    return shorthands[ShorthandIndex].ShorthandId;
  }
}