/*
 * Copyright (C) 2012 University of Szeged. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY UNIVERSITY OF SZEGED ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL UNIVERSITY OF SZEGED OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Core/Detection/NumberOfCores.hpp"
#include "Krystal.Core/Numeric.hpp"

#if KRYS_OS(DARWIN)
  #include <sys/sysctl.h>
#elif KRYS_OS(LINUX) || KRYS_OS(AIX) || KRYS_OS(OPENBSD) || KRYS_OS(NETBSD) || KRYS_OS(FREEBSD)              \
  || KRYS_OS(HAIKU)
  #include <unistd.h>
#elif KRYS_OS(QNX)
  #include <sys/syspage.h>
#elif KRYS_OS(WINDOWS)
  #include <windows.h>
#endif

namespace krys
{
  int NumberOfProcessorCores()
  {
    const int Default = 1;
    static int NumberOfCores = -1;

    if (NumberOfCores > 0)
    {
      return NumberOfCores;
    }

#if KRYS_OS(DARWIN)
    unsigned result;
    size_t length = sizeof(result);
    int name[] = {CTL_HW, HW_AVAILCPU};
    int sysctlResult = sysctl(name, sizeof(name) / sizeof(int), &result, &length, 0, 0);

    NumberOfCores = sysctlResult < 0 ? Default : result;
#elif KRYS_OS(LINUX) || KRYS_OS(AIX) || KRYS_OS(OPENBSD) || KRYS_OS(NETBSD) || KRYS_OS(FREEBSD)              \
  || KRYS_OS(HAIKU)
    long sysconfResult = sysconf(_SC_NPROCESSORS_ONLN);

    NumberOfCores = sysconfResult < 0 ? Default : static_cast<int>(sysconfResult);
#elif KRYS_OS(QNX)
    int numCpuQNX = _syspage_ptr->num_cpu;
    NumberOfCores = numCpuQNX < 0 ? Default : numCpuQNX;
#elif KRYS_OS(WINDOWS)
    (void)Default;
    ::SYSTEM_INFO sysInfo;
    ::GetSystemInfo(&sysInfo);

    NumberOfCores = sysInfo.dwNumberOfProcessors;
#else
    NumberOfCores = Default;
#endif

    return NumberOfCores;
  }

#if KRYS_OS(DARWIN)
  int NumberOfPhysicalProcessorCores()
  {
    static int32 numCores = []
    {
      constexpr int32 Default = 1;
      size_t valueSize = sizeof(numCores);
      int32 numCores = 0;
      int result = sysctlbyname("hw.physicalcpu_max", &numCores, &valueSize, nullptr, 0);
      if (result < 0)
      {
        return Default;
      }
      return numCores;
    }();

    return numCores;
  }
#endif
}
