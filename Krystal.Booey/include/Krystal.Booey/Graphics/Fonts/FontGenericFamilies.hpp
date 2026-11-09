/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Font/GenericFontFamilyNames.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/String.hpp"
#include <unicode/uscript.h>

namespace krys::boo
{
  using ScriptFontFamilyMap = HashMap<int32, utf8_string>;

  /// @brief The families the generic font families (`serif`, `sans-serif`, `monospace`, ...) resolve to, per
  /// script. A script without a family of its own uses the common script's; an empty family means none.
  class FontGenericFamilies
  {
  private:
    ScriptFontFamilyMap _standardFontFamilyMap;
    ScriptFontFamilyMap _serifFontFamilyMap;
    ScriptFontFamilyMap _fixedFontFamilyMap;
    ScriptFontFamilyMap _sansSerifFontFamilyMap;
    ScriptFontFamilyMap _cursiveFontFamilyMap;
    ScriptFontFamilyMap _fantasyFontFamilyMap;
    ScriptFontFamilyMap _pictographFontFamilyMap;
    ScriptFontFamilyMap _mathFontFamilyMap;

  public:
    FontGenericFamilies() noexcept;

    /// @brief Whether no generic family resolves to a family, for any script.
    KRYS_NODISCARD bool IsEmpty() const noexcept;

    KRYS_NODISCARD const utf8_string &StandardFontFamily(UScriptCode script = USCRIPT_COMMON) const noexcept;
    KRYS_NODISCARD const utf8_string &FixedFontFamily(UScriptCode script = USCRIPT_COMMON) const noexcept;
    KRYS_NODISCARD const utf8_string &SerifFontFamily(UScriptCode script = USCRIPT_COMMON) const noexcept;
    KRYS_NODISCARD const utf8_string &SansSerifFontFamily(UScriptCode script = USCRIPT_COMMON) const noexcept;
    KRYS_NODISCARD const utf8_string &CursiveFontFamily(UScriptCode script = USCRIPT_COMMON) const noexcept;
    KRYS_NODISCARD const utf8_string &FantasyFontFamily(UScriptCode script = USCRIPT_COMMON) const noexcept;
    KRYS_NODISCARD const utf8_string &
      PictographFontFamily(UScriptCode script = USCRIPT_COMMON) const noexcept;
    KRYS_NODISCARD const utf8_string &MathFontFamily(UScriptCode script = USCRIPT_COMMON) const noexcept;

    /// @brief The family the generic family @p family resolves to for @p script, or null for `system-ui`.
    KRYS_NODISCARD const utf8_string *FontFamily(css::GenericFontFamilyNames::FamilyNamesIndex family,
                                                 UScriptCode script = USCRIPT_COMMON) const noexcept;

    /// @return Whether the family changed.
    bool SetStandardFontFamily(const utf8_string &family, UScriptCode script) noexcept;
    bool SetFixedFontFamily(const utf8_string &family, UScriptCode script) noexcept;
    bool SetSerifFontFamily(const utf8_string &family, UScriptCode script) noexcept;
    bool SetSansSerifFontFamily(const utf8_string &family, UScriptCode script) noexcept;
    bool SetCursiveFontFamily(const utf8_string &family, UScriptCode script) noexcept;
    bool SetFantasyFontFamily(const utf8_string &family, UScriptCode script) noexcept;
    bool SetPictographFontFamily(const utf8_string &family, UScriptCode script) noexcept;
    bool SetMathFontFamily(const utf8_string &family, UScriptCode script) noexcept;
  };
}
