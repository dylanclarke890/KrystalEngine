/*
 * Copyright (C) 2013-2017 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Fonts/FontGenericFamilies.hpp"
#include "Krystal.Core/Threading/NeverDestroyed.hpp"

namespace krys::boo
{
  using namespace css::GenericFontFamilyNames;

  KRYS_NODISCARD static const utf8_string &EmptyFamily() noexcept
  {
    static MainThreadNeverDestroyed<utf8_string> empty;
    return empty;
  }

  static bool SetGenericFontFamilyForScript(ScriptFontFamilyMap &fontMap, const utf8_string &family,
                                            UScriptCode script) noexcept
  {
    if (family.empty())
    {
      return fontMap.erase(static_cast<int32>(script));
    }
    auto &familyInMap = fontMap[static_cast<int32>(script)];
    if (familyInMap == family)
    {
      return false;
    }
    familyInMap = family;
    return true;
  }

  KRYS_NODISCARD static const utf8_string &GenericFontFamilyForScript(const ScriptFontFamilyMap &fontMap,
                                                                      UScriptCode script) noexcept
  {
    auto it = fontMap.find(static_cast<int32>(script));
    if (it != fontMap.end())
    {
      return it->second;
    }
    // Content using USCRIPT_HAN doesn't tell us if we should be using Simplified Chinese or Traditional
    // Chinese. In the absence of all other signals, we consult with the user's system preferences.
    // FIXME: There are no user language preferences, so Traditional Chinese is used.
    if (script == USCRIPT_HAN)
    {
      it = fontMap.find(static_cast<int32>(USCRIPT_TRADITIONAL_HAN));
      if (it != fontMap.end())
      {
        return it->second;
      }
    }
    if (script != USCRIPT_COMMON)
    {
      return GenericFontFamilyForScript(fontMap, USCRIPT_COMMON);
    }
    return EmptyFamily();
  }

  FontGenericFamilies::FontGenericFamilies() noexcept = default;

  bool FontGenericFamilies::IsEmpty() const noexcept
  {
    return _standardFontFamilyMap.empty() && _serifFontFamilyMap.empty() && _fixedFontFamilyMap.empty()
           && _sansSerifFontFamilyMap.empty() && _cursiveFontFamilyMap.empty()
           && _fantasyFontFamilyMap.empty() && _pictographFontFamilyMap.empty() && _mathFontFamilyMap.empty();
  }

  const utf8_string &FontGenericFamilies::StandardFontFamily(UScriptCode script) const noexcept
  {
    return GenericFontFamilyForScript(_standardFontFamilyMap, script);
  }

  const utf8_string &FontGenericFamilies::FixedFontFamily(UScriptCode script) const noexcept
  {
    return GenericFontFamilyForScript(_fixedFontFamilyMap, script);
  }

  const utf8_string &FontGenericFamilies::SerifFontFamily(UScriptCode script) const noexcept
  {
    return GenericFontFamilyForScript(_serifFontFamilyMap, script);
  }

  const utf8_string &FontGenericFamilies::SansSerifFontFamily(UScriptCode script) const noexcept
  {
    return GenericFontFamilyForScript(_sansSerifFontFamilyMap, script);
  }

  const utf8_string &FontGenericFamilies::CursiveFontFamily(UScriptCode script) const noexcept
  {
    return GenericFontFamilyForScript(_cursiveFontFamilyMap, script);
  }

  const utf8_string &FontGenericFamilies::FantasyFontFamily(UScriptCode script) const noexcept
  {
    return GenericFontFamilyForScript(_fantasyFontFamilyMap, script);
  }

  const utf8_string &FontGenericFamilies::PictographFontFamily(UScriptCode script) const noexcept
  {
    return GenericFontFamilyForScript(_pictographFontFamilyMap, script);
  }

  const utf8_string &FontGenericFamilies::MathFontFamily(UScriptCode script) const noexcept
  {
    return GenericFontFamilyForScript(_mathFontFamilyMap, script);
  }

  const utf8_string *FontGenericFamilies::FontFamily(FamilyNamesIndex family,
                                                     UScriptCode script) const noexcept
  {
    switch (family)
    {
      case FamilyNamesIndex::CursiveFamily:    return &CursiveFontFamily(script);
      case FamilyNamesIndex::FantasyFamily:    return &FantasyFontFamily(script);
      case FamilyNamesIndex::MathFamily:       return &MathFontFamily(script);
      case FamilyNamesIndex::MonospaceFamily:  return &FixedFontFamily(script);
      case FamilyNamesIndex::PictographFamily: return &PictographFontFamily(script);
      case FamilyNamesIndex::SansSerifFamily:  return &SansSerifFontFamily(script);
      case FamilyNamesIndex::SerifFamily:      return &SerifFontFamily(script);
      case FamilyNamesIndex::StandardFamily:   return &StandardFontFamily(script);
      case FamilyNamesIndex::SystemUiFamily:   return nullptr;
    }
    krys_unreachable();
  }

  bool FontGenericFamilies::SetStandardFontFamily(const utf8_string &family, UScriptCode script) noexcept
  {
    return SetGenericFontFamilyForScript(_standardFontFamilyMap, family, script);
  }

  bool FontGenericFamilies::SetFixedFontFamily(const utf8_string &family, UScriptCode script) noexcept
  {
    return SetGenericFontFamilyForScript(_fixedFontFamilyMap, family, script);
  }

  bool FontGenericFamilies::SetSerifFontFamily(const utf8_string &family, UScriptCode script) noexcept
  {
    return SetGenericFontFamilyForScript(_serifFontFamilyMap, family, script);
  }

  bool FontGenericFamilies::SetSansSerifFontFamily(const utf8_string &family, UScriptCode script) noexcept
  {
    return SetGenericFontFamilyForScript(_sansSerifFontFamilyMap, family, script);
  }

  bool FontGenericFamilies::SetCursiveFontFamily(const utf8_string &family, UScriptCode script) noexcept
  {
    return SetGenericFontFamilyForScript(_cursiveFontFamilyMap, family, script);
  }

  bool FontGenericFamilies::SetFantasyFontFamily(const utf8_string &family, UScriptCode script) noexcept
  {
    return SetGenericFontFamilyForScript(_fantasyFontFamilyMap, family, script);
  }

  bool FontGenericFamilies::SetPictographFontFamily(const utf8_string &family, UScriptCode script) noexcept
  {
    return SetGenericFontFamilyForScript(_pictographFontFamilyMap, family, script);
  }

  bool FontGenericFamilies::SetMathFontFamily(const utf8_string &family, UScriptCode script) noexcept
  {
    return SetGenericFontFamilyForScript(_mathFontFamilyMap, family, script);
  }
}
