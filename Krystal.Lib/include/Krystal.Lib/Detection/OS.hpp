#pragma once

/// @brief Check the underlying operating system.
#define KRYS_OS(OS) (defined KRYS_OS_##OS && KRYS_OS_##OS)

#if defined(__APPLE__)
  #include <Availability.h>
  #include <AvailabilityMacros.h>
  #include <TargetConditionals.h>
#endif

// KRYS_OS(AIX) - AIX
#if defined(_AIX)
  #define KRYS_OS_AIX 1
#endif

// KRYS_OS(ANDROID) - Android
#if defined(__ANDROID__) || defined(ANDROID)
  #define KRYS_OS_ANDROID 1
#endif

// KRYS_OS(DARWIN) - Any Darwin-based OS, including macOS, iOS, iPadOS, macCatalyst, tvOS, watchOS and
// visionOS
#if defined(__APPLE__)
  #define KRYS_OS_DARWIN 1
#endif

// KRYS_OS(IOS_FAMILY) - iOS family, including iOS, iPadOS, macCatalyst, tvOS, watchOS, and visionOS
#if KRYS_OS(DARWIN) && TARGET_OS_IPHONE
  #define KRYS_OS_IOS_FAMILY 1
#endif

// KRYS_OS(IOS) - iOS and iPadOS only (iPhone and iPad), not including macCatalyst, not including watchOS, not
// including tvOS, not including visionOS
#if KRYS_OS(DARWIN)                                                                                          \
  && (TARGET_OS_IOS && !(defined(TARGET_OS_MACCATALYST) && TARGET_OS_MACCATALYST)                            \
      && !(defined(TARGET_OS_VISION) && TARGET_OS_VISION))
  #define KRYS_OS_IOS 1
#endif

// KRYS_OS(TVOS) - tvOS
#if KRYS_OS(DARWIN) && TARGET_OS_TV
  #define KRYS_OS_TVOS 1
#endif

// KRYS_OS(WATCHOS) - watchOS
#if KRYS_OS(DARWIN) && TARGET_OS_WATCH
  #define KRYS_OS_WATCHOS 1
#endif

// OS(MACOS) - macOS (not including iOS family)
#if KRYS_OS(DARWIN) && TARGET_OS_OSX
  #define KRYS_OS_MACOS 1
#endif

// KRYS_OS(FREEBSD) - FreeBSD
#if defined(__FreeBSD__) || defined(__DragonFly__) || defined(__FreeBSD_kernel__)
  #define KRYS_OS_FREEBSD 1
#endif

// KRYS_OS(FUCHSIA) - Fuchsia
#if defined(__Fuchsia__)
  #define KRYS_OS_FUCHSIA 1
#endif

// KRYS_OS(HAIKU) - Haiku
#if defined(__HAIKU__)
  #define KRYS_OS_HAIKU 1
#endif

// KRYS_OS(HURD) - GNU/Hurd
#if defined(__GNU__)
  #define KRYS_OS_HURD 1
#endif

// KRYS_OS(LINUX) - Linux
#if defined(__linux__)
  #define KRYS_OS_LINUX 1
#endif

// KRYS_OS(NETBSD) - NetBSD
#if defined(__NetBSD__)
  #define KRYS_OS_NETBSD 1
#endif

// KRYS_OS(OPENBSD) - OpenBSD
#if defined(__OpenBSD__)
  #define KRYS_OS_OPENBSD 1
#endif

// KRYS_OS(QNX) - QNX
#if defined(__QNX__)
  #define KRYS_OS_QNX 1
#endif

// KRYS_OS(WINDOWS) - Any version of Windows
#if defined(WIN32) || defined(_WIN32)
  #define KRYS_OS_WINDOWS 1
#endif

// KRYS_OS(UNIX) - Any Unix-like system
#if KRYS_OS(AIX) || KRYS_OS(DARWIN) || KRYS_OS(FREEBSD) || KRYS_OS(FUCHSIA) || KRYS_OS(HAIKU)                \
  || KRYS_OS(HURD) || KRYS_OS(LINUX) || KRYS_OS(NETBSD) || KRYS_OS(OPENBSD) || defined(unix)                 \
  || defined(__unix) || defined(__unix__)
  #define KRYS_OS_UNIX 1
#endif
