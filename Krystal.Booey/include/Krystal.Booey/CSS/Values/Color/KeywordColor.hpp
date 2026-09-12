#pragma once

#include "Krystal.Booey/CSS/Values/Color/ColorType.hpp"
#include "Krystal.Booey/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.Core/Color/Color.hpp"

namespace krys::boo::css
{
  struct PlatformColorResolutionState;

  struct KeywordColor
  {
    CSSValueId valueID;

    bool operator==(const KeywordColor &) const = default;
  };

  KRYS_NODISCARD bool IsAbsoluteColorKeyword(CSSValueId keyword) noexcept;

  KRYS_NODISCARD bool IsCurrentColorKeyword(CSSValueId keyword) noexcept;

  KRYS_NODISCARD bool IsSystemColorKeyword(CSSValueId keyword) noexcept;

  KRYS_NODISCARD bool IsDeprecatedSystemColorKeyword(CSSValueId keyword) noexcept;

  KRYS_NODISCARD bool IsColorKeyword(CSSValueId) noexcept;

  KRYS_NODISCARD bool IsColorKeyword(CSSValueId keyword, ColorType types) noexcept;

  KRYS_NODISCARD krys::Color ColorFromAbsoluteKeyword(CSSValueId) noexcept;

  // KRYS_NODISCARD krys::Color ColorFromKeyword(CSSValueId, OptionSet<StyleColorOptions>) noexcept;

  KRYS_NODISCARD krys::Color CreateColor(const KeywordColor &, PlatformColorResolutionState &) noexcept;

  KRYS_NODISCARD bool ContainsCurrentColor(const KeywordColor &) noexcept;

  KRYS_NODISCARD bool ContainsColorSchemeDependentColor(const KeywordColor &) noexcept;
}