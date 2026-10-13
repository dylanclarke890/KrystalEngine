#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::css::WebKitFontFamilyNames
{
  /// @brief Indexes the generic font family names, in name order.
  enum class FamilyNamesIndex : uint8
  {
    CursiveFamily,
    FantasyFamily,
    MathFamily,
    MonospaceFamily,
    PictographFamily,
    SansSerifFamily,
    SerifFamily,
    StandardFamily,
    SystemUiFamily,
  };

  /// @brief The internal family name for the generic `cursive` family.
  KRYS_NODISCARD const CSSOMStringAtom &CursiveFamily() noexcept;

  /// @brief The internal family name for the generic `fantasy` family.
  KRYS_NODISCARD const CSSOMStringAtom &FantasyFamily() noexcept;

  /// @brief The internal family name for the generic `math` family.
  KRYS_NODISCARD const CSSOMStringAtom &MathFamily() noexcept;

  /// @brief The internal family name for the generic `monospace` family.
  KRYS_NODISCARD const CSSOMStringAtom &MonospaceFamily() noexcept;

  /// @brief The internal family name for the generic `-webkit-pictograph` family.
  KRYS_NODISCARD const CSSOMStringAtom &PictographFamily() noexcept;

  /// @brief The internal family name for the generic `sans-serif` family.
  KRYS_NODISCARD const CSSOMStringAtom &SansSerifFamily() noexcept;

  /// @brief The internal family name for the generic `serif` family.
  KRYS_NODISCARD const CSSOMStringAtom &SerifFamily() noexcept;

  /// @brief The internal family name for the standard (default) family.
  KRYS_NODISCARD const CSSOMStringAtom &StandardFamily() noexcept;

  /// @brief The internal family name for the generic `system-ui` family.
  KRYS_NODISCARD const CSSOMStringAtom &SystemUiFamily() noexcept;

  /// @brief The internal family name at `index`.
  KRYS_NODISCARD const CSSOMStringAtom &FamilyName(FamilyNamesIndex index) noexcept;
}
