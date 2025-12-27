#pragma once

/// @brief Check if a particular environment is being used.
#define KRYS_ENV(ENV) (defined KRYS_ENV_##ENV && KRYS_ENV_##ENV)

/// @brief Check if a particular CPU is supported. KRYS_CPU(KNOWN) will be false if the CPU is unknown.
#define KRYS_CPU(CPU) (defined KRYS_CPU_##CPU && KRYS_CPU_##CPU)

/// @brief Check the underlying operating system.
#define KRYS_OS(OS) (defined KRYS_OS_##OS && KRYS_OS_##OS)

#pragma region Environment

// Default to development environment if none is specified.
#if !KRYS_ENV(DEV) && !KRYS_ENV(STAGE) && !KRYS_ENV(PROD)
  #define KRYS_ENV_DEV 1
  #warning "No environment specified, defaulting to development environment."
#endif

#pragma endregion

#pragma region Compiler

#if defined(__clang__)
  #define KRYS_COMPILER_CLANG 1
#endif

#if defined(__GNUC__)
  #define KRYS_COMPILER_GCC_COMPATIBLE 1
#endif

#if KRYS_COMPILER(GCC_COMPATIBLE) && !KRYS_COMPILER(CLANG)
  #define KRYS_COMPILER_GCC 1
#endif

#if defined(_MSC_VER)
  #define KRYS_COMPILER_MSVC 1
#endif

#pragma endregion

#pragma region Endianness

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  #define KRYS_CPU_BIG_ENDIAN 1
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  #define KRYS_CPU_LITTLE_ENDIAN 1
#elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
  #define KRYS_CPU_MIDDLE_ENDIAN 1
#else
  #error "Unknown endian"
#endif

#if !KRYS_CPU(LITTLE_ENDIAN) && !KRYS_CPU(BIG_ENDIAN)
  #error "Unsupported endian"
#endif

#pragma endregion

#pragma region CPU

// KRYS_CPU(MIPS) - MIPS 32-bit and 64-bit
#if (defined(mips) || defined(__mips__) || defined(MIPS) || defined(_MIPS_) || defined(__mips64))
  #if defined(_ABI64) && (_MIPS_SIM == _ABI64)
    #define KRYS_CPU_MIPS64 1
    #define KRYS_MIPS_ARCH __mips64
  #else
    #define KRYS_CPU_MIPS 1
    #define KRYS_MIPS_ARCH __mips
  #endif
  #define KRYS_CPU_KNOWN 1
  #define KRYS_MIPS_PIC (defined __PIC__)
  #define KRYS_MIPS_ISA(v) (defined KRYS_MIPS_ARCH && KRYS_MIPS_ARCH == v)
  #define KRYS_MIPS_ISA_AT_LEAST(v) (defined KRYS_MIPS_ARCH && KRYS_MIPS_ARCH >= v)
  #define KRYS_MIPS_ARCH_REV __mips_isa_rev
  #define KRYS_MIPS_ISA_REV(v) (defined KRYS_MIPS_ARCH_REV && KRYS_MIPS_ARCH_REV == v)
  #define KRYS_MIPS_ISA_REV_AT_LEAST(v) (defined KRYS_MIPS_ARCH_REV && KRYS_MIPS_ARCH_REV >= v)
  #define KRYS_MIPS_DOUBLE_FLOAT (defined __mips_hard_float && !defined __mips_single_float)
  #define KRYS_MIPS_FP64 (defined __mips_fpr && __mips_fpr == 64)
#endif

// KRYS_CPU(LOONGARCH64) - LOONGARCH64
#if defined(__loongarch64)
  #define KRYS_CPU_LOONGARCH64 1
  #define KRYS_CPU_KNOWN 1
#endif

// KRYS_CPU(PPC64) - PowerPC 64-bit Big Endian
#if (defined(__ppc64__) || defined(__PPC64__)) && defined(__BYTE_ORDER__)                                    \
  && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
  #define KRYS_CPU_PPC64 1
  #define KRYS_CPU_KNOWN 1
#endif

