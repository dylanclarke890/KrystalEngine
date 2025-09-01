#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Core/Events/EventManager.hpp"
#include "Krystal.Platform/Events.hpp"
#include "Krystal.Platform/IWindow.hpp"
#include "Krystal.Platform/Keyboard.hpp"
#include "Krystal.Platform/Mouse.hpp"

namespace Krys
{
  class Application;
}

namespace Krys::Platform
{
  class Input
  {
    Keyboard _keyboard;
    Mouse _mouse;
    Ptr<EventManager> _events;

    friend class Application;

  public:
    NO_COPY_MOVE(Input)

    Input(Ptr<EventManager> events) noexcept;

    ~Input() noexcept = default;

    void BeginFrame() noexcept;

    const Keyboard &GetKeyboard() const noexcept;

    const Mouse &GetMouse() const noexcept;

  private:
    void OnKeyboardEvent(WindowHandle window, Key key, KeyState state) noexcept;

    void OnMouseButtonEvent(WindowHandle window, MouseButton btn, MouseButtonState state) noexcept;

    void OnMouseMoveEvent(WindowHandle window, float dx, float dy, float clientX, float clientY) noexcept;

    void OnMouseScrollEvent(WindowHandle window, float delta) noexcept;
  };
}