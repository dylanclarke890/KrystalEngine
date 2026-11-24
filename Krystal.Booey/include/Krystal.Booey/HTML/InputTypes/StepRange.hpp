#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/Decimal.hpp"

namespace krys::boo::html
{
  enum class AnyStepHandling : bool
  {
    Reject,
    Default,
  };

  enum class RangeLimitations : bool
  {
    Valid,
    Invalid,
  };

  /// @brief The range and step of a number-like input: its minimum, maximum, step and step base.
  class StepRange
  {
  public:
    enum StepValueShouldBe
    {
      StepValueShouldBeReal,
      ParsedStepValueShouldBeInteger,
      ScaledStepValueShouldBeInteger,
    };

    struct StepDescription
    {
      int DefaultStep {1};
      int DefaultStepBase {0};
      int StepScaleFactor {1};
      StepValueShouldBe StepValueShouldBe {StepValueShouldBeReal};

      /// @brief The type's default step, scaled to its units.
      KRYS_NODISCARD Decimal DefaultValue() const noexcept
      {
        return Decimal(DefaultStep * StepScaleFactor);
      }
    };

    enum class IsReversible : bool
    {
      No,
      Yes,
    };

  private:
    Decimal _maximum {100}; // maximum must be >= minimum.
    Decimal _minimum {0};
    Decimal _step {1};
    Decimal _stepBase {0};
    StepDescription _stepDescription;
    bool _hasRangeLimitations {false};
    bool _hasStep {false};
    bool _isReversible {false};

  public:
    StepRange() noexcept = default;

    StepRange(const Decimal &stepBase, RangeLimitations rangeLimitations, const Decimal &minimum,
              const Decimal &maximum, const Decimal &step, const StepDescription &stepDescription,
              IsReversible isReversible = IsReversible::No) noexcept;

    /// @brief How far a value can be from a step and still match it: a float's precision of the step, for
    /// types with real steps.
    KRYS_NODISCARD Decimal AcceptableError() const noexcept;

    /// @brief @p newValue rounded to a step, unless @p currentValue was off the step already or @p newValue
    /// is 1e21 or more.
    KRYS_NODISCARD Decimal AlignValueForStep(const Decimal &currentValue,
                                             const Decimal &newValue) const noexcept;

    /// @brief @p value clamped to the range and rounded to a step, staying in the range.
    KRYS_NODISCARD Decimal ClampValue(const Decimal &value) const noexcept;

    /// @brief Whether there's an allowed value step (step isn't `any`).
    KRYS_NODISCARD bool HasStep() const noexcept
    {
      return _hasStep;
    }

    /// @brief Whether the type has a minimum or maximum.
    KRYS_NODISCARD bool HasRangeLimitations() const noexcept
    {
      return _hasRangeLimitations;
    }

    /// @brief The range's maximum and minimum.
    KRYS_NODISCARD Decimal Maximum() const noexcept
    {
      return _maximum;
    }

    KRYS_NODISCARD Decimal Minimum() const noexcept
    {
      return _minimum;
    }

    /// @brief The largest value in the range that's on a step, or NaN if there's none.
    KRYS_NODISCARD Decimal StepSnappedMaximum() const noexcept;

    /// @brief The allowed value step from the step attribute, scaled to the type's units; NaN for `any`
    /// (unless @p anyStepHandling defaults it) and the default for an invalid one.
    /// @see https://html.spec.whatwg.org/#concept-input-step
    KRYS_NODISCARD static Decimal ParseStep(AnyStepHandling anyStepHandling,
                                            const StepDescription &stepDescription,
                                            Maybe<DOMStringView> stepString) noexcept;

    /// @brief The step, its base, and the type's units per step value.
    KRYS_NODISCARD Decimal Step() const noexcept
    {
      return _step;
    }

    KRYS_NODISCARD Decimal StepBase() const noexcept
    {
      return _stepBase;
    }

    KRYS_NODISCARD int StepScaleFactor() const noexcept
    {
      return _stepDescription.StepScaleFactor;
    }

    /// @brief Whether @p valueForCheck isn't on a step.
    KRYS_NODISCARD bool StepMismatch(const Decimal &valueForCheck) const noexcept;

    /// @brief Whether the maximum may be below the minimum (a time range across midnight).
    KRYS_NODISCARD bool IsReversibleRange() const noexcept
    {
      return _isReversible;
    }

    /// @brief The middle value, clamped by the step.
    KRYS_NODISCARD Decimal DefaultValue() const noexcept
    {
      return ClampValue((_minimum + _maximum) / Decimal(2));
    }

    /// @brief Maps @p value into the 0-1 range.
    KRYS_NODISCARD Decimal ProportionFromValue(const Decimal &value) const noexcept
    {
      if (_minimum == _maximum)
      {
        return Decimal(0);
      }
      return (value - _minimum) / (_maximum - _minimum);
    }

    /// @brief Maps @p proportion from the 0-1 range to a value.
    KRYS_NODISCARD Decimal ValueFromProportion(const Decimal &proportion) const noexcept
    {
      return _minimum + proportion * (_maximum - _minimum);
    }

  private:
    /// @brief @p value rounded to the nearest step from @p base.
    KRYS_NODISCARD Decimal RoundByStep(const Decimal &value, const Decimal &base) const noexcept;
  };
}
