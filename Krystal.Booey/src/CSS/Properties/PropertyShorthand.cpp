#include "Krystal.Booey/CSS/Properties/PropertyShorthand.hpp"
#include "Krystal.Core/Types/Array.hpp"

namespace krys::boo::css
{
  PropertyShorthand TransitionShorthandForParsing() noexcept
  {
    // Similar to animations, we have property after timing-function and delay after
    // duration.
    constexpr static Array<PropertyId, 5uz> transitionProperties = {{
      PropertyId::TransitionDuration,
      PropertyId::TransitionTimingFunction,
      PropertyId::TransitionDelay,
      PropertyId::TransitionBehavior,
      PropertyId::TransitionProperty,
    }};
    return PropertyShorthand(PropertyId::Transition, Span {transitionProperties});
  }

  uint32 IndexOfShorthandForLonghand(PropertyId shorthandId, const PropertyShorthandList &shorthands) noexcept
  {
    for (uint32 i = 0u, size = static_cast<uint32>(shorthands.size()); i < size; ++i)
    {
      if (shorthands[i].Id() == shorthandId)
      {
        return i;
      }
    }
    krys_debug_assert_msg(false, "The shorthand must be in the list.");
    return 0u;
  }
}
