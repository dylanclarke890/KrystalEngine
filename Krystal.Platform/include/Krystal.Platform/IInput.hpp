#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Core/Events/EventManager.hpp"
#include "Krystal.Platform/Keyboard.hpp"
#include "Krystal.Platform/Keys.hpp"
#include "Krystal.Platform/Mouse.hpp"
#include "Krystal.Platform/MouseButtons.hpp"

namespace Krys::Platform
{
  class IInput;

  Unique<IInput> CreateInput(Ptr<EventManager> eventManager) noexcept;

  class IInput
  {
  protected:
    Keyboard _keyboard;
    Mouse _mouse;

  public:
    virtual ~IInput() noexcept = default;

    virtual void BeginFrame() noexcept = 0;

    virtual void PollDevices() noexcept = 0;

    const Keyboard &GetKeyboard() const noexcept;

    const Mouse &GetMouse() const noexcept;

  protected:
    NO_COPY_MOVE(IInput)

    IInput() noexcept = default;

    void OnKeyboardEvent(const Key key, const KeyState state) noexcept;

    void OnMouseButtonEvent(const MouseButton button, const MouseButtonState state) noexcept;

    void OnMouseMoveEvent(const float deltaX, const float deltaY, const float clientX,
                          const float clientY) noexcept;

    void OnScrollWheelEvent(const float delta) noexcept;

    void ClearState() noexcept;
  };
}