// KRYS_CPU(PPC64LE) - PowerPC 64-bit Little Endian
#if (defined(__ppc64__) || defined(__PPC64__) || defined(__ppc64le__) || defined(__PPC64LE__))               \
  && defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
  #define KRYS_CPU_PPC64LE 1
  #define KRYS_CPU_KNOWN 1
#endif

// KRYS_CPU(PPC) - PowerPC 32-bit
#if (defined(__ppc__) || defined(__PPC__) || defined(__powerpc__) || defined(__powerpc)                      \
     || defined(__POWERPC__) || defined(_M_PPC) || defined(__PPC))                                           \
  && !KRYS_CPU(PPC64) && KRYS_CPU(BIG_ENDIAN)
  #define KRYS_CPU_PPC 1
  #define KRYS_CPU_KNOWN 1
#endif

// KRYS_CPU(X86) - i386 / x86 32-bit
#if defined(__i386__) || defined(i386) || defined(_M_IX86) || defined(_X86_) || defined(__THW_INTEL)
  #define KRYS_CPU_X86 1
  #define KRYS_CPU_KNOWN 1

  #if defined(__SSE2__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
    #define KRYS_CPU_X86_SSE2 1
  #endif
#endif

// KRYS_CPU(X86_64) - AMD64 / Intel64 / x86_64 64-bit
#if defined(__x86_64__) || defined(_M_X64)
  #define KRYS_CPU_X86_64 1
  #define KRYS_CPU_X86_SSE2 1
  #define KRYS_CPU_KNOWN 1
#endif

// KRYS_CPU(ARM64)
#if defined(__arm64__) || defined(__aarch64__)
  #define KRYS_CPU_ARM64 1
  #define KRYS_CPU_KNOWN 1

  #if defined(__arm64e__)
    #define KRYS_CPU_ARM64E 1
  #endif
#endif

// KRYS_CPU(ARM) - ARM, any version
#define KRYS_ARM_ARCH_AT_LEAST(N) (KRYS_CPU(ARM) && KRYS_ARM_ARCH_VERSION >= N)

