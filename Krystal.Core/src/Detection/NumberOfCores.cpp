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
