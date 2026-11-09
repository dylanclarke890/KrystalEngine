/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  // MARK: - Serialisation

  struct SerialisableNumber
  {
    double Value;
    CSSOMStringView Suffix;
  };

  /// @brief Appends a non-finite number as `infinity`, `-infinity` or `NaN`, multiplied by 1 of the suffix's
  /// unit if there is one (e.g. `infinity * 1px`).
  void FormatNonfiniteCSSNumberValue(StringBuilder &builder, const SerialisableNumber &number) noexcept;
  KRYS_NODISCARD CSSOMString FormatNonfiniteCSSNumberValue(const SerialisableNumber &number) noexcept;

  /// @brief Appends a number rounded to at most 6 decimal places, without trailing zeros, followed by its
  /// suffix.
  void FormatCSSNumberValue(StringBuilder &builder, const SerialisableNumber &number) noexcept;
  KRYS_NODISCARD CSSOMString FormatCSSNumberValue(const SerialisableNumber &number) noexcept;

  template <>
  struct Serialiser<SerialisableNumber>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &,
                    const SerialisableNumber &number) noexcept
    {
      FormatCSSNumberValue(builder, number);
    }
  };

  template <NumericRaw RawType>
  struct Serialiser<RawType>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const RawType &value) noexcept
    {
      Serialise(builder, context, SerialisableNumber {value.Value, UnitString(value.Unit)});
    }
  };

  template <Range nR, Range pR, typename V>
  struct Serialiser<NumberOrPercentageResolvedToNumber<nR, pR, V>>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const NumberOrPercentageResolvedToNumber<nR, pR, V> &value) noexcept
    {
      value.SwitchOn([&](const typename NumberOrPercentageResolvedToNumber<nR, pR, V>::Number &number)
                     { Serialise(builder, context, number); },
                     [&](const typename NumberOrPercentageResolvedToNumber<nR, pR, V>::Percentage &percentage)
                     {
                       if (auto raw = percentage.GetRaw())
                       {
                         Serialise(builder, context, NumberRaw<nR, V> {raw->Value / 100.0});
                       }
                       else
                       {
                         Serialise(builder, context, percentage);
                       }
                     });
    }
  };
}
