#pragma once

#include "Krystal.Core/Events/EventManager.hpp"
#include "Krystal.PAL/Events.hpp"
#include "Krystal.PAL/IWindow.hpp"
#include "Krystal.PAL/Keyboard.hpp"
#include "Krystal.PAL/Mouse.hpp"

namespace krys
{
  class Application;
}

namespace krys::pal
{
  class Input
  {
    friend class Application;

  private:
    Keyboard _keyboard;
    Mouse _mouse;
    EventManager *_events;

  public:
    Input(EventManager *events) noexcept;

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