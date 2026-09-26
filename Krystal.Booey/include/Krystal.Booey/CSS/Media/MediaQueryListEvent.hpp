#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Event.hpp"

namespace krys::boo::css
{
  struct MediaQueryListEventInit : public dom::EventInit
  {
    CSSOMString Media;
    bool Matches {false};
  };

  class MediaQueryListEvent final : public dom::Event
  {
  private:
    CSSOMString _media;
    bool _matches;

    MediaQueryListEvent(const CSSOMStringAtom &type, const CSSOMString &media, bool matches) noexcept;

    MediaQueryListEvent(const CSSOMStringAtom &type, const MediaQueryListEventInit &init,
                        bool isTrusted) noexcept;

  public:
    KRYS_NODISCARD static Ref<MediaQueryListEvent> Create(const CSSOMStringAtom &type,
                                                          const CSSOMString &media, bool matches) noexcept;

    KRYS_NODISCARD static Ref<MediaQueryListEvent> Create(const CSSOMStringAtom &type,
                                                          const MediaQueryListEventInit &init,
                                                          bool isTrusted = false) noexcept;

    KRYS_NODISCARD const CSSOMString &Media() const noexcept
    {
      return _media;
    }

    KRYS_NODISCARD bool Matches() const noexcept
    {
      return _matches;
    }
  };
}