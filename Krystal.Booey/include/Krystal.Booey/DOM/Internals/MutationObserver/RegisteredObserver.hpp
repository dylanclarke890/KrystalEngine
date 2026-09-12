#pragma once

#include "Krystal.Booey/DOM/Dicts/MutationObserverInit.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class MutationObserver;

  class RegisteredObserver : public RefCounted<RegisteredObserver>
  {
  protected:
    Ref<MutationObserver> _observer;
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