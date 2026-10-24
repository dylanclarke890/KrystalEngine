#pragma once

#include "Krystal.Booey/Page/NavigationHistoryEntry.hpp"
#include "Krystal.Booey/Page/NavigationNavigationType.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief The navigation that last activated the window's document (`navigation.activation`): its type, the
  /// entry it went to, and the one it came from (if it was of the same origin).
  /// @see https://html.spec.whatwg.org/#navigationactivation
  class NavigationActivation final : public RefCounted<NavigationActivation>
  {
    KRYS_NON_COPY_MOVABLE(NavigationActivation);

  private:
    NavigationNavigationType _navigationType;
    Ref<NavigationHistoryEntry> _entry;
    RefPtr<NavigationHistoryEntry> _fromEntry;

  public:
    NavigationActivation(NavigationNavigationType type, Ref<NavigationHistoryEntry> &&entry,
                         RefPtr<NavigationHistoryEntry> &&fromEntry) noexcept
        : _navigationType(type), _entry(krys::move(entry)), _fromEntry(krys::move(fromEntry))
    {
    }

    KRYS_NODISCARD static Ref<NavigationActivation>
      Create(NavigationNavigationType type, Ref<NavigationHistoryEntry> &&entry,
             RefPtr<NavigationHistoryEntry> &&fromEntry) noexcept
    {
      return CreateRef<NavigationActivation>(type, krys::move(entry), krys::move(fromEntry));
    }

    /// @see https://html.spec.whatwg.org/#dom-navigationactivation-navigationtype
    KRYS_NODISCARD NavigationNavigationType NavigationType() const noexcept
    {
      return _navigationType;
    }

    /// @see https://html.spec.whatwg.org/#dom-navigationactivation-from
    KRYS_NODISCARD NavigationHistoryEntry *From() const noexcept
    {
      return _fromEntry.get();
    }

    /// @see https://html.spec.whatwg.org/#dom-navigationactivation-entry
    KRYS_NODISCARD NavigationHistoryEntry &Entry() const noexcept
    {
      return *_entry;
    }
  };
}
