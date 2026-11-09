/*
 * Copyright (C) 2006, 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Platform/Decimal.hpp"

namespace krys::boo::html
{
  enum class AnyStepHandling : bool
  {
    Reject,
    Default
  };

  enum class RangeLimitations : bool
  {
    Valid,
    Invalid
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

      KRYS_NODISCARD Decimal DefaultValue() const noexcept
      {
        return Decimal(DefaultStep * StepScaleFactor);
      }
    };

    enum class IsReversible : bool
    {
      No,
      Yes
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

    KRYS_NODISCARD Decimal AcceptableError() const noexcept;
    KRYS_NODISCARD Decimal AlignValueForStep(const Decimal &currentValue,
                                             const Decimal &newValue) const noexcept;
    KRYS_NODISCARD Decimal ClampValue(const Decimal &value) const noexcept;

    KRYS_NODISCARD bool HasStep() const noexcept
    {
      return _hasStep;
    }

    KRYS_NODISCARD bool HasRangeLimitations() const noexcept
    {
      return _hasRangeLimitations;
    }

    KRYS_NODISCARD Decimal Maximum() const noexcept
    {
      return _maximum;
    }

    KRYS_NODISCARD Decimal Minimum() const noexcept
    {
      return _minimum;
    }

    KRYS_NODISCARD Decimal StepSnappedMaximum() const noexcept;

    KRYS_NODISCARD static Decimal ParseStep(AnyStepHandling anyStepHandling,
                                            const StepDescription &stepDescription,
                                            Maybe<dom::DOMStringView> stepString) noexcept;

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

    KRYS_NODISCARD bool StepMismatch(const Decimal &valueForCheck) const noexcept;

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
    KRYS_NODISCARD Decimal RoundByStep(const Decimal &value, const Decimal &base) const noexcept;
  };
}
