#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Web/Css/Types/CSSOMString.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"

namespace krys::web
{
  /// @see https://drafts.csswg.org/cssom-view/#dictdef-mediaquerylisteventinit
  struct MediaQueryListEventInit : public EventInit
  {
    CSSOMString Media;
    bool Matches {false};
  };

  /// @brief Fired at a `MediaQueryList` when its media query starts or stops matching.
  /// @see https://drafts.csswg.org/cssom-view/#mediaquerylistevent
  class MediaQueryListEvent final : public Event
  {
  private:
    CSSOMString _media;
    bool _matches {false};

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = MediaQueryListEventInit;

    /// @see https://drafts.csswg.org/cssom-view/#dom-mediaquerylistevent-mediaquerylistevent
    explicit MediaQueryListEvent(CSSOMStringAtom type,
                                 const MediaQueryListEventInit &eventInitDict = {}) noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-mediaquerylistevent-media
    KRYS_NODISCARD const CSSOMString &Media() const noexcept
    {
      return _media;
    }

    /// @see https://drafts.csswg.org/cssom-view/#dom-mediaquerylistevent-matches
    KRYS_NODISCARD bool Matches() const noexcept
    {
      return _matches;
    }
  };
}
