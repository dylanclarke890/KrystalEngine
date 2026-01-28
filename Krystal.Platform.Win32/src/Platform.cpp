#include "Krystal.Platform/Platform.hpp"
#include "Krystal.Lib/String/String.hpp"
#include <algorithm>
#include <cassert>
#include <optional>

#define NOMINMAX
#include <shellscalingapi.h>
#include <windows.h>

#pragma comment(lib, "Shcore.lib")

namespace Krys::Platform
{
  void Initialise() noexcept
  {
    SetTimerPrecision();
  }

  void Shutdown() noexcept
  {
    // Reset timer precision to default
    TIMECAPS timeCaps {};
    auto result = ::timeGetDevCaps(&timeCaps, sizeof(timeCaps));
    assert(result != TIMERR_NOCANDO);
    if (timeCaps.wPeriodMin > 0)
    {
      ::timeEndPeriod(timeCaps.wPeriodMin);
    }
  }

  int GetDPIForWindow(NativeHandle windowHandle) noexcept
  {
    if (!windowHandle.IsValid())
    {
      windowHandle = GetActiveWindow();
    }
    auto window = windowHandle.As<HWND>();
    return static_cast<int>(::GetDpiForWindow(window));
  }

  NativeHandle GetActiveWindow() noexcept
  {
    return NativeHandle(::GetActiveWindow());
  }

  uint SetTimerPrecision(Nullable<uint> min) noexcept
  {
    TIMECAPS timeCaps {};

    {
      auto result = ::timeGetDevCaps(&timeCaps, sizeof(timeCaps));
      assert(result != TIMERR_NOCANDO);
    }

    auto minUint = min.has_value() ? min.value() : 0;
    const uint &period = ::std::max(timeCaps.wPeriodMin, minUint);
    {
      auto result = ::timeBeginPeriod(period);
      assert(result != TIMERR_NOCANDO);
    }

    return period;
  }

  void Sleep(Milliseconds duration) noexcept
  {
    ::Sleep(static_cast<unsigned long>(duration.count()));
  }
}