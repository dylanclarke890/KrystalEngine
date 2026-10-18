#pragma once

#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorDescriptors.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Booey/CSS/Values/Color/RelativeColorResolver.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesEvaluateCalc.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  template <typename Descriptor, size_t Index>
  using RelativeColorComponent = GetCSSColorParseTypeWithCalcAndSymbolsComponentResult<Descriptor, Index>;

  template <typename D>
  struct RelativeColor
  {
    using Descriptor = D;

    css::Color Origin;
    CSSColorParseTypeWithCalcAndSymbols<Descriptor> Components;

    KRYS_NODISCARD bool operator==(const RelativeColor<Descriptor> &) const noexcept = default;
  };

  template <typename Descriptor>
  KRYS_NODISCARD auto
    SimplifyUnevaluatedCalc(const CSSColorParseTypeWithCalcAndSymbols<Descriptor> &components,
                            const ToLengthConversionData &conversionData,
                            const SymbolTable &symbolTable) noexcept
    -> CSSColorParseTypeWithCalcAndSymbols<Descriptor>
  {
    return CSSColorParseTypeWithCalcAndSymbols<Descriptor> {
      SimplifyUnevaluatedCalc(std::get<0>(components), conversionData, symbolTable),
      SimplifyUnevaluatedCalc(std::get<1>(components), conversionData, symbolTable),
      SimplifyUnevaluatedCalc(std::get<2>(components), conversionData, symbolTable),
      SimplifyUnevaluatedCalc(std::get<3>(components), conversionData, symbolTable)};
  }

  template <typename Descriptor>
  KRYS_NODISCARD boo::Color CreateColor(const RelativeColor<Descriptor> &unresolved,
                                        PlatformColorResolutionState &state) noexcept
  {
    PlatformColorResolutionStateNester nester {state};

    auto origin = CreateColor(unresolved.Origin, state);
    if (!origin.IsValid())
    {
      return {};
    }

    auto resolver =
      RelativeColorResolver<Descriptor> {.Origin = krys::move(origin), .Components = unresolved.Components};

    if (state.ConversionData)
    {
      return Resolve(krys::move(resolver), *state.ConversionData);
    }

    if (!ComponentsRequireConversionData<Descriptor>(resolver.Components))
    {
      return ResolveNoConversionDataRequired(krys::move(resolver));
    }

    return {};
  }

  template <typename Descriptor>
  KRYS_NODISCARD bool ContainsColorSchemeDependentColor(const RelativeColor<Descriptor> &unresolved) noexcept
  {
    return ContainsColorSchemeDependentColor(unresolved.Origin);
  }

  template <typename Descriptor>
  KRYS_NODISCARD bool ContainsCurrentColor(const RelativeColor<Descriptor> &unresolved) noexcept
  {
    return ContainsCurrentColor(unresolved.Origin);
  }

  template <typename D>
  struct Serialiser<RelativeColor<D>>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const RelativeColor<D> &value) noexcept
    {
      if constexpr (D::UsesColorFunctionForSerialisation)
      {
        builder.Append(u8"color(from ");
        Serialise(builder, context, value.Origin);
        builder.Append(u8' ');
        builder.Append(boo::Serialisation(ColourSpaceFor<typename D::ColorType>));
      }
      else
      {
        builder.Append(D::SerialisationFunctionName);
        builder.Append(u8"(from ");
        Serialise(builder, context, value.Origin);
      }

      const auto &[c1, c2, c3, alpha] = value.Components;

      builder.Append(u8' ');
      Serialise(builder, context, c1);
      builder.Append(u8' ');
      Serialise(builder, context, c2);
      builder.Append(u8' ');
      Serialise(builder, context, c3);

      if (alpha)
      {
        builder.Append(u8" / ");
        Serialise(builder, context, *alpha);
      }

      builder.Append(u8')');
    }
  };

  template <typename D>
  struct ComputedStyleDependenciesCollector<RelativeColor<D>>
  {
    void operator()(ComputedStyleDependencies &dependencies, const RelativeColor<D> &value) noexcept
    {
      CollectComputedStyleDependencies(dependencies, value.Origin);
      CollectComputedStyleDependencies(dependencies, std::get<0>(value.Components));
      CollectComputedStyleDependencies(dependencies, std::get<1>(value.Components));
      CollectComputedStyleDependencies(dependencies, std::get<2>(value.Components));
      CollectComputedStyleDependencies(dependencies, std::get<3>(value.Components));
    }
  };

  template <typename D>
  struct ValueChildrenVisitor<RelativeColor<D>>
  {
    IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                               const RelativeColor<D> &value) noexcept
    {
      if (VisitValueChildren(func, value.Origin) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
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
