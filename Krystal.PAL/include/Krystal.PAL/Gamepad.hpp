#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Time/Duration.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::pal
{
  class Input;

  /// @brief A gamepad's buttons, in the order of the standard gamepad layout.
  /// @see https://w3c.github.io/gamepad/#remapping
  enum class GamepadButton : uint8
  {
    /// @brief The bottom button of the right cluster (A on an Xbox pad).
    South,
    /// @brief The right button of the right cluster (B).
    East,
    /// @brief The left button of the right cluster (X).
    West,
    /// @brief The top button of the right cluster (Y).
    North,
    LeftShoulder,
    RightShoulder,
    LeftTrigger,
    RightTrigger,
    /// @brief The left button of the centre cluster (View, or Back).
    Back,
    /// @brief The right button of the centre cluster (Menu, or Start).
    Start,
    /// @brief Pressing the left stick.
    LeftStick,
    /// @brief Pressing the right stick.
    RightStick,
    DPadUp,
    DPadDown,
    DPadLeft,
    DPadRight,
    /// @brief The middle button of the centre cluster (the Xbox button).
    Guide
  };

  /// @brief A gamepad's axes, in the order of the standard gamepad layout.
  enum class GamepadAxis : uint8
  {
    LeftX,
    LeftY,
    RightX,
    RightY
  };

  /// @brief The state of a gamepad's button.
  enum class GamepadButtonState : uint8
  {
    Pressed,
    Released
  };

  inline constexpr size_t GamepadButtonCount = 17;
  inline constexpr size_t GamepadAxisCount = 4;
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::pal::GamepadButton, 17uz)
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::pal::GamepadAxis, 4uz)

namespace krys::pal
{
  /// @brief A gamepad's state as its backend reads it.
  struct GamepadReading
  {
    /// @brief Each button's value, from 0 (released) to 1 (fully pressed); between them for analogue
    /// triggers.
    Array<float, GamepadButtonCount> Buttons {};
    /// @brief Each axis's value, from -1 to 1, with right and down positive; 0 within the stick's dead zone.
    Array<float, GamepadAxisCount> Axes {};
    /// @brief Changes whenever the pad's state does.
    uint32 PacketNumber {0};
  };

  /// @brief Reads gamepads from the system, in numbered slots.
  class GamepadBackend
  {
  public:
    virtual ~GamepadBackend() noexcept = default;

    /// @brief How many slots there are: the most pads that can be connected at once.
    KRYS_NODISCARD virtual uint32 SlotCount() const noexcept = 0;

    /// @brief Reads the pad in @p slot.
    /// @return Its state, or none if no pad is connected there.
    KRYS_NODISCARD virtual Maybe<GamepadReading> Read(uint32 slot) noexcept = 0;

    /// @brief A name for the pad in @p slot, saying what kind of pad it is.
    KRYS_NODISCARD virtual utf8_string Name(uint32 slot) const noexcept = 0;

    /// @brief Sets the pad's rumble motors' strengths, from 0 (off) to 1, until they're set again.
    /// @param strong The low frequency (heavy) motor's strength.
    /// @param weak The high frequency (light) motor's strength.
    /// @return Whether the pad took them.
    virtual bool SetRumble(uint32 slot, float strong, float weak) noexcept = 0;
  };

  /// @brief Creates the platform's gamepad backend.
  KRYS_NODISCARD UniquePtr<GamepadBackend> CreateGamepadBackend() noexcept;

  /// @brief A gamepad slot's state for the current frame, updated each frame by `Input`.
  class Gamepad
  {
    friend class Input;

  private:
    utf8_string _name;
    Array<float, GamepadButtonCount> _buttons {};
    Array<float, GamepadAxisCount> _axes {};
    EnumSet<GamepadButton> _pressed;
    EnumSet<GamepadButton> _held;
    EnumSet<GamepadButton> _released;
    nanoseconds _lastChanged {};
    uint32 _packetNumber {0};
    uint32 _slot {0};
    bool _connected {false};

  public:
    Gamepad() noexcept = default;

    /// @brief How far a trigger must be pressed to count as pressed.
    constexpr static float TriggerThreshold = 30.0f / 255.0f;

    /// @brief The slot the pad is in.
    KRYS_NODISCARD uint32 Slot() const noexcept
    {
      return _slot;
    }

    /// @brief Whether a pad is connected in the slot.
    KRYS_NODISCARD bool Connected() const noexcept
    {
      return _connected;
    }

    /// @brief A name saying what kind of pad it is.
    KRYS_NODISCARD const utf8_string &Name() const noexcept
    {
      return _name;
    }

    /// @brief How far @p button is pressed, from 0 to 1.
    KRYS_NODISCARD float ButtonValue(GamepadButton button) const noexcept
    {
      return _buttons[static_cast<size_t>(button)];
    }

    /// @brief @p axis's value, from -1 to 1, with right and down positive.
    KRYS_NODISCARD float AxisValue(GamepadAxis axis) const noexcept
    {
      return _axes[static_cast<size_t>(axis)];
    }

    /// @brief Whether @p button was pressed this frame.
    KRYS_NODISCARD bool IsButtonPressed(GamepadButton button) const noexcept
    {
      return _pressed.Contains(button);
    }

    /// @brief Whether @p button is down (pressed this frame or before).
    KRYS_NODISCARD bool IsButtonHeld(GamepadButton button) const noexcept
    {
      return _held.Contains(button) || _pressed.Contains(button);
    }

    /// @brief Whether @p button was released this frame.
    KRYS_NODISCARD bool WasButtonReleased(GamepadButton button) const noexcept
    {
      return _released.Contains(button);
    }

    /// @brief When the pad's state last changed (on the monotonic clock).
    KRYS_NODISCARD nanoseconds LastChanged() const noexcept
    {
      return _lastChanged;
    }

    /// @brief Whether a button's @p value counts as pressed.
    KRYS_NODISCARD constexpr static bool IsPressedValue(GamepadButton button, float value) noexcept
    {
      if (button == GamepadButton::LeftTrigger || button == GamepadButton::RightTrigger)
      {
        return value > TriggerThreshold;
      }
      return value > 0.5f;
    }
  };
}
