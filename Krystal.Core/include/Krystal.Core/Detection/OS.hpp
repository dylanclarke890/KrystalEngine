/*
 * Copyright (C) 2006-2021 Apple Inc. All rights reserved.
 * Copyright (C) 2007-2009 Torch Mobile, Inc.
 * Copyright (C) 2010, 2011 Research In Motion Limited. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#if defined(__APPLE__)
  #include <Availability.h>
  #include <AvailabilityMacros.h>
  #include <TargetConditionals.h>
#endif

/// @brief Check the underlying operating system.
#define KRYS_OS(OS) (defined KRYS_OS_##OS && KRYS_OS_##OS)

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

// KRYS_OS(WIN64) - 64-bit Windows
#if defined(WIN64) || defined(_WIN64)
  #define KRYS_OS_WIN64 1
#endif

// KRYS_OS(WIN32) - 32-bit Windows
#if !KRYS_OS(WIN64) && (defined(WIN32) || defined(_WIN32))
  #define KRYS_OS_WIN32 1
#endif

// KRYS_OS(WINDOWS) - Any version of Windows
#if KRYS_OS(WIN64) || KRYS_OS(WIN32)
  #define KRYS_OS_WINDOWS 1
#endif

// KRYS_OS(UNIX) - Any Unix-like system
#if KRYS_OS(AIX) || KRYS_OS(DARWIN) || KRYS_OS(FREEBSD) || KRYS_OS(FUCHSIA) || KRYS_OS(HAIKU)                \
  || KRYS_OS(HURD) || KRYS_OS(LINUX) || KRYS_OS(NETBSD) || KRYS_OS(OPENBSD) || defined(unix)                 \
  || defined(__unix) || defined(__unix__)
  #define KRYS_OS_UNIX 1
#endif
