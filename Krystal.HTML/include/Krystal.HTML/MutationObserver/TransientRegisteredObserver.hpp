#pragma once

#include "Krystal.Lib/Mixins/RefCounted.hpp"

namespace Krys::HTML
{
  class TransientRegisteredObserver : public RefCounted<TransientRegisteredObserver>
  {
  private:
    Ref<RegisteredObserver> _source;

  public:
    TransientRegisteredObserver(Ref<RegisteredObserver> source) noexcept : _source(Krys::Move(source))
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