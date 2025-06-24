#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Core/Events/EventManager.hpp"
#include "Krystal.Engine/Events.hpp"
#include "Krystal.Engine/Keyboard.hpp"
#include "Krystal.Engine/Mouse.hpp"
#include "Krystal.Platform/IWindow.hpp"

namespace Krys
{
  class Application;
}

namespace Krys::Engine
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
    void OnKeyboardEvent(Platform::WindowHandle window, Platform::Key key, Platform::KeyState state) noexcept;

    void OnMouseButtonEvent(Platform::WindowHandle window, Platform::MouseButton btn,
                            Platform::MouseButtonState state) noexcept;

    void OnMouseMoveEvent(Platform::WindowHandle window, float dx, float dy, float clientX,
                          float clientY) noexcept;

    void OnMouseScrollEvent(Platform::WindowHandle window, float delta) noexcept;
  };
}