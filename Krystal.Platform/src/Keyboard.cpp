#include "Krystal.Platform/Keyboard.hpp"
#include "Krystal.Platform/Keys.hpp"

namespace Krys::Platform
{
  NO_DISCARD bool Keyboard::IsKeyPressed(Key key) const noexcept
  {
    return _pressed.contains(key);
  }

  NO_DISCARD bool Keyboard::IsKeyHeld(Key key) const noexcept
  {
    return _held.contains(key);
  }

  NO_DISCARD bool Keyboard::WasKeyReleased(Key key) const noexcept
  {
    return _released.contains(key);
  }
}
