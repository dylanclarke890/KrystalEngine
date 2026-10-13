#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Calc/Tree.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCalc.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: Interpolation of base numeric types
  // https://drafts.csswg.org/css-values/#combining-values
  template <Numeric StyleType>
  struct Blending<StyleType>
  {
    KRYS_NODISCARD StyleType Blend(const StyleType &from, const StyleType &to,
                                   const BlendingContext &context) noexcept
    {
      if (!context.Progress && context.IsReplace())
      {
        return from;
      }

      if (context.Progress == 1 && context.IsReplace())
      {
        return to;
      }

      // FIXME: As interpolation may result in a value outside of the range allowed by the
      // primitive, we clamp the value back down to the allowed range. The spec states that
      // in some cases, an accumulated intermediate value should be allowed to be out of the
      // allowed range until after interpolation has completed, but we currently don't have
      // that concept.
      // https://drafts.csswg.org/css-values/#combining-range

      return StyleType {css::ClampToRange<StyleType::Range, typename StyleType::ResolvedValueType>(
        krys::boo::Blend(from.Value, to.Value, context))};
    }
  };

  template <auto R, typename V>
  struct Blending<Length<R, V>>
  {
    using StyleType = Length<R, V>;

    KRYS_NODISCARD StyleType Blend(const StyleType &from, const StyleType &to,
                                   const BlendingContext &context) noexcept
    {
      if (!context.Progress && context.IsReplace())
      {
        return from;
      }

      if (context.Progress == 1 && context.IsReplace())
      {
        return to;
      }

      // FIXME: As interpolation may result in a value outside of the range allowed by the
      // primitive, we clamp the value back down to the allowed range. The spec states that
      // in some cases, an accumulated intermediate value should be allowed to be out of the
      // allowed range until after interpolation has completed, but we currently don't have
      // that concept.
      // https://drafts.csswg.org/css-values/#combining-range

      return StyleType {css::ClampToRange<StyleType::Range, typename StyleType::ResolvedValueType>(
        krys::boo::Blend(from.UnresolvedValue(), to.UnresolvedValue(), context))};
    }
  };

  // MARK: Interpolation of mixed numeric types
  // https://drafts.csswg.org/css-values/#combine-mixed
  template <auto R, typename V>
  struct Blending<LengthPercentage<R, V>>
  {
    using StyleType = LengthPercentage<R, V>;
    using Length = typename StyleType::Dimension;
    using Percentage = typename StyleType::Percentage;
    using Calc = typename StyleType::Calc;

    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const StyleType &from,
                                                                      const StyleType &to) noexcept
    {
      return krys::HoldsAlternative<Calc>(from) || krys::HoldsAlternative<Calc>(to)
             || from.index() != to.index();
    }

    KRYS_NODISCARD StyleType Blend(const StyleType &from, const StyleType &to,
                                   const BlendingContext &context) noexcept
    {
      // Interpolation of dimension-percentage value combinations (e.g. <length-percentage>,
      // <frequency-percentage>, <angle-percentage>, <time-percentage> or equivalent notations) is defined as:
      //
      //  - equivalent to interpolation of <length> if both VA and VB are pure <length> values
      //  - equivalent to interpolation of <percentage> if both VA and VB are pure <percentage> values
      //  - equivalent to converting both values into a calc() expression representing the sum of the
      //    dimension type and a percentage (each possibly zero) and interpolating each component
      //    individually (as a <length>/<frequency>/<angle>/<time> and as a <percentage>, respectively)

      if (krys::HoldsAlternative<Calc>(from) || krys::HoldsAlternative<Calc>(to)
          || (from.index() != to.index()))
      {
        if (context.CompositeOperation != CompositeOperation::Replace)
        {
          return Calc {calc::Add(CopyCalculation(from), CopyCalculation(to))};
        }

        bool fromIsZero = from.IsKnownZero();
        bool toIsZero = to.IsKnownZero();

        // 0% to 0px -> calc(0px + 0%) to calc(0px + 0%) -> 0px
        // 0px to 0% -> calc(0px + 0%) to calc(0px + 0%) -> 0px
        if (fromIsZero && toIsZero)
        {
          return 0_css_px;
        }

        if (!krys::HoldsAlternative<Calc>(to) && !krys::HoldsAlternative<Percentage>(from)
            && (context.Progress == 1 || fromIsZero))
        {
          if (krys::HoldsAlternative<Length>(to))
          {
            return style::Blend(Length(0_css_px), get<Length>(to), context);
          }

          return style::Blend(Percentage(0_css_percentage), get<Percentage>(to), context);
        }

        if (!krys::HoldsAlternative<Calc>(from) && !krys::HoldsAlternative<Percentage>(to)
            && (!context.Progress || toIsZero))
        {
          if (krys::HoldsAlternative<Length>(from))
          {
            return style::Blend(get<Length>(from), Length(0_css_px), context);
          }

          return style::Blend(get<Percentage>(from), Percentage(0_css_percentage), context);
        }

        return Calc {calc::BlendChildren(CopyCalculation(from), CopyCalculation(to), context.Progress)};
      }

      if (!context.Progress && context.IsReplace())
      {
        return from;
      }

      if (context.Progress == 1 && context.IsReplace())
      {
        return to;
      }

      if (krys::HoldsAlternative<Length>(to))
      {
        return style::Blend(get<Length>(from), get<Length>(to), context);
      }

      return style::Blend(get<Percentage>(from), get<Percentage>(to), context);
    }
  };

  // `NumberOrPercentageResolvedToNumber<nR, pR, V>` forwards to `Number<nR, V>`.
  template <auto nR, auto pR, typename V>
  struct Blending<NumberOrPercentageResolvedToNumber<nR, pR, V>>
  {
    using StyleType = NumberOrPercentageResolvedToNumber<nR, pR, V>;

    KRYS_NODISCARD bool CanBlend(const StyleType &a, const StyleType &b) noexcept
    {
      return style::CanBlend(a.Value, b.Value);
    }

    KRYS_NODISCARD StyleType Blend(const StyleType &a, const StyleType &b,
                                   const BlendingContext &context) noexcept
    {
      return style::Blend(a.Value, b.Value, context);
    }
  };
}
