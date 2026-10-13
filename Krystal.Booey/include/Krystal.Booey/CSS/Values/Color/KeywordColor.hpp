#pragma once

#include "Krystal.Booey/CSS/Values/Color/ColorType.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorOptions.hpp"

namespace krys::boo::css
{
  struct PlatformColorResolutionState;

  struct KeywordColor
  {
    ValueId Value;

    KRYS_NODISCARD constexpr bool operator==(const KeywordColor &) const noexcept = default;
  };

  KRYS_NODISCARD bool IsAbsoluteColorKeyword(ValueId) noexcept;
  KRYS_NODISCARD bool IsCurrentColorKeyword(ValueId) noexcept;
  KRYS_NODISCARD bool IsSystemColorKeyword(ValueId) noexcept;
  KRYS_NODISCARD bool IsDeprecatedSystemColorKeyword(ValueId) noexcept;
  KRYS_NODISCARD bool IsColorKeyword(ValueId) noexcept;
  KRYS_NODISCARD bool IsColorKeyword(ValueId, ColorType allowedColorTypes) noexcept;

  KRYS_NODISCARD boo::Color ColorFromAbsoluteKeyword(ValueId) noexcept;
  KRYS_NODISCARD boo::Color ColorFromKeyword(ValueId, style::ColorOptions) noexcept;

  KRYS_NODISCARD boo::Color CreateColor(const KeywordColor &, PlatformColorResolutionState &) noexcept;
  KRYS_NODISCARD bool ContainsCurrentColor(const KeywordColor &) noexcept;
  KRYS_NODISCARD bool ContainsColorSchemeDependentColor(const KeywordColor &) noexcept;

  template <>
  struct Serialiser<KeywordColor>
  {
    void operator()(CSSOMString &builder, const SerialisationContext &context,
                    const KeywordColor &value) noexcept;
  };

  template <>
  struct ComputedStyleDependenciesCollector<KeywordColor>
  {
    constexpr void operator()(ComputedStyleDependencies &, const KeywordColor &) noexcept
    {
    }
  };

  template <>
  struct ValueChildrenVisitor<KeywordColor>
  {
    constexpr IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &,
                                         const KeywordColor &) noexcept
    {
      return IterationStatus::Continue;
    }
  };
}
