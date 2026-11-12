/*
 * Copyright (C) 2003, 2006, 2010, 2013, 2017 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief Sets the user's preferred languages, most preferred first, as BCP 47 tags (empty for the
  /// system's).
  void OverrideUserPreferredLanguages(List<utf8_string> languages) noexcept;

  /// @brief The languages the host set; empty if it set none.
  KRYS_NODISCARD const List<utf8_string> &UserPreferredLanguagesOverride() noexcept;

  /// @brief The user's preferred languages, most preferred first: the host's, or else the system's.
  KRYS_NODISCARD List<utf8_string> UserPreferredLanguages() noexcept;

  /// @brief A number that changes whenever the host sets the user's preferred languages, for caches of what
  /// they decide.
  KRYS_NODISCARD uint32 UserPreferredLanguagesGeneration() noexcept;

  /// @brief Whether Han text with no language of its own prefers Simplified Chinese: unless the user prefers
  /// `zh-TW` before `zh-CN`.
  KRYS_NODISCARD bool UserPrefersSimplifiedChinese() noexcept;
}
