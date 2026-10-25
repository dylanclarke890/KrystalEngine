#pragma once

#include "Krystal.Booey/DOM/Dicts/TouchEventInit.hpp"
#include "Krystal.Booey/DOM/MouseRelatedEvent.hpp"
#include "Krystal.Booey/DOM/TouchList.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::dom
{
  /// @brief A change to the touches on the screen: `touchstart`, `touchmove`, `touchend` and `touchcancel`.
  /// @see https://w3c.github.io/touch-events/#touchevent-interface
  class TouchEvent final : public MouseRelatedEvent
  {
    friend class EventFactory;

    using dictionary_type = TouchEventInit;

  private:
    RefPtr<TouchList> _touches;
    RefPtr<TouchList> _targetTouches;
    RefPtr<TouchList> _changedTouches;

  public:
    /// @see https://w3c.github.io/touch-events/#dom-touchevent-touchevent
    TouchEvent(DOMStringAtom type, const TouchEventInit &initializer = {}) noexcept;

    ~TouchEvent() noexcept override;

    /// @brief A trusted event, which happened in @p view.
    KRYS_NODISCARD static Ref<TouchEvent> Create(TouchList *touches, TouchList *targetTouches,
                                                 TouchList *changedTouches, DOMStringAtom type,
                                                 LocalFrameView *view, const DoublePoint &globalLocation,
                                                 EnumSet<Modifier> modifiers) noexcept;

    /// @see https://w3c.github.io/touch-events/#dom-touchevent-touches
    KRYS_NODISCARD TouchList *Touches() const noexcept
    {
      return _touches.get();
    }

    /// @see https://w3c.github.io/touch-events/#dom-touchevent-targettouches
    KRYS_NODISCARD TouchList *TargetTouches() const noexcept
    {
      return _targetTouches.get();
    }

    /// @see https://w3c.github.io/touch-events/#dom-touchevent-changedtouches
    KRYS_NODISCARD TouchList *ChangedTouches() const noexcept
    {
      return _changedTouches.get();
    }

    void SetTouches(RefPtr<TouchList> &&touches) noexcept
    {
      _touches = krys::move(touches);
    }

    void SetTargetTouches(RefPtr<TouchList> &&targetTouches) noexcept
    {
      _targetTouches = krys::move(targetTouches);
    }

    void SetChangedTouches(RefPtr<TouchList> &&changedTouches) noexcept
    {
      _changedTouches = krys::move(changedTouches);
    }

    KRYS_NODISCARD bool IsTouchEvent() const noexcept override
    {
      return true;
    }

  private:
    TouchEvent(TouchList *touches, TouchList *targetTouches, TouchList *changedTouches, DOMStringAtom type,
               LocalFrameView *view, const DoublePoint &globalLocation, EnumSet<Modifier> modifiers) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::TouchEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsTouchEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
