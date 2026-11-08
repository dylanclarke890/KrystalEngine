#include "Krystal.Threading/Thread.hpp"
#include "Krystal.Core/Utils/Move.hpp"

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <string>
  #include <windows.h>
#endif

namespace krys
{
  namespace
  {
    /// @brief Gives the calling thread @p name, as debuggers and profilers show it.
    void SetCurrentThreadName(utf8_stringview name) noexcept
    {
#ifdef _WIN32
      const auto *bytes = reinterpret_cast<const char *>(name.data());
      const int length = static_cast<int>(name.size());
      const int wideLength = length ? ::MultiByteToWideChar(CP_UTF8, 0, bytes, length, nullptr, 0) : 0;
      std::wstring wide(static_cast<size_t>(wideLength), L'\0');
      if (wideLength)
      {
        ::MultiByteToWideChar(CP_UTF8, 0, bytes, length, wide.data(), wideLength);
      }
      (void)::SetThreadDescription(::GetCurrentThread(), wide.c_str());
#else
      (void)name;
#endif
    }
  }

  Thread::Thread(utf8_string name, MoveOnlyFunc<void()> body) noexcept : _name(krys::move(name))
  {
    _thread = std::jthread(
      [name = _name, body = krys::move(body)]() mutable
      {
        SetCurrentThreadName(name);
        body();
      });
  }

  Thread::~Thread() noexcept
  {
    if (_thread.joinable())
    {
      _thread.join();
    }
  }
}
