#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief A gamepad's button, as `getGamepads()` last saw it.
  /// @see https://w3c.github.io/gamepad/#gamepadbutton-interface
  class GamepadButton final : public RefCounted<GamepadButton>
  {
    KRYS_NON_COPY_MOVABLE(GamepadButton);

  private:
    double _value {0.0};

  public:
    GamepadButton() noexcept = default;

    KRYS_NODISCARD static Ref<GamepadButton> Create() noexcept
    {
      return CreateRef<GamepadButton>();
    }

    /// @see https://w3c.github.io/gamepad/#dom-gamepadbutton-pressed
    KRYS_NODISCARD bool Pressed() const noexcept
    {
      // FIXME: The spec states that for analog buttons there needs to be a threshold for determining the
      // state of "pressed". What should that threshold be?
      return _value > 0.1;
    }

    /// @brief How far it's pressed, from 0 to 1.
    /// @see https://w3c.github.io/gamepad/#dom-gamepadbutton-value
    KRYS_NODISCARD double Value() const noexcept
    {
      return _value;
    }

    void SetValue(double value) noexcept
    {
      _value = value;
    }
  };
}
