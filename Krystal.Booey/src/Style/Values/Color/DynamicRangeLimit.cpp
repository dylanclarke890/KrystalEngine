#include "Krystal.Booey/Style/Values/Color/DynamicRangeLimit.hpp"
#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimit.hpp"
#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimitMix.hpp"
#include "Krystal.Booey/CSS/Values/DynamicRangeLimitValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/PlatformDynamicRangeLimit.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Color/DynamicRangeLimitMix.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  namespace
  {
    // Resolves a `dynamic-range-limit-mix` function value for the `dynamic-range-limit` property.
    KRYS_NODISCARD DynamicRangeLimit Resolve(DynamicRangeLimitMixFunction &&mix) noexcept
    {
      // https://drafts.csswg.org/css-color-hdr/#computing-dynamic-range-limit
      // This function implements steps 5 and 6 of the algorithm for computing the dynamic
      // range limit value. The first 4 steps are in `ToStyle<css::DynamicRangeLimitMixFunction>`.

      if (mix->Standard == 100_css_percentage)
      {
        return DynamicRangeLimit(css::keywords::Standard {});
      }
      if (mix->Constrained == 100_css_percentage)
      {
        return DynamicRangeLimit(css::keywords::Constrained {});
      }
      if (mix->NoLimit == 100_css_percentage)
      {
        return DynamicRangeLimit(css::keywords::NoLimit {});
      }
      return DynamicRangeLimit(krys::move(mix));
    }
  }

  // MARK: - Conversion

  css::DynamicRangeLimit ToCSS<DynamicRangeLimit>::operator()(const DynamicRangeLimit &limit,
                                                              const RenderStyle &style) noexcept
  {
    return krys::SwitchOn(limit, [&](const auto &value) -> css::DynamicRangeLimit
                          { return css::DynamicRangeLimit(ConvertToCSS(value, style)); });
  }

  DynamicRangeLimit ToStyle<css::DynamicRangeLimit>::operator()(const css::DynamicRangeLimit &limit,
                                                                const BuilderState &state) noexcept
  {
    return this->operator()(limit, state.CssToLengthConversionData());
  }

  DynamicRangeLimit
    ToStyle<css::DynamicRangeLimit>::operator()(const css::DynamicRangeLimit &limit,
                                                const css::ToLengthConversionData &conversionData) noexcept
  {
    return krys::SwitchOn(
      limit, [&]<css::ValueId Id>(const css::KeywordValueConstant<Id> &keyword) -> DynamicRangeLimit
      { return DynamicRangeLimit(keyword); },
      [&](const css::DynamicRangeLimitMixFunction &mix) -> DynamicRangeLimit
      { return Resolve(ConvertToStyle(mix, conversionData)); });
  }

  Ref<css::Value> CSSValueCreation<DynamicRangeLimit>::operator()(css::ValuePool &, const RenderStyle &style,
                                                                  const DynamicRangeLimit &value) noexcept
  {
    return css::DynamicRangeLimitValue::Create(ConvertToCSS(value, style));
  }

  DynamicRangeLimit CSSValueConversion<DynamicRangeLimit>::operator()(BuilderState &state,
                                                                      const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Standard:    return css::keywords::Standard {};
        case css::ValueId::Constrained: return css::keywords::Constrained {};
        case css::ValueId::NoLimit:     return css::keywords::NoLimit {};
        default:                        break;
      }

      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::NoLimit {};
    }

    auto *dynamicRangeLimit = RequiredDowncast<css::DynamicRangeLimitValue>(state, value);
    if (!dynamicRangeLimit)
    {
      return css::keywords::NoLimit {};
    }
    return ConvertToStyle(dynamicRangeLimit->GetDynamicRangeLimit(), state);
  }

  // MARK: - Serialisation

  void Serialiser<DynamicRangeLimit>::operator()(StringBuilder &builder,
                                                 const css::SerialisationContext &context,
                                                 const RenderStyle &style,
                                                 const DynamicRangeLimit &value) noexcept
  {
    css::Serialise(builder, context, ConvertToCSS(value, style));
  }

  // MARK: - Blending

  DynamicRangeLimit Blending<DynamicRangeLimit>::Blend(const DynamicRangeLimit &from,
                                                       const DynamicRangeLimit &to,
                                                       const BlendingContext &context) noexcept
  {
    // Blending is defined as "by dynamic-range-limit-mix()". This requires computing the equivalent of:
    //
    //   dynamic-range-limit-mix(`from` (1 - context.Progress) * 100%, `to` context.Progress * 100%)
    //
    // and then resolving that as we would when converting from css::DynamicRangeLimit. Rather than
    // construct a full fledged css::DynamicRangeLimitMixFunction, we can utilize the weighted sum
    // addition functionality to construct just what we need and then resolve that directly.

    DynamicRangeLimitMixPercentage fromMixPercentage = (1.0 - context.Progress) * 100.0;
    DynamicRangeLimitMixPercentage toMixPercentage = (context.Progress) * 100.0;

    DynamicRangeLimitMixFunction function {
      .Parameters = {0_css_percentage, 0_css_percentage, 0_css_percentage}};
    AddWeightedLimitTo(function, from, fromMixPercentage);
    AddWeightedLimitTo(function, to, toMixPercentage);

    return Resolve(krys::move(function));
  }

  // MARK: - Conversion to platform object

  PlatformDynamicRangeLimit DynamicRangeLimit::ToPlatformDynamicRangeLimit() const noexcept
  {
    return krys::SwitchOn(*this,
                          [&]<typename Kind>(const Kind &kind) -> PlatformDynamicRangeLimit
                          {
                            if constexpr (SameType<Kind, css::keywords::Standard>)
                            {
                              return PlatformDynamicRangeLimit::Standard();
                            }
                            else if constexpr (SameType<Kind, css::keywords::Constrained>)
                            {
                              return PlatformDynamicRangeLimit::Constrained();
                            }
                            else if constexpr (SameType<Kind, css::keywords::NoLimit>)
                            {
                              return PlatformDynamicRangeLimit::NoLimit();
                            }
                            else if constexpr (SameType<Kind, DynamicRangeLimitMixFunction>)
                            {
                              return PlatformDynamicRangeLimit(static_cast<float>(kind->Standard.Value),
                                                               static_cast<float>(kind->Constrained.Value),
                                                               static_cast<float>(kind->NoLimit.Value));
                            }
                          });
  }
}
