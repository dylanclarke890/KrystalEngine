#include "Krystal.PAL/Platform.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"
#include <algorithm>

#define NOMINMAX
#include <shellscalingapi.h>
#include <windows.h>

#pragma comment(lib, "Shcore.lib")
#pragma comment(lib, "Winmm.lib")

namespace krys::pal
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
    krys_assert(result != TIMERR_NOCANDO);

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

  uint SetTimerPrecision(Maybe<uint> min) noexcept
  {
    TIMECAPS timeCaps {};

    {
      auto result = ::timeGetDevCaps(&timeCaps, sizeof(timeCaps));
      krys_assert(result != TIMERR_NOCANDO);
    }

    auto minUint = min.has_value() ? min.value() : 0;
    const uint &period = ::std::max(timeCaps.wPeriodMin, minUint);
    {
      auto result = ::timeBeginPeriod(period);
      krys_assert(result != TIMERR_NOCANDO);
    }

    return period;
  }

  void Sleep(milliseconds duration) noexcept
  {
    ::Sleep(static_cast<unsigned long>(duration.count()));
  }

  WheelScrollSettings GetWheelScrollSettings() noexcept
  {
    WheelScrollSettings settings;

    UINT lines = 0;
    if (::SystemParametersInfoW(SPI_GETWHEELSCROLLLINES, 0, &lines, 0))
    {
      if (lines == WHEEL_PAGESCROLL)
      {
        settings.Lines = null;
      }
      else
      {
        settings.Lines = static_cast<uint32>(lines);
      }
    }

    UINT chars = 0;
    if (::SystemParametersInfoW(SPI_GETWHEELSCROLLCHARS, 0, &chars, 0))
    {
      settings.Chars = static_cast<uint32>(chars);
    }
    return settings;
  }
}