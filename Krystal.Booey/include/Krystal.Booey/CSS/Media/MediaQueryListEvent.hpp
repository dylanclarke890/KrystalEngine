#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Dicts/EventInit.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::css
{
  class MediaQueryList;

  /// @see https://drafts.csswg.org/cssom-view/#dictdef-mediaquerylisteventinit
  struct MediaQueryListEventInit : public dom::EventInit
  {
    CSSOMString Media;
    bool Matches {false};
  };

  /// @brief Fired at a `MediaQueryList` when its media query starts or stops matching.
  /// @see https://drafts.csswg.org/cssom-view/#mediaquerylistevent
  class MediaQueryListEvent final : public dom::Event
  {
    friend class MediaQueryList;

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
