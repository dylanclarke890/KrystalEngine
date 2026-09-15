#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorType.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{
  struct PlatformColorResolutionState;

  struct KeywordColor
  {
    ValueId valueID;

    bool operator==(const KeywordColor &) const = default;
  };

  KRYS_NODISCARD bool IsAbsoluteColorKeyword(ValueId keyword) noexcept;

  KRYS_NODISCARD bool IsCurrentColorKeyword(ValueId keyword) noexcept;

  KRYS_NODISCARD bool IsSystemColorKeyword(ValueId keyword) noexcept;

  KRYS_NODISCARD bool IsDeprecatedSystemColorKeyword(ValueId keyword) noexcept;

  KRYS_NODISCARD bool IsColorKeyword(ValueId) noexcept;

  KRYS_NODISCARD bool IsColorKeyword(ValueId keyword, ColorType types) noexcept;

  KRYS_NODISCARD boo::Color ColorFromAbsoluteKeyword(ValueId) noexcept;

  // KRYS_NODISCARD krys::Color ColorFromKeyword(ValueId, OptionSet<StyleColorOptions>) noexcept;

  KRYS_NODISCARD boo::Color CreateColor(const KeywordColor &, PlatformColorResolutionState &) noexcept;

  KRYS_NODISCARD bool ContainsCurrentColor(const KeywordColor &) noexcept;

  KRYS_NODISCARD bool ContainsColorSchemeDependentColor(const KeywordColor &) noexcept;
}