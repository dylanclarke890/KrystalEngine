#pragma once

#include "Krystal.HTML/MutationObserver/MutationObserverInit.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class MutationObserver;

  class RegisteredObserver : public RefCounted<RegisteredObserver>
  {
  private:
    Ref<MutationObserver> _observer;
    MutationObserverInit _options;

  public:
    RegisteredObserver(Ref<MutationObserver> &&observer, const MutationObserverInit &options) noexcept
        : _observer(Krys::Move(observer)), _options(options)
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

    void SetOptions(const MutationObserverInit &options) noexcept
    {
      _options = options;
    }
  };
}