#pragma once

#include "Krystal.Booey/CSS/Values/Color/AbsoluteColorResolver.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorDescriptors.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"

namespace krys::boo
{
  class Color;
}

namespace krys::boo::css
{
  template <typename D, size_t Index>
  using AbsoluteColorComponent = GetCSSColorParseTypeWithCalcComponentResult<D, Index>;

  template <typename D>
  struct AbsoluteColor
  {
    using Descriptor = D;

    CSSColorParseTypeWithCalc<Descriptor> Components;

    bool operator==(const AbsoluteColor<Descriptor> &) const = default;
  };

  template <typename D>
  KRYS_NODISCARD boo::Color createColor(const AbsoluteColor<D> &unresolved, PlatformColorResolutionState &state) noexcept
  {
    PlatformColorResolutionStateNester nester {state};

    auto resolver =
      AbsoluteColorResolver<D> {.components = unresolved.components, .nestingLevel = state.nestingLevel};

    if (state.ConversionData)
    {
      return Resolve(krys::move(resolver), *state.ConversionData);
    }

    if (!componentsRequireConversionData<D>(resolver.components))
      return resolveNoConversionDataRequired(WTF::move(resolver));

    return {};
  }

  template <typename D>
  constexpr bool containsColorSchemeDependentColor(const AbsoluteColor<D> &)
  {
    return false;
  }

  template <typename D>
  constexpr bool containsCurrentColor(const AbsoluteColor<D> &)
  {
    return false;
  }

  template <typename D>
  struct Serialise<AbsoluteColor<D>>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const AbsoluteColor<D> &value)
    {
      if constexpr (D::usesColorFunctionForSerialisation)
        builder.append("color("_s, serialization(ColorSpaceFor<typename D::ColorType>), ' ');
      else
        builder.append(D::serializationFunctionName, '(');

      auto [c1, c2, c3, alpha] = value.components;

      serializationForCSS(builder, context, c1);
      builder.append(' ');
      serializationForCSS(builder, context, c2);
      builder.append(' ');
      serializationForCSS(builder, context, c3);

      if (alpha)
      {
        builder.append(" / "_s);
        serializationForCSS(builder, context, *alpha);
      }

      builder.append(')');
    }
  };

  template <typename D>
  struct ComputedStyleDependenciesCollector<AbsoluteColor<D>>
  {
    void operator()(ComputedStyleDependencies &dependencies, const AbsoluteColor<D> &value)
    {
      collectComputedStyleDependencies(dependencies, std::get<0>(value.components));
      collectComputedStyleDependencies(dependencies, std::get<1>(value.components));
      collectComputedStyleDependencies(dependencies, std::get<2>(value.components));
      collectComputedStyleDependencies(dependencies, std::get<3>(value.components));
    }
  };

  template <typename D>
  struct CSSValueChildrenVisitor<AbsoluteColor<D>>
  {
    IterationStatus operator()(NOESCAPE const Function<IterationStatus(CSSValue &)> &func,
                               const AbsoluteColor<D> &value)
    {
      if (visitCSSValueChildren(func, std::get<0>(value.components)) == IterationStatus::Done)
        return IterationStatus::Done;
      if (visitCSSValueChildren(func, std::get<1>(value.components)) == IterationStatus::Done)
        return IterationStatus::Done;
      if (visitCSSValueChildren(func, std::get<2>(value.components)) == IterationStatus::Done)
        return IterationStatus::Done;
      if (visitCSSValueChildren(func, std::get<3>(value.components)) == IterationStatus::Done)
        return IterationStatus::Done;
      return IterationStatus::Continue;
    }
  };
}
