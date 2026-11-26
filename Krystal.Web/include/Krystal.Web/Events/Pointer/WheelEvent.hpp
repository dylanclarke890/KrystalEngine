#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Events/Pointer/MouseEvent.hpp"
#include "Krystal.Web/Input/PlatformWheelEvent.hpp"

namespace krys::web
{
  /// @see https://w3c.github.io/pointerevents/#dom-wheeleventinit
  struct WheelEventInit : MouseEventInit
  {
    double DeltaX {0};
    double DeltaY {0};
    double DeltaZ {0};
    uint32 DeltaMode {0};
  };

  /// @brief A turn of the mouse wheel or a similar device: `wheel`.
  /// @see https://w3c.github.io/pointerevents/#interface-wheelevent
  class WheelEvent final : public MouseEvent
  {
  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = WheelEventInit;

    /// @brief Whether a trusted wheel event can be canceled.
    enum class IsCancelable : bool
    {
      No,
      Yes,
    };

    /// @see https://w3c.github.io/pointerevents/#dom-wheelevent-dom_delta_pixel
    constexpr static uint32 DOM_DELTA_PIXEL = 0x00;
    /// @see https://w3c.github.io/pointerevents/#dom-wheelevent-dom_delta_line
    constexpr static uint32 DOM_DELTA_LINE = 0x01;
    /// @see https://w3c.github.io/pointerevents/#dom-wheelevent-dom_delta_page
    constexpr static uint32 DOM_DELTA_PAGE = 0x02;

  private:
    double _deltaX {0};
    double _deltaY {0};
    double _deltaZ {0};
    uint32 _deltaMode {DOM_DELTA_PIXEL};
    /// @brief The host's event, for a trusted event.
    Maybe<PlatformWheelEvent> _underlyingPlatformEvent;

  public:
    /// @see https://w3c.github.io/pointerevents/#dom-wheelevent-constructor
    WheelEvent(DOMStringAtom type, const WheelEventInit &initializer = {}) noexcept;

    /// @brief A trusted `wheel` event from the host's @p event, over @p view. It bubbles and is composed.
    KRYS_NODISCARD static Ref<WheelEvent> Create(const PlatformWheelEvent &event, web::FrameView *view,
                                                 IsCancelable isCancelable = IsCancelable::Yes) noexcept;

    /// @brief The host's event a trusted wheel event came from, with what the DOM event leaves out (the
    /// gesture's phase and momentum, wheel ticks).
    KRYS_NODISCARD const Maybe<PlatformWheelEvent> &UnderlyingPlatformEvent() const noexcept
    {
      return _underlyingPlatformEvent;
    }

    /// @brief Positive when scrolling right.
    /// @see https://w3c.github.io/pointerevents/#dom-wheelevent-deltax
    KRYS_NODISCARD double DeltaX() const noexcept
    {
      return _deltaX;
    }

    /// @brief Positive when scrolling down.
    /// @see https://w3c.github.io/pointerevents/#dom-wheelevent-deltay
    KRYS_NODISCARD double DeltaY() const noexcept
    {
      return _deltaY;
    }

    /// @see https://w3c.github.io/pointerevents/#dom-wheelevent-deltaz
    KRYS_NODISCARD double DeltaZ() const noexcept
    {
      return _deltaZ;
    }

    /// @see https://w3c.github.io/pointerevents/#dom-wheelevent-deltamode
    KRYS_NODISCARD uint32 DeltaMode() const noexcept
    {
      return _deltaMode;
    }

    KRYS_NODISCARD bool IsWheelEvent() const noexcept override
    {
      return true;
    }

  private:
    WheelEvent(const PlatformWheelEvent &event, web::FrameView *view, IsCancelable isCancelable) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::WheelEvent)
  KRYS_NODISCARD static bool IsType(const krys::web::Event &event) noexcept
  {
    return event.IsWheelEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
