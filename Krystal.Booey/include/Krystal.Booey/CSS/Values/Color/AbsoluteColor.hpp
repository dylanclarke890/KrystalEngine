#pragma once

#include "Krystal.Booey/CSS/Values/Color/AbsoluteColorResolver.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorDescriptors.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"

namespace krys::boo::css
{
  template <typename D, size_t Index>
  using AbsoluteColorComponent = GetCSSColorParseTypeWithCalcComponentResult<D, Index>;

  template <typename D>
  struct AbsoluteColor
  {
    using Descriptor = D;

    CSSColorParseTypeWithCalc<Descriptor> Components;

    KRYS_NODISCARD bool operator==(const AbsoluteColor<Descriptor> &) const noexcept = default;
  };

  template <typename D>
  KRYS_NODISCARD boo::Color CreateColor(const AbsoluteColor<D> &unresolved,
                                        PlatformColorResolutionState &state) noexcept
  {
    PlatformColorResolutionStateNester nester {state};

    auto resolver =
      AbsoluteColorResolver<D> {.Components = unresolved.Components, .NestingLevel = state.NestingLevel};

    if (state.ConversionData)
    {
      return Resolve(krys::move(resolver), *state.ConversionData);
    }

    if (!ComponentsRequireConversionData<D>(resolver.Components))
    {
      return ResolveNoConversionDataRequired(krys::move(resolver));
    }

    return {};
  }

  template <typename D>
  KRYS_NODISCARD constexpr bool ContainsColorSchemeDependentColor(const AbsoluteColor<D> &) noexcept
  {
    return false;
  }

  template <typename D>
  KRYS_NODISCARD constexpr bool ContainsCurrentColor(const AbsoluteColor<D> &) noexcept
  {
    return false;
  }

  template <typename D>
  struct Serialiser<AbsoluteColor<D>>
  {
    void operator()(CSSOMString &builder, const SerialisationContext &context,
                    const AbsoluteColor<D> &value) noexcept
    {
      if constexpr (D::UsesColorFunctionForSerialisation)
      {
        builder.append(u8"color(");
        builder.append(boo::Serialisation(ColorSpaceFor<typename D::ColorType>));
        builder.push_back(u8' ');
      }
      else
      {
        builder.append(D::SerialisationFunctionName);
        builder.push_back(u8'(');
      }

      const auto &[c1, c2, c3, alpha] = value.Components;

      Serialise(builder, context, c1);
      builder.push_back(u8' ');
      Serialise(builder, context, c2);
      builder.push_back(u8' ');
      Serialise(builder, context, c3);

      if (alpha)
      {
        builder.append(u8" / ");
        Serialise(builder, context, *alpha);
      }

      builder.push_back(u8')');
    }
  };

  template <typename D>
  struct ComputedStyleDependenciesCollector<AbsoluteColor<D>>
  {
    void operator()(ComputedStyleDependencies &dependencies, const AbsoluteColor<D> &value) noexcept
    {
      CollectComputedStyleDependencies(dependencies, std::get<0>(value.Components));
      CollectComputedStyleDependencies(dependencies, std::get<1>(value.Components));
      CollectComputedStyleDependencies(dependencies, std::get<2>(value.Components));
      CollectComputedStyleDependencies(dependencies, std::get<3>(value.Components));
    }
  };

  template <typename D>
  struct ValueChildrenVisitor<AbsoluteColor<D>>
  {
    IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                               const AbsoluteColor<D> &value) noexcept
    {
      if (VisitValueChildren(func, std::get<0>(value.Components)) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      if (VisitValueChildren(func, std::get<1>(value.Components)) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      if (VisitValueChildren(func, std::get<2>(value.Components)) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      if (VisitValueChildren(func, std::get<3>(value.Components)) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      return IterationStatus::Continue;
    }
  };
}
