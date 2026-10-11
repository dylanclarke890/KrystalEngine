#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"

namespace krys::boo::css
{
  struct PlatformColorResolutionState;

  struct ResolvedColor
  {
    boo::Color Value;

    bool operator==(const ResolvedColor &) const noexcept = default;
  };

  KRYS_NODISCARD inline boo::Color CreateColor(const ResolvedColor &color,
                                               PlatformColorResolutionState &) noexcept
  {
    return color.Value;
  }

  KRYS_NODISCARD constexpr bool ContainsColorSchemeDependentColor(const ResolvedColor &) noexcept
  {
    return false;
  }

  KRYS_NODISCARD constexpr bool ContainsCurrentColor(const ResolvedColor &) noexcept
  {
    return false;
  }

  template <>
  struct Serialiser<ResolvedColor>
  {
    void operator()(CSSOMString &builder, const SerialisationContext &context,
                    const ResolvedColor &value) noexcept;
  };

  template <>
  struct ComputedStyleDependenciesCollector<ResolvedColor>
  {
    constexpr void operator()(ComputedStyleDependencies &, const ResolvedColor &) noexcept
    {
    }
  };

  template <>
  struct ValueChildrenVisitor<ResolvedColor>
  {
    constexpr IterationStatus operator()(const Func<IterationStatus(css::Value &)> &,
                                         const ResolvedColor &) noexcept
    {
      return IterationStatus::Continue;
    }
  };
}