#include "Krystal.Booey/CSS/Values/Color/LightDarkColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  bool LightDarkColor::operator==(const LightDarkColor &) const noexcept = default;

  boo::Color CreateColor(const LightDarkColor &unresolved, PlatformColorResolutionState &state) noexcept
  {
    if (!state.Appearance)
    {
      return {};
    }

    PlatformColorResolutionStateNester nester {state};

    switch (*state.Appearance)
    {
      case LightDarkColorAppearance::Light: return CreateColor(unresolved.LightColor, state);
      case LightDarkColorAppearance::Dark:  return CreateColor(unresolved.DarkColor, state);
    }

    krys_debug_assert_msg(false, "Unknown light-dark() appearance.");
    return {};
  }

  bool ContainsCurrentColor(const LightDarkColor &unresolved) noexcept
  {
    return ContainsCurrentColor(unresolved.LightColor) || ContainsCurrentColor(unresolved.DarkColor);
  }

  void Serialiser<LightDarkColor>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                              const LightDarkColor &value) noexcept
  {
    builder.Append(u8"light-dark(");
    Serialise(builder, context, value.LightColor);
    builder.Append(u8", ");
    Serialise(builder, context, value.DarkColor);
    builder.Append(u8')');
  }

  void ComputedStyleDependenciesCollector<LightDarkColor>::operator()(ComputedStyleDependencies &dependencies,
                                                                      const LightDarkColor &value) noexcept
  {
    CollectComputedStyleDependencies(dependencies, value.LightColor);
    CollectComputedStyleDependencies(dependencies, value.DarkColor);
  }

  IterationStatus
    ValueChildrenVisitor<LightDarkColor>::operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                                                     const LightDarkColor &value) noexcept
  {
    if (VisitValueChildren(func, value.LightColor) == IterationStatus::Done)
    {
      return IterationStatus::Done;
    }
    if (VisitValueChildren(func, value.DarkColor) == IterationStatus::Done)
    {
      return IterationStatus::Done;
    }
    return IterationStatus::Continue;
  }
}
