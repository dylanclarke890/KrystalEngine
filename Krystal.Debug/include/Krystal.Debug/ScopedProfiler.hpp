#pragma once

#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/String.hpp"

namespace Krys::Debug
{
  class ScopedProfiler
  {
  public:
    ScopedProfiler(const string &name) noexcept;
    ~ScopedProfiler() noexcept;

  private:
    double _start;
    string _name;
  };

#ifdef KRYS_ENABLE_PROFILING
  #define KRYS_SCOPED_PROFILER(name)                                                                         \
    auto KRYS_CONCAT(scopedProfiler, __LINE__) = Krys::Debug::ScopedProfiler(name);
#else
  #define KRYS_SCOPED_PROFILER(name)
#endif
}