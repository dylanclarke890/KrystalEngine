#pragma once

#include "Krystal.Booey/DOM/Dicts/EventInit.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#pagetransitioneventinit
  struct PageTransitionEventInit : dom::EventInit
  {
    bool Persisted {false};
  };

  /// @brief `pageshow` and `pagehide`: a document is shown or hidden as its frame navigates.
  /// @see https://html.spec.whatwg.org/#pagetransitionevent
  class PageTransitionEvent final : public dom::Event
  {
    friend class dom::EventFactory;
    using dictionary_type = PageTransitionEventInit;

  private:
    bool _persisted {false};

  public:
    /// @see https://html.spec.whatwg.org/#dom-pagetransitionevent-pagetransitionevent
    PageTransitionEvent(dom::DOMStringAtom type, const PageTransitionEventInit &eventInitDict = {}) noexcept;

    /// @brief A trusted page transition event.
    KRYS_NODISCARD static Ref<PageTransitionEvent> Create(dom::DOMStringAtom type, bool persisted) noexcept;

    /// @brief Whether the document comes from, or goes into, the back/forward cache.
    /// @see https://html.spec.whatwg.org/#dom-pagetransitionevent-persisted
    KRYS_NODISCARD bool Persisted() const noexcept
    {
      return _persisted;
    }

    KRYS_NODISCARD bool IsPageTransitionEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::PageTransitionEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsPageTransitionEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
