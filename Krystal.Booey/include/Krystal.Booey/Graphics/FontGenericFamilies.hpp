#pragma once

#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"
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
    KRYS_NODISCARD const utf8_string *FontFamily(css::WebKitFontFamilyNames::FamilyNamesIndex family,
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
