#include "Krystal.PAL/Platform.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>

#define NOMINMAX
#include <windows.h>

#include "Krystal.PAL/Win32/DropTarget.hpp"

#include <ole2.h>
#include <shellapi.h>
#include <shellscalingapi.h>
#include <shlwapi.h>

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shcore.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Winmm.lib")

namespace krys::pal
{
  namespace
  {
    /// @brief Whether OLE was initialised on the thread, for the windows' drop targets.
    bool OleInitialised = false;
  }

  void Initialise() noexcept
  {
    SetTimerPrecision();
    OleInitialised = SUCCEEDED(::OleInitialize(nullptr));
  }

  void Shutdown() noexcept
  {
    if (OleInitialised)
    {
      ::OleUninitialize();
      OleInitialised = false;
    }

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

  PointingDevices GetPointingDevices() noexcept
  {
    PointingDevices devices;
    devices.Mouse = ::GetSystemMetrics(SM_MOUSEPRESENT) != 0;
    const int digitizer = ::GetSystemMetrics(SM_DIGITIZER);
    const bool ready = (digitizer & NID_READY) != 0;
    devices.Touch = ready && (digitizer & (NID_INTEGRATED_TOUCH | NID_EXTERNAL_TOUCH)) != 0;
    devices.Pen = ready && (digitizer & (NID_INTEGRATED_PEN | NID_EXTERNAL_PEN)) != 0;
    devices.MaxTouchPoints = devices.Touch ? static_cast<uint32>(::GetSystemMetrics(SM_MAXIMUMTOUCHES)) : 0;
    return devices;
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

  namespace
  {
    // The clipboard's text uses Windows line breaks.
    KRYS_NODISCARD utf8_string ToWindowsLineBreaks(utf8_stringview text) noexcept
    {
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
      return windowsText;
    }

    // Windows line breaks become LFs, and lone CRs too.
    KRYS_NODISCARD utf8_string FromWindowsLineBreaks(utf8_stringview windowsText) noexcept
    {
      utf8_string text;
      text.reserve(windowsText.size());
      for (size_t i = 0; i < windowsText.size(); ++i)
      {
        if (windowsText[i] == u8'\r')
        {
          text.push_back(u8'\n');
          if (i + 1 < windowsText.size() && windowsText[i + 1] == u8'\n')
          {
            ++i;
          }
          continue;
        }
        text.push_back(windowsText[i]);
      }
      return text;
    }

    KRYS_NODISCARD std::wstring ToWide(utf8_stringview text) noexcept
    {
      const auto *bytes = reinterpret_cast<const char *>(text.data());
      const int length = static_cast<int>(text.size());
      const int wideLength = length ? ::MultiByteToWideChar(CP_UTF8, 0, bytes, length, nullptr, 0) : 0;
      std::wstring wide(static_cast<size_t>(wideLength), L'\0');
      if (wideLength)
      {
        ::MultiByteToWideChar(CP_UTF8, 0, bytes, length, wide.data(), wideLength);
      }
      return wide;
    }

    KRYS_NODISCARD utf8_string FromWide(const wchar_t *wide, int wideLength) noexcept
    {
      const int length =
        wideLength ? ::WideCharToMultiByte(CP_UTF8, 0, wide, wideLength, nullptr, 0, nullptr, nullptr) : 0;
      utf8_string text(static_cast<size_t>(length), u8'\0');
      if (length)
      {
        ::WideCharToMultiByte(CP_UTF8, 0, wide, wideLength, reinterpret_cast<char *>(text.data()), length,
                              nullptr, nullptr);
      }
      return text;
    }

    KRYS_NODISCARD HGLOBAL CreateGlobal(const void *data, size_t size) noexcept
    {
      HGLOBAL memory = ::GlobalAlloc(GMEM_MOVEABLE, size ? size : 1);
      if (!memory)
      {
        return nullptr;
      }
      if (auto *destination = ::GlobalLock(memory))
      {
        if (size)
        {
          std::memcpy(destination, data, size);
        }
        ::GlobalUnlock(memory);
      }
      return memory;
    }

    KRYS_NODISCARD HGLOBAL CreateGlobalText(utf8_stringview text) noexcept
    {
      auto wide = ToWide(ToWindowsLineBreaks(text));
      return CreateGlobal(wide.c_str(), (wide.size() + 1) * sizeof(wchar_t));
    }

    KRYS_NODISCARD UINT HTMLClipboardFormat() noexcept
    {
      static const UINT format = ::RegisterClipboardFormatW(L"HTML Format");
      return format;
    }

    KRYS_NODISCARD UINT RegisteredClipboardFormat(utf8_stringview name) noexcept
    {
      return ::RegisterClipboardFormatW(ToWide(name).c_str());
    }

    // The HTML format: a header of byte offsets, then the markup between fragment markers in a document.
    // See https://learn.microsoft.com/windows/win32/dataxchg/html-clipboard-format.
    KRYS_NODISCARD std::string MarkupToCFHTML(utf8_stringview markup) noexcept
    {
      constexpr std::string_view startMarkup = "<HTML>\n<BODY>\n<!--StartFragment-->\n";
      constexpr std::string_view endMarkup = "\n<!--EndFragment-->\n</BODY>\n</HTML>";
      constexpr size_t headerLength = std::string_view("Version:0.9\n"
                                                       "StartHTML:0000000000\n"
                                                       "EndHTML:0000000000\n"
                                                       "StartFragment:0000000000\n"
                                                       "EndFragment:0000000000\n")
                                        .size();

      const size_t startHTMLOffset = headerLength;
      const size_t startFragmentOffset = startHTMLOffset + startMarkup.size();
      const size_t endFragmentOffset = startFragmentOffset + markup.size();
      const size_t endHTMLOffset = endFragmentOffset + endMarkup.size();

      char header[headerLength + 1];
      std::snprintf(
        header, sizeof(header),
        "Version:0.9\nStartHTML:%010zu\nEndHTML:%010zu\nStartFragment:%010zu\nEndFragment:%010zu\n",
        startHTMLOffset, endHTMLOffset, startFragmentOffset, endFragmentOffset);

      std::string result(header, headerLength);
      result.append(startMarkup);
      result.append(reinterpret_cast<const char *>(markup.data()), markup.size());
      result.append(endMarkup);
      return result;
    }

    KRYS_NODISCARD size_t FindIgnoringASCIICase(std::string_view string, std::string_view lowercaseTarget,
                                                size_t start = 0) noexcept
    {
      if (start > string.size())
      {
        return std::string_view::npos;
      }
      auto it = std::search(string.begin() + static_cast<ptrdiff_t>(start), string.end(),
                            lowercaseTarget.begin(), lowercaseTarget.end(), [](char a, char b)
                            { return (a >= 'A' && a <= 'Z' ? static_cast<char>(a - 'A' + 'a') : a) == b; });
      return it == string.end() ? std::string_view::npos : static_cast<size_t>(it - string.begin());
    }

    KRYS_NODISCARD utf8_string ExtractMarkupFromCFHTML(std::string_view cfhtml) noexcept
    {
      auto markupStart = FindIgnoringASCIICase(cfhtml, "<html");
      auto tagStart = FindIgnoringASCIICase(cfhtml, "startfragment",
                                            markupStart == std::string_view::npos ? 0 : markupStart);
      if (tagStart == std::string_view::npos)
      {
        return {};
      }
      auto fragmentStart = cfhtml.find('>', tagStart);
      if (fragmentStart == std::string_view::npos)
      {
        return {};
      }
      ++fragmentStart;
      auto tagEnd = FindIgnoringASCIICase(cfhtml, "endfragment", fragmentStart);
      auto fragmentEnd =
        tagEnd == std::string_view::npos ? std::string_view::npos : cfhtml.rfind('<', tagEnd);
      if (fragmentEnd == std::string_view::npos || fragmentEnd < fragmentStart)
      {
        return {};
      }

      auto fragment = cfhtml.substr(fragmentStart, fragmentEnd - fragmentStart);
      auto isSpace = [](char c)
      {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
      };
      while (!fragment.empty() && isSpace(fragment.front()))
      {
        fragment.remove_prefix(1);
      }
      while (!fragment.empty() && isSpace(fragment.back()))
      {
        fragment.remove_suffix(1);
      }
      return utf8_string(reinterpret_cast<const char8_t *>(fragment.data()), fragment.size());
    }

    /// @brief Opens the clipboard for the owner window, the active window if null.
    KRYS_NODISCARD bool OpenClipboardForWriting(NativeHandle windowHandle) noexcept
    {
      if (!windowHandle.IsValid())
      {
        windowHandle = GetActiveWindow();
      }
      // Without an owner window, emptying the clipboard leaves it unowned, and setting its data fails.
      return windowHandle.IsValid() && ::OpenClipboard(windowHandle.As<HWND>());
    }

    /// @brief Gives @p memory to the open clipboard as @p format, freeing it if the clipboard doesn't take
    /// it.
    bool SetData(UINT format, HGLOBAL memory) noexcept
    {
      if (!memory)
      {
        return false;
      }
      if (!::SetClipboardData(format, memory))
      {
        // The clipboard owns the memory only once it takes it.
        ::GlobalFree(memory);
        return false;
      }
      return true;
    }

    /// @brief The bytes of the clipboard's @p format, if it holds that format.
    KRYS_NODISCARD Maybe<std::string> GetData(UINT format) noexcept
    {
      if (!format || !::IsClipboardFormatAvailable(format) || !::OpenClipboard(nullptr))
      {
        return {};
      }

      Maybe<std::string> result;
      if (HANDLE memory = ::GetClipboardData(format))
      {
        if (const auto *bytes = static_cast<const char *>(::GlobalLock(memory)))
        {
          result = std::string(bytes, ::GlobalSize(memory));
          ::GlobalUnlock(memory);
        }
      }
      ::CloseClipboard();
      return result;
    }

    /// @brief The paths of the files @p drop names.
    KRYS_NODISCARD List<utf8_string> FilesFromDrop(HDROP drop) noexcept
    {
      List<utf8_string> paths;
      const UINT count = ::DragQueryFileW(drop, 0xFF'FF'FF'FF, nullptr, 0);
      for (UINT i = 0; i < count; ++i)
      {
        const UINT length = ::DragQueryFileW(drop, i, nullptr, 0);
        std::wstring path(length + 1, L'\0');
        ::DragQueryFileW(drop, i, path.data(), length + 1);
        paths.push_back(FromWide(path.c_str(), static_cast<int>(length)));
      }
      return paths;
    }
  }

  bool SetClipboardText(utf8_stringview text, NativeHandle windowHandle) noexcept
  {
    ClipboardContent content;
    content.Text = utf8_string(text);
    return SetClipboardContent(content, windowHandle);
  }

  bool SetClipboardContent(const ClipboardContent &content, NativeHandle windowHandle) noexcept
  {
    if (!OpenClipboardForWriting(windowHandle))
    {
      return false;
    }
    ::EmptyClipboard();

    bool set = true;
    if (content.Text)
    {
      set &= SetData(CF_UNICODETEXT, CreateGlobalText(*content.Text));
    }
    if (content.HTML)
    {
      auto cfhtml = MarkupToCFHTML(*content.HTML);
      set &= SetData(HTMLClipboardFormat(), CreateGlobal(cfhtml.c_str(), cfhtml.size() + 1));
    }
    for (auto &[name, bytes] : content.Data)
    {
      set &= SetData(RegisteredClipboardFormat(name), CreateGlobal(bytes.data(), bytes.size()));
    }
    ::CloseClipboard();
    return set;
  }

  bool ClearClipboard(NativeHandle windowHandle) noexcept
  {
    if (!OpenClipboardForWriting(windowHandle))
    {
      return false;
    }
    bool emptied = ::EmptyClipboard();
    ::CloseClipboard();
    return emptied;
  }

  bool IsClipboardFormatAvailable(ClipboardFormat format) noexcept
  {
    switch (format)
    {
      case ClipboardFormat::Text:  return ::IsClipboardFormatAvailable(CF_UNICODETEXT);
      case ClipboardFormat::HTML:  return ::IsClipboardFormatAvailable(HTMLClipboardFormat());
      case ClipboardFormat::Files: return ::IsClipboardFormatAvailable(CF_HDROP);
    }
    return false;
  }

  bool IsClipboardDataAvailable(utf8_stringview formatName) noexcept
  {
    return ::IsClipboardFormatAvailable(RegisteredClipboardFormat(formatName));
  }

  Maybe<utf8_string> GetClipboardText() noexcept
  {
    auto bytes = GetData(CF_UNICODETEXT);
    if (!bytes)
    {
      return {};
    }
    const auto *wide = reinterpret_cast<const wchar_t *>(bytes->data());
    const int wideLength = static_cast<int>(::wcsnlen(wide, bytes->size() / sizeof(wchar_t)));
    return FromWindowsLineBreaks(FromWide(wide, wideLength));
  }

  Maybe<utf8_string> GetClipboardHTML() noexcept
  {
    auto bytes = GetData(HTMLClipboardFormat());
    if (!bytes)
    {
      return {};
    }
    std::string_view cfhtml(bytes->data(), ::strnlen(bytes->data(), bytes->size()));
    return ExtractMarkupFromCFHTML(cfhtml);
  }

  List<utf8_string> GetClipboardFiles() noexcept
  {
    List<utf8_string> paths;
    if (!::IsClipboardFormatAvailable(CF_HDROP) || !::OpenClipboard(nullptr))
    {
      return paths;
    }

    if (auto drop = static_cast<HDROP>(::GetClipboardData(CF_HDROP)))
    {
      paths = FilesFromDrop(drop);
    }
    ::CloseClipboard();
    return paths;
  }

  Maybe<List<uint8>> GetClipboardData(utf8_stringview formatName) noexcept
  {
    auto bytes = GetData(RegisteredClipboardFormat(formatName));
    if (!bytes)
    {
      return {};
    }
    return List<uint8>(bytes->begin(), bytes->end());
  }

  uint32 GetClipboardChangeCount() noexcept
  {
    return static_cast<uint32>(::GetClipboardSequenceNumber());
  }

  namespace
  {
    /// @brief The bytes of @p data's @p format, if it has that format.
    KRYS_NODISCARD Maybe<std::string> GetDragData(IDataObject &data, UINT format) noexcept
    {
      FORMATETC request {static_cast<CLIPFORMAT>(format), nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
      STGMEDIUM medium {};
      if (!format || FAILED(data.GetData(&request, &medium)))
      {
        return {};
      }

      Maybe<std::string> result;
      if (const auto *bytes = static_cast<const char *>(::GlobalLock(medium.hGlobal)))
      {
        result = std::string(bytes, ::GlobalSize(medium.hGlobal));
        ::GlobalUnlock(medium.hGlobal);
      }
      ::ReleaseStgMedium(&medium);
      return result;
    }

    /// @brief The UTF-16 text in @p bytes, up to its terminator.
    KRYS_NODISCARD utf8_string WideText(const std::string &bytes) noexcept
    {
      const auto *wide = reinterpret_cast<const wchar_t *>(bytes.data());
      return FromWide(wide, static_cast<int>(::wcsnlen(wide, bytes.size() / sizeof(wchar_t))));
    }

    /// @brief The text in the system's code page in @p bytes, up to its terminator.
    KRYS_NODISCARD utf8_string AnsiText(const std::string &bytes) noexcept
    {
      const int length = static_cast<int>(::strnlen(bytes.data(), bytes.size()));
      const int wideLength = length ? ::MultiByteToWideChar(CP_ACP, 0, bytes.data(), length, nullptr, 0) : 0;
      std::wstring wide(static_cast<size_t>(wideLength), L'\0');
      if (wideLength)
      {
        ::MultiByteToWideChar(CP_ACP, 0, bytes.data(), length, wide.data(), wideLength);
      }
      return FromWide(wide.c_str(), wideLength);
    }

    /// @brief Firefox's markup format.
    KRYS_NODISCARD UINT TextHTMLFormat() noexcept
    {
      static const UINT format = ::RegisterClipboardFormatW(L"text/html");
      return format;
    }

    KRYS_NODISCARD UINT URLWFormat() noexcept
    {
      static const UINT format = ::RegisterClipboardFormatW(L"UniformResourceLocatorW");
      return format;
    }

    KRYS_NODISCARD UINT URLFormat() noexcept
    {
      static const UINT format = ::RegisterClipboardFormatW(L"UniformResourceLocator");
      return format;
    }

    /// @brief Sets @p content's URL to the internet shortcut (a `.url` file) at @p path's, and its title to
    /// the shortcut's name.
    /// @return Whether @p path is an internet shortcut with a URL.
    bool ReadInternetShortcut(utf8_stringview path, DragContent &content) noexcept
    {
      auto widePath = ToWide(path);
      if (::_wcsicmp(::PathFindExtensionW(widePath.c_str()), L".url"))
      {
        return false;
      }

      // The longest URL Windows takes (`INTERNET_MAX_URL_LENGTH`).
      constexpr DWORD maxURLLength = 2'048 + 32 + 4;
      std::wstring url(maxURLLength, L'\0');
      const DWORD length = ::GetPrivateProfileStringW(L"InternetShortcut", L"url", nullptr, url.data(),
                                                      maxURLLength, widePath.c_str());
      if (!length)
      {
        return false;
      }

      content.URL = FromWide(url.c_str(), static_cast<int>(length));
      std::wstring name(::PathFindFileNameW(widePath.c_str()));
      ::PathRemoveExtensionW(name.data());
      content.URLTitle = FromWide(name.c_str(), static_cast<int>(::wcslen(name.c_str())));
      return true;
    }

    /// @brief Sets @p content's URL and title from @p url, a URL and its title on the next line, if it has
    /// one; else the URL is its own title.
    void SetURLAndTitle(utf8_string &&url, DragContent &content) noexcept
    {
      auto split = url.find(u8'\n');
      if (split == utf8_string::npos)
      {
        content.URLTitle = url;
        content.URL = krys::move(url);
        return;
      }
      content.URLTitle = url.substr(split + 1);
      url.resize(split);
      content.URL = krys::move(url);
    }
  }

  namespace win32
  {
    DragContent ReadDragContent(IDataObject &data) noexcept
    {
      DragContent content;
      if (auto bytes = GetDragData(data, CF_UNICODETEXT))
      {
        content.Text = FromWindowsLineBreaks(WideText(*bytes));
      }
      else if (auto ansiBytes = GetDragData(data, CF_TEXT))
      {
        content.Text = FromWindowsLineBreaks(AnsiText(*ansiBytes));
      }

      // Firefox's markup, else the HTML format's fragment.
      if (auto bytes = GetDragData(data, TextHTMLFormat()))
      {
        content.HTML = WideText(*bytes);
      }
      else if (auto cfhtmlBytes = GetDragData(data, HTMLClipboardFormat()))
      {
        auto markup = ExtractMarkupFromCFHTML(
          std::string_view(cfhtmlBytes->data(), ::strnlen(cfhtmlBytes->data(), cfhtmlBytes->size())));
        if (!markup.empty())
        {
          content.HTML = krys::move(markup);
        }
      }

      FORMATETC filesRequest {CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
      STGMEDIUM files {};
      if (SUCCEEDED(data.GetData(&filesRequest, &files)))
      {
        content.Files = FilesFromDrop(static_cast<HDROP>(files.hGlobal));
        ::ReleaseStgMedium(&files);
      }

      // An internet shortcut dragged is its URL; else the URL formats' URL, with its title.
      if (!content.Files.empty() && ReadInternetShortcut(content.Files.front(), content))
      {
        return content;
      }
      if (auto bytes = GetDragData(data, URLWFormat()))
      {
        SetURLAndTitle(WideText(*bytes), content);
      }
      else if (auto ansiBytes = GetDragData(data, URLFormat()))
      {
        SetURLAndTitle(AnsiText(*ansiBytes), content);
      }
      return content;
    }
  }
}
