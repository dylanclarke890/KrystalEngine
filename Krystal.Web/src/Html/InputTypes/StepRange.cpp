#include "Krystal.Web/Html/InputTypes/StepRange.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Web/Html/Parser/ParserIdioms.hpp"
#include <algorithm>
#include <cfloat>

namespace krys::web::html
{
  StepRange::StepRange(const Decimal &stepBase, RangeLimitations rangeLimitations, const Decimal &minimum,
                       const Decimal &maximum, const Decimal &step, const StepDescription &stepDescription,
                       IsReversible isReversible) noexcept
      : _maximum(maximum), _minimum(minimum), _step(step.IsFinite() ? step : Decimal(1)),
        _stepBase(stepBase.IsFinite() ? stepBase : Decimal(1)), _stepDescription(stepDescription),
        _hasRangeLimitations(rangeLimitations == RangeLimitations::Valid), _hasStep(step.IsFinite()),
        _isReversible(isReversible == IsReversible::Yes)
  {
    krys_debug_assert(_maximum.IsFinite());
    krys_debug_assert(_minimum.IsFinite());
    krys_debug_assert(_step.IsFinite());
    krys_debug_assert(_stepBase.IsFinite());
  }

  Decimal StepRange::AcceptableError() const noexcept
  {
    // A value within a float's precision of a step matches it: the specification gives no tolerance, and
    // Chrome uses this one.
    static const Decimal twoPowerOfFloatMantissaBits(Decimal::Positive, 0, uint64 {1} << FLT_MANT_DIG);
    return _stepDescription.StepValueShouldBe == StepValueShouldBeReal ? _step / twoPowerOfFloatMantissaBits
                                                                       : Decimal(0);
  }

  Decimal StepRange::AlignValueForStep(const Decimal &currentValue, const Decimal &newValue) const noexcept
  {
    static const Decimal tenPowerOf21(Decimal::Positive, 21, 1);
    if (newValue >= tenPowerOf21)
    {
      return newValue;
    }

    return StepMismatch(currentValue) ? newValue : RoundByStep(newValue, _stepBase);
  }

  Decimal StepRange::ClampValue(const Decimal &value) const noexcept
  {
    const Decimal inRangeValue = std::max(_minimum, std::min(value, _maximum));
    if (!_hasStep)
    {
      return inRangeValue;
    }
    // To the nearest step: the step base plus a multiple of the step.
    const Decimal roundedValue = RoundByStep(inRangeValue, _stepBase);
    const Decimal clampedValue = roundedValue > _maximum
                                   ? roundedValue - _step
                                   : (roundedValue < _minimum ? roundedValue + _step : roundedValue);
    // clampedValue can be outside of [_minimum, _maximum] if _step is huge.
    if (clampedValue < _minimum || clampedValue > _maximum)
    {
      return inRangeValue;
    }
    return clampedValue;
  }

  Decimal StepRange::ParseStep(AnyStepHandling anyStepHandling, const StepDescription &stepDescription,
                               Maybe<DOMStringView> stepString) noexcept
  {
    if (!stepString || stepString->empty())
    {
      return stepDescription.DefaultValue();
    }

    if (text::IsASCIICaselessEqual(*stepString, u8"any"))
    {
      switch (anyStepHandling)
      {
        case AnyStepHandling::Reject:  return Decimal::NaN();
        case AnyStepHandling::Default: return stepDescription.DefaultValue();
      }
    }

    Decimal step = ParseToDecimalForNumberType(*stepString);
    if (!step.IsFinite() || step <= Decimal(0))
    {
      return stepDescription.DefaultValue();
    }

    switch (stepDescription.StepValueShouldBe)
    {
      case StepValueShouldBeReal: step *= Decimal(stepDescription.StepScaleFactor); break;
      case ParsedStepValueShouldBeInteger:
        // A whole number of steps for date, month and week.
        step = std::max(step.Round(), Decimal(1));
        step *= Decimal(stepDescription.StepScaleFactor);
        break;
      case ScaledStepValueShouldBeInteger:
        // A whole number of milliseconds for datetime-local and time.
        step *= Decimal(stepDescription.StepScaleFactor);
        step = std::max(step.Round(), Decimal(1));
        break;
    }

    krys_debug_assert(step > Decimal(0));
    return step;
  }

  Decimal StepRange::RoundByStep(const Decimal &value, const Decimal &base) const noexcept
  {
    return base + ((value - base) / _step).Round() * _step;
  }

  Decimal StepRange::StepSnappedMaximum() const noexcept
  {
    Decimal base = StepBase();
    Decimal step = _step;
    if (base - step == base || !(base / step).IsFinite())
    {
      return Decimal::NaN();
    }

    Decimal alignedMaximum = base + ((Maximum() - base) / step).Floor() * step;
    if (alignedMaximum > Maximum())
    {
      alignedMaximum -= step;
    }

    if (alignedMaximum < Minimum())
    {
      return Decimal::NaN();
    }

    return alignedMaximum;
  }

  bool StepRange::StepMismatch(const Decimal &valueForCheck) const noexcept
  {
    if (!_hasStep)
    {
      return false;
    }
    if (!valueForCheck.IsFinite())
    {
      return false;
    }
    const Decimal value = (valueForCheck - _stepBase).Abs();
    if (!value.IsFinite())
    {
      return false;
    }
    // Decimal's fractional part size is DBL_MAN_DIG-bit. If the current value is greater than
    // step*2^DBL_MANT_DIG, the following computation for remainder makes no sense.
    static const Decimal twoPowerOfDoubleMantissaBits(Decimal::Positive, 0, uint64 {1} << DBL_MANT_DIG);
    if (value / twoPowerOfDoubleMantissaBits > _step)
    {
      return false;
    }
    // A value that's not a multiple of the step from the step base mismatches it.
    // https://html.spec.whatwg.org/#concept-input-step-mismatch
    const Decimal remainder = (value - _step * (value / _step).Round()).Abs();
    // Within a float's precision of a multiple, it matches (see AcceptableError).
    const Decimal computedAcceptableError = AcceptableError();
    return computedAcceptableError < remainder && remainder < (_step - computedAcceptableError);
  }
}
