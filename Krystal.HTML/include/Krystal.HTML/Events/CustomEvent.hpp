#pragma once

#include "Krystal.HTML/Events/CustomEventInit.hpp"
#include "Krystal.HTML/Events/Event.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  template <typename T>
  class CustomEvent : public Event
  {
  private:
    Maybe<T> _detail;

  public:
#pragma region CustomEvent - https://dom.spec.whatwg.org/#customevent

    CustomEvent(const CustomEventInit<T> &eventInitDict = {}, DOMStringAtom type = {}) noexcept
        : Event(type, eventInitDict), _detail(eventInitDict.Detail)
    {
    }

    KRYS_NODISCARD Maybe<T> &Detail() noexcept
    {
      return _detail;
    }

    KRYS_NODISCARD const Maybe<T> &Detail() const noexcept
    {
      return _detail;
    }

    void Detail(T &&detail) noexcept
    {
      _detail = std::move(detail);
    }

    void InitCustomEvent(DOMStringAtom type, bool bubbles = false, bool cancelable = false,
                         Maybe<T> detail = std::nullopt) noexcept
    {
      if (_dispatched)
      {
        return;
      }

      InitEvent(type, bubbles, cancelable);

      _detail = detail;
    }

#pragma endregion
  };
}