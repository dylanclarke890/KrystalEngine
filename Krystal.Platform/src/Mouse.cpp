#include "Krystal.Platform/Mouse.hpp"

namespace Krys::Platform
{
  KRYS_NODISCARD float Mouse::ClientX() const noexcept
  {
    return _clientX;
  }

  KRYS_NODISCARD float Mouse::ClientY() const noexcept
  {
    return _clientY;
  }

  KRYS_NODISCARD float Mouse::DeltaX() const noexcept
  {
    return _deltaX;
  }

  KRYS_NODISCARD float Mouse::DeltaY() const noexcept
  {
    return _deltaY;
  }

  KRYS_NODISCARD float Mouse::ScrollDelta() const noexcept
  {
    return _scrollDelta;
  }

  KRYS_NODISCARD bool Mouse::IsButtonPressed(MouseButton button) const noexcept
  {
    return (_pressed & button) == button;
  }

  KRYS_NODISCARD bool Mouse::IsButtonHeld(MouseButton button) const noexcept
  {
    return (_held & button) == button;
  }

  KRYS_NODISCARD bool Mouse::WasButtonReleased(MouseButton button) const noexcept
  {
    return (_released & button) == button;
  }
}
