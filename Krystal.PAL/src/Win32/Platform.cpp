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

  bool SetClipboardText(utf8_stringview text, NativeHandle windowHandle) noexcept
  {
    if (!windowHandle.IsValid())
    {
      windowHandle = GetActiveWindow();
    }

    // The clipboard's text uses Windows line breaks.
    utf8_string windowsText;
    windowsText.reserve(text.size());
    for (auto c : text)
    {
      if (c == u8'\n')
      {
        windowsText.push_back(u8'\r');
      }
      windowsText.push_back(c);
    }

    const auto *bytes = reinterpret_cast<const char *>(windowsText.data());
    const int length = static_cast<int>(windowsText.size());
    const int wideLength = length ? ::MultiByteToWideChar(CP_UTF8, 0, bytes, length, nullptr, 0) : 0;
    if (length && !wideLength)
    {
      return false;
    }

    HGLOBAL memory = ::GlobalAlloc(GMEM_MOVEABLE, (static_cast<SIZE_T>(wideLength) + 1) * sizeof(wchar_t));
    if (!memory)
    {
      return false;
    }
    auto *wide = static_cast<wchar_t *>(::GlobalLock(memory));
    if (length)
    {
      ::MultiByteToWideChar(CP_UTF8, 0, bytes, length, wide, wideLength);
    }
    wide[wideLength] = L'\0';
    ::GlobalUnlock(memory);

    // Without an owner window, emptying the clipboard leaves it unowned, and setting its data fails.
    if (!windowHandle.IsValid() || !::OpenClipboard(windowHandle.As<HWND>()))
    {
      ::GlobalFree(memory);
      return false;
    }
    ::EmptyClipboard();
    bool set = ::SetClipboardData(CF_UNICODETEXT, memory) != nullptr;
    ::CloseClipboard();
    if (!set)
    {
      // The clipboard owns the memory only once it takes it.
      ::GlobalFree(memory);
    }
    return set;
  }
}
