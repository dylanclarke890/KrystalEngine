#pragma once

#include "Krystal.HTML/DOM/Internals/MutationObserver/RegisteredObserver.hpp"

namespace Krys::HTML
{
  class TransientRegisteredObserver : public RegisteredObserver
  {
  private:
    Ref<RegisteredObserver> _source;

  public:
    TransientRegisteredObserver(Ref<MutationObserver> &&observer, const MutationObserverInit &options,
                                Ref<RegisteredObserver> &&source) noexcept
        : RegisteredObserver(Krys::Move(observer), options), _source(Krys::Move(source))
    {
    }

    KRYS_NODISCARD Ref<MutationObserver> Observer() const noexcept
    {
      return _source->Observer();
    }

    KRYS_NODISCARD MutationObserverInit Options() const noexcept
    {
      return _source->Options();
    }

    KRYS_NODISCARD Ref<RegisteredObserver> Source() const noexcept
    {
      return _source;
    }
  };
}