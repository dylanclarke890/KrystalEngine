#pragma once

#include "Krystal.Booey/DOM/Types/Exception.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class NavigationHistoryEntry;

  /// @brief The outcome of a navigation the Navigation API started, as script would wait on its promise: it's
  /// fulfilled with the entry the navigation committed to (none for a transition), or rejected with an
  /// exception, once. Callbacks given before or after it settles run as microtasks once it has.
  class NavigationPromise final : public RefCounted<NavigationPromise>
  {
    KRYS_NON_COPY_MOVABLE(NavigationPromise);

  public:
    enum class Status : uint8
    {
      Pending,
      Fulfilled,
      Rejected
    };

    using FulfilledCallback = Func<void(NavigationHistoryEntry *entry)>;
    using RejectedCallback = Func<void(const dom::Exception &exception)>;

  private:
    Status _status {Status::Pending};
    RefPtr<NavigationHistoryEntry> _entry;
    Maybe<dom::Exception> _exception;
    List<Pair<FulfilledCallback, RejectedCallback>> _callbacks;

  public:
    NavigationPromise() noexcept;
    ~NavigationPromise() noexcept;

    KRYS_NODISCARD static Ref<NavigationPromise> Create() noexcept
    {
      return CreateRef<NavigationPromise>();
    }

    KRYS_NODISCARD Status GetStatus() const noexcept
    {
      return _status;
    }

    /// @brief The entry the promise was fulfilled with, if any.
    KRYS_NODISCARD NavigationHistoryEntry *Entry() const noexcept;

    /// @brief The exception the promise was rejected with, if it was.
    KRYS_NODISCARD const Maybe<dom::Exception> &GetException() const noexcept
    {
      return _exception;
    }

    /// @brief Calls @p onFulfilled or @p onRejected (either may be null) as a microtask once the promise
    /// settles.
    void Then(FulfilledCallback &&onFulfilled, RejectedCallback &&onRejected = nullptr) noexcept;

    void Resolve(NavigationHistoryEntry *entry = nullptr) noexcept;
    void Reject(dom::Exception &&exception) noexcept;

  private:
    void Settle() noexcept;
  };

  /// @brief The promises a Navigation API method returns: when the navigation commits, and when it finishes.
  /// @see https://html.spec.whatwg.org/#navigationresult
  struct NavigationResult
  {
    Ref<NavigationPromise> Committed;
    Ref<NavigationPromise> Finished;
  };
}
