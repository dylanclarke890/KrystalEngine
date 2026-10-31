#pragma once

#include "Krystal.Booey/DOM/Dicts/CustomEventInit.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#customevent
  template <typename T>
  class CustomEvent : public Event
  {
  private:
    Maybe<T> _detail;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = CustomEventInit<T>;

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
      // 1. If this's dispatch flag is set, then return.
      if (_dispatched)
      {
        return;
      }

      // 2. Initialize this with type, bubbles, and cancelable.
      Initialize(*this, type, bubbles, cancelable);

      // 3. Set this's detail attribute to detail.
      _detail = detail;
    }

#pragma endregion
  };
}