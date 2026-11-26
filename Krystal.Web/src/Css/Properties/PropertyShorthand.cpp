#include "Krystal.Web/Css/Properties/PropertyShorthand.hpp"
#include "Krystal.Core/Containers/Array.hpp"
#include <algorithm>

namespace krys::web::css
{
  PropertyShorthand TransitionShorthandForParsing() noexcept
  {
    constexpr static Array<PropertyId, 5uz> longhands {
      PropertyId::TransitionDuration, PropertyId::TransitionTimingFunction, PropertyId::TransitionDelay,
      PropertyId::TransitionBehavior, PropertyId::TransitionProperty,
    };
    return PropertyShorthand(PropertyId::Transition, Span {longhands});
  }

  uint8 IndexOfShorthandForLonghand(PropertyId shorthandId, const PropertyShorthandList &shorthands) noexcept
  {
    auto found = std::ranges::find(shorthands, shorthandId, &PropertyShorthand::Id);
    krys_debug_assert(found != shorthands.end());
    return static_cast<uint8>(found - shorthands.begin());
  }
}
