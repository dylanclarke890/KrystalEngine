#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Platform/IInput.hpp"

namespace Krys::Platform
{
  struct WindowSettings;
  class IWindow;

  Expected<Unique<IWindow>> CreateWindow(const WindowSettings &settings, Ptr<IInput> input,
                                         Ptr<EventManager> events) noexcept;

  struct WindowSettings
  {
    /// @brief The title of the window.
    string Title {};

    /// @brief The initial width of the window.
    uint32 Width {800};

    /// @brief The initial height of the window.
    uint32 Height {600};

    /// @brief The initial vsync state.
    bool VSync {true};
  };

  class IWindow
  {
  public:
    NO_COPY_MOVE(IWindow)

    virtual ~IWindow() noexcept = default;

    NO_DISCARD virtual NativeHandle GetNativeHandle() const noexcept = 0;

    virtual void ProcessMessages() noexcept = 0;

    virtual void SetTitle(const string &title) noexcept = 0;

    NO_DISCARD virtual const string &GetTitle() const noexcept = 0;

    virtual void Show() noexcept = 0;

    virtual void Hide() noexcept = 0;

  protected:
    IWindow() noexcept = default;
  };
}