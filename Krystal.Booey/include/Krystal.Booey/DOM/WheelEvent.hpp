#pragma once

#include "Krystal.Booey/DOM/Dicts/WheelEventInit.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/Platform/PlatformWheelEvent.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  /// @brief A turn of the mouse wheel or a similar device: `wheel`.
  /// @see https://w3c.github.io/uievents/#interface-wheelevent
  class WheelEvent final : public MouseEvent
  {
    friend class EventFactory;

    using dictionary_type = WheelEventInit;

  public:
    enum class IsCancelable : bool
    {
      No,
      Yes
    };

    /// @see https://w3c.github.io/uievents/#dom-wheelevent-dom_delta_pixel
    static constexpr uint32 DOM_DELTA_PIXEL = 0x00;
    /// @see https://w3c.github.io/uievents/#dom-wheelevent-dom_delta_line
    static constexpr uint32 DOM_DELTA_LINE = 0x01;
    /// @see https://w3c.github.io/uievents/#dom-wheelevent-dom_delta_page
    static constexpr uint32 DOM_DELTA_PAGE = 0x02;

  private:
    double _deltaX {0};
    double _deltaY {0};
    double _deltaZ {0};
    uint32 _deltaMode {DOM_DELTA_PIXEL};
    Maybe<PlatformWheelEvent> _underlyingPlatformEvent;

  public:
    /// @see https://w3c.github.io/uievents/#dom-wheelevent-wheelevent
    WheelEvent(DOMStringAtom type, const WheelEventInit &initializer = {}) noexcept;

    /// @brief A trusted `wheel` event from @p event, which happened in @p view.
    KRYS_NODISCARD static Ref<WheelEvent> Create(const PlatformWheelEvent &event, LocalFrameView *view,
                                                 IsCancelable isCancelable = IsCancelable::Yes) noexcept;

    KRYS_NODISCARD const Maybe<PlatformWheelEvent> &UnderlyingPlatformEvent() const noexcept
    {
      return _underlyingPlatformEvent;
    }

    /// @brief Positive when scrolling right.
    /// @see https://w3c.github.io/uievents/#dom-wheelevent-deltax
    KRYS_NODISCARD double DeltaX() const noexcept
    {
      return _deltaX;
    }
    /// @brief Positive when scrolling down.
    /// @see https://w3c.github.io/uievents/#dom-wheelevent-deltay
    KRYS_NODISCARD double DeltaY() const noexcept
    {
      return _deltaY;
    }
    /// @see https://w3c.github.io/uievents/#dom-wheelevent-deltaz
    KRYS_NODISCARD double DeltaZ() const noexcept
    {
      return _deltaZ;
    }
    /// @see https://w3c.github.io/uievents/#dom-wheelevent-deltamode
    KRYS_NODISCARD uint32 DeltaMode() const noexcept
    {
      return _deltaMode;
    }

    KRYS_NODISCARD bool IsWheelEvent() const noexcept override
    {
      return true;
    }

  private:
    WheelEvent(const PlatformWheelEvent &event, LocalFrameView *view, IsCancelable isCancelable) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::WheelEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsWheelEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
