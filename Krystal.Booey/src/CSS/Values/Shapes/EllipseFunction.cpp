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

#include "Krystal.Booey/CSS/Values/Shapes/EllipseFunction.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD bool HasDefaultValueForEllipseRadius(const Ellipse::RadialSize &radius) noexcept
    {
      return krys::SwitchOn(
        radius,
        [](const Ellipse::Extent &extent)
        {
          // FIXME: The spec says that `farthest-corner` should be the default, but this does not
          // match the tests.
          return std::holds_alternative<keywords::ClosestSide>(extent);
        },
        [](const auto &) { return false; });
    }
  }

  void Serialiser<Ellipse>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                       const Ellipse &value) noexcept
  {
    // <ellipse()> = ellipse( <radial-size>? [ at <position> ]? )

    auto lengthBefore = builder.Length();

    if (!HasDefaultValueForEllipseRadius(get<0>(value.Radii))
        || !HasDefaultValueForEllipseRadius(get<1>(value.Radii)))
    {
      Serialise(builder, context, value.Radii);
    }

    if (value.Position)
    {
      // FIXME: To match other serialization of Percentage, this should not serialize if equal to the default
      // value of 50% 50%, but this does not match the tests.
      bool wroteSomething = builder.Length() != lengthBefore;
      builder.Append(wroteSomething ? u8" at " : u8"at ");
      Serialise(builder, context, *value.Position);
    }
  }
}
