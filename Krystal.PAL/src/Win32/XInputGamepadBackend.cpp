#include "Krystal.PAL/Gamepad.hpp"
#include <algorithm>
#include <cmath>

#define NOMINMAX
#include <windows.h>

#include <Xinput.h>

#pragma comment(lib, "Xinput.lib")

namespace krys::pal
{
  namespace
  {
    /// @brief Maps a stick's raw position to axis values from -1 to 1 (down positive), with a radial dead
    /// zone: 0 within it, and rescaled beyond it so the values still start at 0.
    void StickAxes(SHORT rawX, SHORT rawY, SHORT deadZone, float &x, float &y) noexcept
    {
      constexpr float Maximum = 32767.0f;
      float fx = static_cast<float>(rawX);
      float fy = -static_cast<float>(rawY);
      float magnitude = std::sqrt(fx * fx + fy * fy);
      if (magnitude <= deadZone)
      {
        x = y = 0;
        return;
      }
      float scaled = (std::min(magnitude, Maximum) - deadZone) / (Maximum - deadZone);
      x = std::clamp(fx / magnitude * scaled, -1.0f, 1.0f);
      y = std::clamp(fy / magnitude * scaled, -1.0f, 1.0f);
    }

    /// @brief Xbox-style pads through XInput, in its four slots.
    class XInputGamepadBackend final : public GamepadBackend
    {
    public:
      KRYS_NODISCARD uint32 SlotCount() const noexcept override
      {
        return XUSER_MAX_COUNT;
      }

      KRYS_NODISCARD Maybe<GamepadReading> Read(uint32 slot) noexcept override
      {
        XINPUT_STATE state {};
        if (::XInputGetState(slot, &state) != ERROR_SUCCESS)
        {
          return null;
        }

        GamepadReading reading;
        reading.PacketNumber = state.dwPacketNumber;
        const auto &pad = state.Gamepad;
        auto set = [&](GamepadButton button, WORD flag)
        {
          reading.Buttons[static_cast<size_t>(button)] = (pad.wButtons & flag) ? 1.0f : 0.0f;
        };
        set(GamepadButton::South, XINPUT_GAMEPAD_A);
        set(GamepadButton::East, XINPUT_GAMEPAD_B);
        set(GamepadButton::West, XINPUT_GAMEPAD_X);
        set(GamepadButton::North, XINPUT_GAMEPAD_Y);
        set(GamepadButton::LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER);
        set(GamepadButton::RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER);
        set(GamepadButton::Back, XINPUT_GAMEPAD_BACK);
        set(GamepadButton::Start, XINPUT_GAMEPAD_START);
        set(GamepadButton::LeftStick, XINPUT_GAMEPAD_LEFT_THUMB);
        set(GamepadButton::RightStick, XINPUT_GAMEPAD_RIGHT_THUMB);
        set(GamepadButton::DPadUp, XINPUT_GAMEPAD_DPAD_UP);
        set(GamepadButton::DPadDown, XINPUT_GAMEPAD_DPAD_DOWN);
        set(GamepadButton::DPadLeft, XINPUT_GAMEPAD_DPAD_LEFT);
        set(GamepadButton::DPadRight, XINPUT_GAMEPAD_DPAD_RIGHT);
        // XInput doesn't report the Guide button, which stays released.
        reading.Buttons[static_cast<size_t>(GamepadButton::LeftTrigger)] =
          static_cast<float>(pad.bLeftTrigger) / 255.0f;
        reading.Buttons[static_cast<size_t>(GamepadButton::RightTrigger)] =
          static_cast<float>(pad.bRightTrigger) / 255.0f;

        StickAxes(pad.sThumbLX, pad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
                  reading.Axes[static_cast<size_t>(GamepadAxis::LeftX)],
                  reading.Axes[static_cast<size_t>(GamepadAxis::LeftY)]);
        StickAxes(pad.sThumbRX, pad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
                  reading.Axes[static_cast<size_t>(GamepadAxis::RightX)],
                  reading.Axes[static_cast<size_t>(GamepadAxis::RightY)]);
        return reading;
      }

      KRYS_NODISCARD utf8_string Name(uint32) const noexcept override
      {
        return u8"Xbox Controller (XInput STANDARD GAMEPAD)";
      }

      bool SetRumble(uint32 slot, float strong, float weak) noexcept override
      {
        XINPUT_VIBRATION vibration {};
        vibration.wLeftMotorSpeed = static_cast<WORD>(std::clamp(strong, 0.0f, 1.0f) * 65535.0f);
        vibration.wRightMotorSpeed = static_cast<WORD>(std::clamp(weak, 0.0f, 1.0f) * 65535.0f);
        return ::XInputSetState(slot, &vibration) == ERROR_SUCCESS;
      }
    };
  }

  UniquePtr<GamepadBackend> CreateGamepadBackend() noexcept
  {
    return CreateUnique<XInputGamepadBackend>();
  }
}
