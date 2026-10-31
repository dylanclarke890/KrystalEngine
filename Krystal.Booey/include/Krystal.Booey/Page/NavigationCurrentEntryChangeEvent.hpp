#pragma once

#include "Krystal.Booey/DOM/Dicts/EventInit.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/Page/NavigationHistoryEntry.hpp"
#include "Krystal.Booey/Page/NavigationNavigationType.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/#navigationcurrententrychangeeventinit
  struct NavigationCurrentEntryChangeEventInit : dom::EventInit
  {
    Maybe<NavigationNavigationType> NavigationType;
    RefPtr<NavigationHistoryEntry> From;
  };

  /// @brief `currententrychange`: the window's `navigation.currentEntry` changed.
  /// @see https://html.spec.whatwg.org/#navigationcurrententrychangeevent
  class NavigationCurrentEntryChangeEvent final : public dom::Event
  {
  private:
    Maybe<NavigationNavigationType> _navigationType;
    RefPtr<NavigationHistoryEntry> _from;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = NavigationCurrentEntryChangeEventInit;

    /// @see
    /// https://html.spec.whatwg.org/#dom-navigationcurrententrychangeevent-navigationcurrententrychangeevent
    NavigationCurrentEntryChangeEvent(
      dom::DOMStringAtom type, const NavigationCurrentEntryChangeEventInit &eventInitDict = {}) noexcept;
    ~NavigationCurrentEntryChangeEvent() noexcept override;

    /// @brief A trusted `currententrychange`, from @p from by a navigation of @p navigationType (null for
    /// `updateCurrentEntry()`).
    KRYS_NODISCARD static Ref<NavigationCurrentEntryChangeEvent>
      Create(Maybe<NavigationNavigationType> navigationType, NavigationHistoryEntry &from) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-navigationcurrententrychangeevent-navigationtype
    KRYS_NODISCARD Maybe<NavigationNavigationType> NavigationType() const noexcept
    {
      return _navigationType;
    }

    /// @see https://html.spec.whatwg.org/#dom-navigationcurrententrychangeevent-from
    KRYS_NODISCARD NavigationHistoryEntry *From() const noexcept
    {
      return _from.get();
    }

    KRYS_NODISCARD bool IsNavigationCurrentEntryChangeEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::NavigationCurrentEntryChangeEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsNavigationCurrentEntryChangeEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
