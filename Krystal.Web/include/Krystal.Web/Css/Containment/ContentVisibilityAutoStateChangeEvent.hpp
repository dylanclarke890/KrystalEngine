#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"

namespace krys::web
{
  /// @see https://drafts.csswg.org/css-contain/#dictdef-contentvisibilityautostatechangeeventinit
  struct ContentVisibilityAutoStateChangeEventInit : EventInit
  {
    bool Skipped {false};
  };

  /// @brief Fired at an element with `content-visibility: auto` when it starts or stops skipping its
  /// contents.
  /// @see https://drafts.csswg.org/css-contain/#content-visibility-auto-state-change
  class ContentVisibilityAutoStateChangeEvent final : public Event
  {
  private:
    bool _skipped;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = ContentVisibilityAutoStateChangeEventInit;

    /// @see https://drafts.csswg.org/css-contain/#dom-contentvisibilityautostatechangeevent-contentvisibilityautostatechangeevent
    ContentVisibilityAutoStateChangeEvent(
      DOMStringAtom type, const ContentVisibilityAutoStateChangeEventInit &eventInitDict = {}) noexcept;

    /// @brief An event of type @p type; trusted if @p isTrusted.
    KRYS_NODISCARD static Ref<ContentVisibilityAutoStateChangeEvent>
      Create(const DOMStringAtom &type, const ContentVisibilityAutoStateChangeEventInit &init,
             bool isTrusted = false) noexcept;

    /// @brief Whether the element now skips its contents.
    /// @see https://drafts.csswg.org/css-contain/#dom-contentvisibilityautostatechangeevent-skipped
    KRYS_NODISCARD bool Skipped() const noexcept
    {
      return _skipped;
    }

    KRYS_NODISCARD bool IsContentVisibilityAutoStateChangeEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::ContentVisibilityAutoStateChangeEvent)
  KRYS_NODISCARD static bool IsType(const krys::web::Event &event) noexcept
  {
    return event.IsContentVisibilityAutoStateChangeEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
