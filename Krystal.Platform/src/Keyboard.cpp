#include "Krystal.Platform/Keyboard.hpp"

namespace Krys::Platform
{
  KRYS_NODISCARD bool Keyboard::IsKeyPressed(Key key) const noexcept
  {
    return _pressed.contains(key);
  }

  KRYS_NODISCARD bool Keyboard::IsKeyHeld(Key key) const noexcept
  {
    return _held.contains(key);
  }

  KRYS_NODISCARD bool Keyboard::WasKeyReleased(Key key) const noexcept
  {
    return _released.contains(key);
  }
}
