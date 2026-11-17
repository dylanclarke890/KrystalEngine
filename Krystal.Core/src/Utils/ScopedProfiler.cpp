#include "Krystal.Core/Utils/ScopedProfiler.hpp"
#include "Krystal.Core/Log/ILogger.hpp"
#include "Krystal.Core/Time/Duration.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include <chrono>

namespace krys
{
  ScopedProfiler::ScopedProfiler(string name) noexcept : _performance(), _name(krys::move(name))
  {
  }

  ScopedProfiler::~ScopedProfiler() noexcept
  {
    const auto elapsedMs = milliseconds(MonotonicTime::Now()).count();
    KRYS_LOG_INFO("{} took {:.3f} ms.", _name, elapsedMs);
  }
}
