#include "Krystal.Platform/IInput.hpp"

#include "Krystal.Core/Core.hpp"
#include "Krystal.Core/Events/EventManager.hpp"
#include "Krystal.Platform/Keys.hpp"

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace Krys::Platform
{
  class Win32Input final : public IInput
  {
    Set<WPARAM> _pressedKeys;
    Ptr<EventManager> _eventManager;

    NO_COPY_MOVE(Win32Input)
  public:
    Win32Input(Ptr<EventManager> eventManager) noexcept;
    ~Win32Input() noexcept override = default;

    void BeginFrame() noexcept override;

    void PollDevices() noexcept override;

    bool HandleWindowMessage(UINT message, WPARAM wParam, LPARAM lParam, HWND windowHandle) noexcept;

  private:
    Key KeyCodeToEngineKey(const WPARAM keyCode) const noexcept;
  };
}