#include "Krystal.Booey/Page/NavigationPromise.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/NavigationHistoryEntry.hpp"

namespace krys::boo
{
  NavigationPromise::NavigationPromise() noexcept = default;

  NavigationPromise::~NavigationPromise() noexcept = default;

  NavigationHistoryEntry *NavigationPromise::Entry() const noexcept
  {
    return _entry.get();
  }

  void NavigationPromise::Then(FulfilledCallback &&onFulfilled, RejectedCallback &&onRejected) noexcept
  {
    _callbacks.emplace_back(krys::move(onFulfilled), krys::move(onRejected));
    if (_status != Status::Pending)
    {
      Settle();
    }
  }

  void NavigationPromise::Resolve(NavigationHistoryEntry *entry) noexcept
  {
    if (_status != Status::Pending)
    {
      return;
    }
    _status = Status::Fulfilled;
    _entry = ShareRefPtr(entry);
    Settle();
  }

  void NavigationPromise::Reject(dom::Exception &&exception) noexcept
  {
    if (_status != Status::Pending)
    {
      return;
    }
    _status = Status::Rejected;
    _exception = krys::move(exception);
    Settle();
  }

  void NavigationPromise::Settle() noexcept
  {
    // A promise's reactions run as microtasks.
    for (auto &[onFulfilled, onRejected] : std::exchange(_callbacks, {}))
    {
      html::EventLoop::Current().QueueMicrotask(
        [protectedThis = Ref<NavigationPromise>(ShareRef(*this)), onFulfilled = krys::move(onFulfilled),
         onRejected = krys::move(onRejected)]() mutable
        {
          if (protectedThis->_status == Status::Fulfilled && onFulfilled)
          {
            onFulfilled(protectedThis->_entry.get());
          }
          else if (protectedThis->_status == Status::Rejected && onRejected)
          {
            onRejected(*protectedThis->_exception);
          }
        });
    }
  }
}
