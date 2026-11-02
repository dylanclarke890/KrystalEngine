#pragma once

#include "Krystal.Booey/Page/NavigationDestination.hpp"
#include "Krystal.Booey/Page/NavigationHistoryEntry.hpp"
#include "Krystal.Booey/Page/NavigationNavigationType.hpp"
#include "Krystal.Booey/Page/NavigationPromise.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief The navigation an intercepted `navigate` started, until its handlers are done
  /// (`navigation.transition`).
  /// @see https://html.spec.whatwg.org/#navigationtransition
  class NavigationTransition final : public RefCounted<NavigationTransition>
  {
    KRYS_NON_COPY_MOVABLE(NavigationTransition);

  private:
    NavigationNavigationType _navigationType;
    Ref<NavigationHistoryEntry> _from;
    Ref<NavigationDestination> _destination;
    Ref<NavigationPromise> _committed;
    Ref<NavigationPromise> _finished;

  public:
    NavigationTransition(NavigationNavigationType type, Ref<NavigationHistoryEntry> &&fromEntry,
                         Ref<NavigationDestination> &&destination) noexcept
        : _navigationType(type), _from(krys::move(fromEntry)), _destination(krys::move(destination)),
          _committed(NavigationPromise::Create()), _finished(NavigationPromise::Create())
    {
    }

    KRYS_NODISCARD static Ref<NavigationTransition> Create(NavigationNavigationType type,
                                                           Ref<NavigationHistoryEntry> &&fromEntry,
                                                           Ref<NavigationDestination> &&destination) noexcept
    {
      return CreateRef<NavigationTransition>(type, krys::move(fromEntry), krys::move(destination));
    }

    /// @see https://html.spec.whatwg.org/#dom-navigationtransition-navigationtype
    KRYS_NODISCARD NavigationNavigationType NavigationType() const noexcept
    {
      return _navigationType;
    }

    /// @see https://html.spec.whatwg.org/#dom-navigationtransition-from
    KRYS_NODISCARD NavigationHistoryEntry &From() const noexcept
    {
      return *_from;
    }

    /// @see https://html.spec.whatwg.org/#dom-navigationtransition-to
    KRYS_NODISCARD NavigationDestination &To() const noexcept
    {
      return *_destination;
    }

    /// @brief Fulfilled (with no entry) when the navigation commits; rejected if it's aborted before.
    /// @see https://html.spec.whatwg.org/#dom-navigationtransition-committed
    KRYS_NODISCARD NavigationPromise &Committed() const noexcept
    {
      return *_committed;
    }

    /// @brief Fulfilled (with no entry) when the handlers are done; rejected if one fails or the navigation
    /// is aborted.
    /// @see https://html.spec.whatwg.org/#dom-navigationtransition-finished
    KRYS_NODISCARD NavigationPromise &Finished() const noexcept
    {
      return *_finished;
    }

    void ResolveCommittedPromise() noexcept
    {
      _committed->Resolve();
    }

    void ResolvePromise() noexcept
    {
      _finished->Resolve();
    }

    /// @brief Rejects the committed promise, if it's not settled, and the finished promise.
    void RejectPromise(dom::Exception &&exception) noexcept
    {
      _committed->Reject(dom::Exception(exception));
      _finished->Reject(krys::move(exception));
    }
  };
}
