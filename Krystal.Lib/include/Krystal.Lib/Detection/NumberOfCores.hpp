#pragma once

#include "Krystal.Lib/Detection/OS.hpp"

namespace Krys
{
  // This counts logical cores.
  int NumberOfProcessorCores();

#if KRYS_OS(DARWIN)
  int NumberOfPhysicalProcessorCores();
#endif
}