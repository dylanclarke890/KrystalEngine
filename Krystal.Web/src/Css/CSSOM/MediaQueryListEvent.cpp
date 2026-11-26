#include "Krystal.Web/Css/CSSOM/MediaQueryListEvent.hpp"

namespace krys::web
{
  MediaQueryListEvent::MediaQueryListEvent(CSSOMStringAtom type,
                                           const MediaQueryListEventInit &eventInitDict) noexcept
      : Event(krys::move(type), eventInitDict), _media(eventInitDict.Media), _matches(eventInitDict.Matches)
  {
  }
}
