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

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/NumberFormatting.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <cmath>
#include <limits>

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD CSSOMStringView FormatNonfiniteCSSNumberValuePrefix(double number) noexcept
    {
      if (number == std::numeric_limits<double>::infinity())
      {
        return u8"infinity";
      }
      if (number == -std::numeric_limits<double>::infinity())
      {
        return u8"-infinity";
      }
      krys_debug_assert(std::isnan(number));
      return u8"NaN";
    }
  }

  void FormatNonfiniteCSSNumberValue(StringBuilder &builder, const SerialisableNumber &number) noexcept
  {
    builder.Append(FormatNonfiniteCSSNumberValuePrefix(number.Value));
    if (!number.Suffix.empty())
    {
      builder.Append(u8" * 1");
    }
    builder.Append(number.Suffix);
  }

  CSSOMString FormatNonfiniteCSSNumberValue(const SerialisableNumber &number) noexcept
  {
    StringBuilder builder;
    FormatNonfiniteCSSNumberValue(builder, number);
    return krys::move(builder).ToString();
  }

  void FormatCSSNumberValue(StringBuilder &builder, const SerialisableNumber &number) noexcept
  {
    if (!std::isfinite(number.Value))
    {
      FormatNonfiniteCSSNumberValue(builder, number);
      return;
    }

    FormatCSSNumber(builder, number.Value);
    builder.Append(number.Suffix);
  }

  CSSOMString FormatCSSNumberValue(const SerialisableNumber &number) noexcept
  {
    StringBuilder builder;
    FormatCSSNumberValue(builder, number);
    return krys::move(builder).ToString();
  }
}