#if defined(arm) || defined(__arm__) || defined(ARM) || defined(_ARM_)
  #define KRYS_CPU_ARM 1
  #define KRYS_CPU_KNOWN 1

  #if defined(__ARM_PCS_VFP)
    #define KRYS_CPU_ARM_HARDFP 1
  #endif

  // Set KRYS_ARM_ARCH_VERSION
  #if defined(__ARM_ARCH_4__) || defined(__ARM_ARCH_4T__) || defined(__MARM_ARMV4__)
    #define KRYS_ARM_ARCH_VERSION 4
  #elif defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5T__) || defined(__MARM_ARMV5__)
    #define KRYS_ARM_ARCH_VERSION 5
  #elif defined(__ARM_ARCH_5E__) || defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_5TEJ__)
    #define KRYS_ARM_ARCH_VERSION 5
  #elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__)                      \
    || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6KZ__) || defined(__ARM_ARCH_6ZK__)                    \
    || defined(__ARM_ARCH_6T2__) || defined(__ARMV6__)
    #define KRYS_ARM_ARCH_VERSION 6
  #elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7K__) || defined(__ARM_ARCH_7R__)                     \
    || defined(__ARM_ARCH_7S__)
    #define KRYS_ARM_ARCH_VERSION 7
  #elif defined(__ARM_ARCH_8__) || defined(__ARM_ARCH_8A__)
    #define KRYS_ARM_ARCH_VERSION 8
  #elif defined(_M_ARM) // set by MSVC
    #define KRYS_ARM_ARCH_VERSION _M_ARM
  #elif defined(__TARGET_ARCH_ARM) // set by RVCT
    #define KRYS_ARM_ARCH_VERSION __TARGET_ARCH_ARM
  #else
    #warning "Unknown ARM architecture version, defaulting to 0"
    #define KRYS_ARM_ARCH_VERSION 0
  #endif

  // Set KRYS_THUMB_ARCH_VERSION
  #if defined(__ARM_ARCH_4T__)
    #define KRYS_THUMB_ARCH_VERSION 1
  #elif defined(__ARM_ARCH_5T__) || defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_5TEJ__)
    #define KRYS_THUMB_ARCH_VERSION 2
  #elif defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__)                     \
    || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6M__)
    #define KRYS_THUMB_ARCH_VERSION 3
  #elif defined(__ARM_ARCH_6T2__) || defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__)                     \
    || defined(__ARM_ARCH_7K__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7R__)                      \
    || defined(__ARM_ARCH_7S__) || defined(__ARM_ARCH_8__) || defined(__ARM_ARCH_8A__)
    #define KRYS_THUMB_ARCH_VERSION 4
  #elif defined(__TARGET_ARCH_THUMB) // RVCT sets __TARGET_ARCH_THUMB
    #define KRYS_THUMB_ARCH_VERSION __TARGET_ARCH_THUMB
  #else
    #warning "Unknown ARM Thumb architecture version, defaulting to 0"
    #define KRYS_THUMB_ARCH_VERSION 0
  #endif

  // KRYS_CPU(ARMV5_OR_LOWER) - ARM instruction set v5 or earlier
  // On ARMv5 and below the natural alignment is required, along with other differences.
  #if !defined(KRYS_CPU_ARMV5_OR_LOWER) && !KRYS_ARM_ARCH_AT_LEAST(6)
    #define KRYS_CPU_ARMV5_OR_LOWER 1
  #endif

  // KRYS_CPU(ARM_TRADITIONAL) - Thumb2 is not available, only traditional ARM (v4 or greater)
  // KRYS_CPU(ARM_THUMB2) - Thumb2 instruction set is available
  // Only one of the above will be defined.
  #if !defined(KRYS_CPU_ARM_TRADITIONAL) && !defined(KRYS_CPU_ARM_THUMB2)
    #if defined(thumb2) || defined(__thumb2__)                                                               \
      || ((defined(__thumb) || defined(__thumb__)) && KRYS_THUMB_ARCH_VERSION == 4)
      #define KRYS_CPU_ARM_TRADITIONAL 0
      #define KRYS_CPU_ARM_THUMB2 1
    #elif KRYS_ARM_ARCH_AT_LEAST(4)
      #define KRYS_CPU_ARM_TRADITIONAL 1
      #define KRYS_CPU_ARM_THUMB2 0
    #else
      #error "Not supported ARM architecture"
    #endif
  #elif CPU(ARM_TRADITIONAL) && CPU(ARM_THUMB2) // Sanity Check
    #error "'KRYS_CPU_ARM_TRADITIONAL' and 'KRYS_CPU_ARM_THUMB2' were both defined."
  #endif

  #if (defined(__ARM_NEON__) || defined(__ARM_NEON)) && !defined(KRYS_CPU_ARM_NEON)
    #define KRYS_CPU_ARM_NEON 1
  #endif

  #if (defined(__VFP_FP__) && !defined(__SOFTFP__))
    #define KRYS_CPU_ARM_VFP 1
  #endif

  // If KRYS_CPU(ARM_NEON) is not enabled, we'll conservatively assume only VFP2 or VFPv3D16 support is
  // available. Hence, only the first 16 64-bit floating point registers are available. See:
  // NEON registers:
  // http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0473c/CJACABEJ.html
  // VFP2 and VFP3 registers:
  // http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0473c/CIHDIBDG.html
  // NEON to VFP register mapping:
  // http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0473c/CJAIJHFC.html
  #if KRYS_CPU(ARM_NEON)
    #define KRYS_CPU_ARM_VFP_V3_D32 1
  #else
    #define KRYS_CPU_ARM_VFP_V2 1
  #endif

  #if defined(__ARM_ARCH_7K__)
    #define KRYS_CPU_APPLE_ARMV7K 1
  #endif

  #if defined(__ARM_ARCH_7S__)
    #define KRYS_CPU_APPLE_ARMV7S 1
  #endif
#endif

// KRYS_CPU(RISCV64) - RISC-V 64-bit
#if defined(__riscv) && defined(__riscv_xlen) && (__riscv_xlen == 64)
  #define KRYS_CPU_RISCV64 1
  #define KRYS_CPU_KNOWN 1
#endif

#if !KRYS_CPU(KNOWN)
  #define KRYS_CPU_UNKNOWN 1
#endif

#pragma endregion

#pragma region OS

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

#pragma endregion
