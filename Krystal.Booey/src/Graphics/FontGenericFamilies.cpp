#include "Krystal.Booey/Graphics/FontGenericFamilies.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo
{
  using namespace css::WebKitFontFamilyNames;

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
