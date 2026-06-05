#pragma once

#include "Krystal.HTML/DOM/Dicts/CustomEventInit.hpp"
#include "Krystal.HTML/DOM/Event.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#customevent
  template <typename T>
  class CustomEvent : public Event
  {
    friend class EventFactory;

    using dictionary_type = CustomEventInit<T>;

  private:
    Maybe<T> _detail;

  public:
#pragma region CustomEvent - https://dom.spec.whatwg.org/#customevent

    /// @see https://dom.spec.whatwg.org/#dom-customevent-customevent
    CustomEvent(DOMStringAtom type, const CustomEventInit<T> &eventInitDict = {}) noexcept
        : Event(type, eventInitDict), _detail(eventInitDict.Detail)
    {
    }

    /// @see https://dom.spec.whatwg.org/#dom-customevent-detail
    KRYS_NODISCARD Maybe<T> &Detail() noexcept
    {
      return _detail;
    }

    /// @see https://dom.spec.whatwg.org/#dom-customevent-detail
    KRYS_NODISCARD const Maybe<T> &Detail() const noexcept
    {
      return _detail;
    }

    /// @see https://dom.spec.whatwg.org/#dom-customevent-initcustomevent
    void InitCustomEvent(DOMStringAtom type, bool bubbles = false, bool cancelable = false,
                         Maybe<T> detail = std::nullopt) noexcept
    {
      if (_dispatched)
      {
        return;
      }

      Initialize(*this, type, bubbles, cancelable);

      _detail = detail;
    }

#pragma endregion
  };
}