#include "Krystal.Booey/Style/Values/Color/DynamicRangeLimitMix.hpp"
#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimitMix.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Color/DynamicRangeLimit.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  void AddWeightedLimitTo(DynamicRangeLimitMixFunction &addingTo, const DynamicRangeLimit &limit,
                          const DynamicRangeLimitMixPercentage &mixPercentage) noexcept
  {
    krys::SwitchOn(
      limit,
      [&](const css::keywords::Standard &)
      {
        addingTo->Standard.Value += mixPercentage.Value; /* implicit multiplication by (100 / 100) elided */
      },
      [&](const css::keywords::Constrained &)
      {
        addingTo->Constrained.Value +=
          mixPercentage.Value; /* implicit multiplication by (100 / 100) elided */
      },
      [&](const css::keywords::NoLimit &)
      {
        addingTo->NoLimit.Value += mixPercentage.Value; /* implicit multiplication by (100 / 100) elided */
      },
      [&](const DynamicRangeLimitMixFunction &innerMix)
      {
        if (!IsValueZero(innerMix->Standard))
        {
          addingTo->Standard.Value += mixPercentage.Value * (innerMix->Standard.Value / 100.0);
        }
        if (!IsValueZero(innerMix->Constrained))
        {
          addingTo->Constrained.Value += mixPercentage.Value * (innerMix->Constrained.Value / 100.0);
        }
        if (!IsValueZero(innerMix->NoLimit))
        {
          addingTo->NoLimit.Value += mixPercentage.Value * (innerMix->NoLimit.Value / 100.0);
        }
      });
  }

  // MARK: - Conversion

  css::DynamicRangeLimitMixFunction
    ToCSS<DynamicRangeLimitMixFunction>::operator()(const DynamicRangeLimitMixFunction &mix,
                                                    const RenderStyle &style) noexcept
  {
    css::DynamicRangeLimitMixFunction result;
    if (!IsValueZero(mix.Parameters.Standard))
    {
      result->Parameters.Value.emplace_back(css::DynamicRangeLimit {css::keywords::Standard {}},
                                            ConvertToCSS(mix.Parameters.Standard, style));
    }
    if (!IsValueZero(mix.Parameters.Constrained))
    {
      result->Parameters.Value.emplace_back(css::DynamicRangeLimit {css::keywords::Constrained {}},
                                            ConvertToCSS(mix.Parameters.Constrained, style));
    }
    if (!IsValueZero(mix.Parameters.NoLimit))
    {
      result->Parameters.Value.emplace_back(css::DynamicRangeLimit {css::keywords::NoLimit {}},
                                            ConvertToCSS(mix.Parameters.NoLimit, style));
    }
    return result;
  }

  DynamicRangeLimitMixFunction
    ToStyle<css::DynamicRangeLimitMixFunction>::operator()(const css::DynamicRangeLimitMixFunction &mix,
                                                           const BuilderState &state) noexcept
  {
    return this->operator()(mix, state.CssToLengthConversionData());
  }

  DynamicRangeLimitMixFunction ToStyle<css::DynamicRangeLimitMixFunction>::operator()(
    const css::DynamicRangeLimitMixFunction &mix, const css::ToLengthConversionData &conversionData) noexcept
  {
    // https://drafts.csswg.org/css-color-hdr/#computing-dynamic-range-limit
    // This function implements steps 1 - 4 of the algorithm for computing the dynamic
    // range limit value. The final 2 steps are by `ToStyle<css::DynamicRangeLimit>`.

    struct ComputedMixComponent
    {
      DynamicRangeLimit Limit;
      DynamicRangeLimitMixPercentage MixPercentage;
    };

    // 1. Let v1, ..., vN be the computed values for the parameters to be mixed
    // 2. Let p1, ..., pN be the mixing percentages, normalized to sum to 100%.

    List<ComputedMixComponent> computedMixComponents;
    computedMixComponents.reserve(mix->Parameters.size());
    for (const auto &componentCSS : mix->Parameters)
    {
      computedMixComponents.push_back({ConvertToStyle(get<0>(componentCSS), conversionData),
                                       ConvertToStyle(get<1>(componentCSS), conversionData)});
    }

    // If the percentages do not sum to 100%, scale all the mix percentages so they do, normalizing the
    // values.
    DynamicRangeLimitMixPercentage mixPercentageSum = 0;
    for (const auto &[limit, mixPercentage] : computedMixComponents)
    {
      mixPercentageSum.Value += mixPercentage.Value;
    }

    if (mixPercentageSum != 100_css_percentage)
    {
      for (auto &component : computedMixComponents)
      {
        component.MixPercentage.Value *= 100.0 / mixPercentageSum.Value;
      }
    }

    // 3. Define the contributing percentages as:
    //   - Let p1_standard,...,pN_standard be the percentages for standard in v1,...,vN
    //   - Let p1_constrained,...,pN_constrained be the percentages for constrained in v1,...,vN
    //   - Let p1_no_limit,...,pN_no_limit be the percentages for no-limit in v1,...,vN
    // 4. Compute the weighted sums as:
    //   - p_standard=(p1_standard*p1+...+pN_standard*pN)/100.
    //   - p_constrained=(p1_constrained*p1+...+pN_constrained*pN)/100.
    //   - p_no_limit=(p1_no_limit*p1+...+pN_no_limit*pN)/100.

    DynamicRangeLimitMixFunction function {
      .Parameters = {0_css_percentage, 0_css_percentage, 0_css_percentage}};
    for (const auto &[limit, mixPercentage] : computedMixComponents)
    {
      AddWeightedLimitTo(function, limit, mixPercentage);
    }

    // NOTE: These last two steps are performed by `ToStyle<css::DynamicRangeLimit>`.
    // 5. If p_standard, p_constrained, or p_no_limit equals 100%, then the computed value is standard,
    //    constrained, or no-limit, respectively.
    // 6. Otherwise, the computed value is dynamic-range-limit-mix(), with parameters standard, constrained,
    // and
    //    no-limit, in that order, and percentages p_standard, p_constrained, and p_no_limit, omitting
    //    parameters with a percentage equal to 0%.

    return function;
  }
}
