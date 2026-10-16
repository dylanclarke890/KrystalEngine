#include "Krystal.Booey/CSS/Values/Color/ContrastColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/ContrastColorResolver.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  bool ContrastColor::operator==(const ContrastColor &) const noexcept = default;

  boo::Color CreateColor(const ContrastColor &unresolved, PlatformColorResolutionState &state) noexcept
  {
    PlatformColorResolutionStateNester nester {state};
    return Resolve(ContrastColorResolver {CreateColor(unresolved.Color, state)});
  }

  bool ContainsCurrentColor(const ContrastColor &unresolved) noexcept
  {
    return ContainsCurrentColor(unresolved.Color);
  }

  bool ContainsColorSchemeDependentColor(const ContrastColor &unresolved) noexcept
  {
    return ContainsColorSchemeDependentColor(unresolved.Color);
  }

  void Serialiser<ContrastColor>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                             const ContrastColor &value) noexcept
  {
    builder.Append(u8"contrast-color(");
    Serialise(builder, context, value.Color);
    builder.Append(u8')');
  }

  void ComputedStyleDependenciesCollector<ContrastColor>::operator()(ComputedStyleDependencies &dependencies,
                                                                     const ContrastColor &value) noexcept
  {
    CollectComputedStyleDependencies(dependencies, value.Color);
  }

  IterationStatus
    ValueChildrenVisitor<ContrastColor>::operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                                                    const ContrastColor &value) noexcept
  {
    return VisitValueChildren(func, value.Color);
  }
}
