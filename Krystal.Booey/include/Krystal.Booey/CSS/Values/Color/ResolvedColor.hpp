#pragma once

#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  struct PlatformColorResolutionState;

  struct ResolvedColor
  {
    boo::Color Value;

    KRYS_NODISCARD bool operator==(const ResolvedColor &) const noexcept = default;
  };

  KRYS_NODISCARD inline boo::Color CreateColor(const ResolvedColor &unresolved,
                                               PlatformColorResolutionState &) noexcept
  {
    return unresolved.Value;
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
    void operator()(StringBuilder &builder, const SerialisationContext &context,
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
    constexpr IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &,
                                         const ResolvedColor &) noexcept
    {
      return IterationStatus::Continue;
    }
  };
}
