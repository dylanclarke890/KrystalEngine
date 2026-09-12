#pragma once

#include "Krystal.Booey/DOM/Internals/MutationObserver/RegisteredObserver.hpp"

namespace krys::boo::dom
{
  class TransientRegisteredObserver : public RegisteredObserver
  {
  private:
    Ref<RegisteredObserver> _source;

  public:
    TransientRegisteredObserver(Ref<MutationObserver> &&observer, const MutationObserverInit &options,
                                Ref<RegisteredObserver> &&source) noexcept
        : RegisteredObserver(krys::move(observer), options), _source(krys::move(source))
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