#pragma once

#include "Krystal.Booey/DOM/Dicts/MutationObserverInit.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class MutationObserver;

  /// @brief An observer and the options it observes a node with, kept in the node's registered observer list.
  /// @see https://dom.spec.whatwg.org/#registered-observer
  class RegisteredObserver : public RefCounted<RegisteredObserver>
  {
  protected:
    /// @see https://dom.spec.whatwg.org/#registered-observer-observer
    Ref<MutationObserver> _observer;
    /// @see https://dom.spec.whatwg.org/#registered-observer-options
    MutationObserverInit _options;

  public:
    RegisteredObserver(Ref<MutationObserver> &&observer, const MutationObserverInit &options) noexcept
        : _observer(krys::move(observer)), _options(options)
    {
    }

    KRYS_NODISCARD Ref<MutationObserver> Observer() const noexcept
    {
      return _observer;
    }

    KRYS_NODISCARD MutationObserverInit Options() const noexcept
    {
      return _options;
    }

    void Options(const MutationObserverInit &options) noexcept
    {
      _options = options;
    }
  };
}