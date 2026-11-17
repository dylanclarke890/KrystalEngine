#include "Krystal.Booey/CSS/Media/MediaQueryListEvent.hpp"

namespace krys::boo::css
{
  MediaQueryListEvent::MediaQueryListEvent(CSSOMStringAtom type,
                                           const MediaQueryListEventInit &eventInitDict) noexcept
      : Event(krys::move(type), eventInitDict), _media(eventInitDict.Media), _matches(eventInitDict.Matches)
  {
  }
}
