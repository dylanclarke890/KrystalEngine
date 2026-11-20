#pragma once

#include "Krystal.Core/Debug.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"

namespace krys
{
  class ScopedProfiler
  {
  public:
    ScopedProfiler(string name) noexcept;

    ~ScopedProfiler() noexcept;

  private:
    MonotonicTime::duration _performance;
    string _name;
  };

#if KRYS_DEBUG
  #define KRYS_SCOPED_PROFILER(name) auto KRYS_CONCAT(scopedProfiler, __LINE__) = krys::ScopedProfiler(name);
#else
  #define KRYS_SCOPED_PROFILER(name)
#endif
}
