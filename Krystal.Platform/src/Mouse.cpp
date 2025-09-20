#include "Krystal.Platform/Mouse.hpp"
#include "Krystal.Platform/MouseButtons.hpp"

namespace Krys::Platform
{
  NO_DISCARD float Mouse::ClientX() const noexcept
  {
    return _clientX;
  }

  NO_DISCARD float Mouse::ClientY() const noexcept
  {
    return _clientY;
  }

  NO_DISCARD float Mouse::DeltaX() const noexcept
  {
    return _deltaX;
  }

  NO_DISCARD float Mouse::DeltaY() const noexcept
  {
    return _deltaY;
  }

  NO_DISCARD float Mouse::ScrollDelta() const noexcept
  {
    return _scrollDelta;
  }

  NO_DISCARD bool Mouse::IsButtonPressed(MouseButton button) const noexcept
  {
    return (_pressed & button) == button;
  }

  NO_DISCARD bool Mouse::IsButtonHeld(MouseButton button) const noexcept
  {
    return (_held & button) == button;
  }

  NO_DISCARD bool Mouse::WasButtonReleased(MouseButton button) const noexcept
  {
    return (_released & button) == button;
  }
}
