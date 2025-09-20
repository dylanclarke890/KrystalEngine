#pragma once

#include "Krystal.Lib/Types.hpp"

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
  #define UNIQUE_PROFILER_NAME(prefix) CONCATENATE(prefix, __LINE__)
  #define KRYS_SCOPED_PROFILER(name) Krys::Debug::ScopedProfiler UNIQUE_PROFILER_NAME(profiler_)(name)
  #undef UNIQUE_PROFILER_NAME
#else
  #define KRYS_SCOPED_PROFILER(name)
#endif
}