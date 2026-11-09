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

#include "Krystal.Booey/CSS/Values/Borders/BorderRadius.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::css
{
  namespace
  {
    using Radii = SpaceSeparatedList<LengthPercentage<NonNegative>, 4>;

    KRYS_NODISCARD bool HasDefaultValueForAxis(const BorderRadius::Axis &values) noexcept
    {
      return values.Value[3] == values.Value[1] && values.Value[2] == values.Value[0]
             && values.Value[1] == values.Value[0] && values.Value[0] == 0_css_px;
    }

    KRYS_NODISCARD Pair<Radii, bool> GatherSerialisableRadiiForAxis(const BorderRadius::Axis &values) noexcept
    {
      bool isDefaultValue = false;

      Radii::Container result;
      result.push_back(values.Value[0]);

      if (values.Value[3] != values.Value[1])
      {
        result.push_back(values.Value[1]);
        result.push_back(values.Value[2]);
        result.push_back(values.Value[3]);
      }
      else if (values.Value[2] != values.Value[0])
      {
        result.push_back(values.Value[1]);
        result.push_back(values.Value[2]);
      }
      else if (values.Value[1] != values.Value[0])
      {
        result.push_back(values.Value[1]);
      }
      else
      {
        isDefaultValue = result[0] == 0_css_px;
      }

      return {Radii {krys::move(result)}, isDefaultValue};
    }
  }

  bool HasDefaultValue(const BorderRadius &borderRadius) noexcept
  {
    return HasDefaultValueForAxis(borderRadius.Horizontal) && HasDefaultValueForAxis(borderRadius.Vertical);
  }

  void Serialiser<BorderRadius>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                            const BorderRadius &value) noexcept
  {
    // <'border-radius'> = <length-percentage [0,∞]>{1,4} [ / <length-percentage [0,∞]>{1,4} ]?

    auto [horizontal, horizontalIsDefault] = GatherSerialisableRadiiForAxis(value.Horizontal);
    auto [vertical, verticalIsDefault] = GatherSerialisableRadiiForAxis(value.Vertical);

    if (!horizontalIsDefault || !verticalIsDefault)
    {
      Serialise(builder, context, horizontal);

      if (horizontal != vertical)
      {
        builder.Append(u8" / ");
        Serialise(builder, context, vertical);
      }
    }
  }
}
