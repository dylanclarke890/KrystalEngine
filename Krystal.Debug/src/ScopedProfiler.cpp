#include "Krystal.Debug/ScopedProfiler.hpp"
#include "Krystal.Log/ILogger.hpp"
#include "Krystal.Platform/Platform.hpp"

namespace Krys::Debug
{
  ScopedProfiler::ScopedProfiler(const string &name) noexcept : _start(0), _name(name)
  {
    _start = Platform::GetTimeMilliseconds();
  }

  ScopedProfiler::~ScopedProfiler() noexcept
  {
    const auto end = Platform::GetTimeMilliseconds();
    const auto elapsedMs = end - _start;
    KRYS_INFO("{} took {:.3f} ms.", _name, elapsedMs);
  }
}