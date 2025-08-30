#pragma once

#include "Krystal.Core/Core.hpp"

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
}