#include "Krystal.Booey/CSS/CSSOM/MediaQueryListEvent.hpp"

namespace krys::boo
{
  MediaQueryListEvent::MediaQueryListEvent(CSSOMStringAtom type,
                                           const MediaQueryListEventInit &eventInitDict) noexcept
      : Event(krys::move(type), eventInitDict), _media(eventInitDict.Media), _matches(eventInitDict.Matches)
  {
  }
}
