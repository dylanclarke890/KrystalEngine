#pragma once

#include "Krystal.Core/Detection/OS.hpp"

namespace krys
{
  // This counts logical cores.
  int NumberOfProcessorCores();

#if KRYS_OS(DARWIN)
  int NumberOfPhysicalProcessorCores();
#endif
}