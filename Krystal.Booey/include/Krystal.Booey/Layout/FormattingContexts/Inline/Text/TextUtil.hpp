#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  class FontCascade;
  class RenderStyle;
}

namespace krys::boo::layout
{
  /// @brief Text measurement and analysis for inline layout.
  // FIXME: The rest (width, break positions, fallback fonts, directionality, ...), with inline items.
  class TextUtil
  {
  public:
    TextUtil() = delete;

    KRYS_NODISCARD static bool CanUseSimplifiedTextMeasuring(utf8_stringview textContent,
                                                             const FontCascade &fontCascade,
                                                             bool whitespaceIsCollapsed,
                                                             const RenderStyle *firstLineStyle) noexcept;
    KRYS_NODISCARD static bool HasPositionDependentContentWidth(utf8_stringview textContent) noexcept;
    KRYS_NODISCARD static bool ContainsStrongDirectionalityText(utf8_stringview text) noexcept;
    KRYS_NODISCARD static bool IsStrongDirectionalityCharacter(char32 character) noexcept;
  };
}
