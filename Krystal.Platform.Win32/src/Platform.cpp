#include "Krystal.Platform/Platform.hpp"
#include "Krystal.Lib/String/String.hpp"
#include <algorithm>
#include <cassert>
#include <optional>

#define NOMINMAX
#include <shellscalingapi.h>
#include <windows.h>

#pragma comment(lib, "Shcore.lib")

namespace
{
  using namespace Krys;

  int64 StartTicks = 0;
  int64 TickFrequency = 0;
}

namespace Krys::Platform
{
  void Initialise() noexcept
  {
    SetTimerPrecision();

    {
      LARGE_INTEGER freq;
      auto result = ::QueryPerformanceFrequency(&freq);
      assert(result);
      TickFrequency = freq.QuadPart;
    }

    {
      LARGE_INTEGER start;
      auto result = ::QueryPerformanceCounter(&start);
      assert(result);
      StartTicks = start.QuadPart;
    }
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

  double GetTime() noexcept
  {
    LARGE_INTEGER now;
    {
      auto result = ::QueryPerformanceCounter(&now);
      assert(result);
    }
    return static_cast<double>(now.QuadPart - StartTicks) / TickFrequency;
  }

  double GetTimeMilliseconds() noexcept
  {
    LARGE_INTEGER now;
    {
      auto result = ::QueryPerformanceCounter(&now);
      assert(result);
    }
    return static_cast<double>(now.QuadPart - StartTicks) * 1000.0 / TickFrequency;
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

  void Sleep(uint32 milliseconds) noexcept
  {
    ::Sleep(milliseconds);
  }
}