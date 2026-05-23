#include "Krystal.Debug/ScopedProfiler.hpp"
#include "Krystal.Log/ILogger.hpp"
#include "Krystal.Lib/Time/Duration.hpp"
#include <chrono>

namespace Krys::Debug
{
  ScopedProfiler::ScopedProfiler(const string &name) noexcept : _performance(), _name(name)
  {
  }

  ScopedProfiler::~ScopedProfiler() noexcept
  {
    const auto elapsedMs = Milliseconds(MonotonicTime::Now()).count();
    KRYS_INFO("{} took {:.3f} ms.", _name, elapsedMs);
  }
}