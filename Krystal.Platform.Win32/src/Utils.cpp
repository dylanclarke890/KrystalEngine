#include "Krystal.Platform.Win32/Utils.hpp"

#include "Krystal.Core/Core.hpp"

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <errhandlingapi.h>

namespace Krys::Platform::Win32
{
  string GetLastErrorAsString() noexcept
  {
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0)
      return string();

    LPSTR messageBuffer = nullptr;
    size_t size = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
                                     | FORMAT_MESSAGE_IGNORE_INSERTS,
                                   NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                   reinterpret_cast<LPSTR>(&messageBuffer), 0, NULL);

    string message(messageBuffer, size);
    ::LocalFree(messageBuffer);
    return message;
  }
}