#pragma once

#include "Krystal.Booey/DOM/Internals/MutationObserver/RegisteredObserver.hpp"

namespace krys::boo::dom
{
  /// @brief A registered observer added to a node removed from under an observed subtree, so its observer
  /// sees what happens to the node until the observer is next notified.
  /// @see https://dom.spec.whatwg.org/#transient-registered-observer
  class TransientRegisteredObserver : public RegisteredObserver
  {
  private:
    /// @see https://dom.spec.whatwg.org/#transient-registered-observer-source
    Ref<RegisteredObserver> _source;

  public:
    TransientRegisteredObserver(Ref<MutationObserver> &&observer, const MutationObserverInit &options,
                                Ref<RegisteredObserver> &&source) noexcept
        : RegisteredObserver(krys::move(observer), options), _source(krys::move(source))
    {
    }

    KRYS_NODISCARD Ref<RegisteredObserver> Source() const noexcept
    {
      return _source;
    }
  };
}