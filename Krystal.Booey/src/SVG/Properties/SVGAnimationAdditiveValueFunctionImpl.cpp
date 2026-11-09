/*
 * Copyright (C) 2019-2021 Apple Inc. All rights reserved.
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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/SVG/Properties/SVGAnimationAdditiveValueFunctionImpl.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Core/Text/StringToIntegerConversion.hpp"

namespace krys::boo::svg
{
  Maybe<float> SVGAnimationColorFunction::CalculateDistance(SVGElement &targetElement,
                                                            dom::DOMStringView from,
                                                            dom::DOMStringView to) const noexcept
  {
    auto simpleFrom =
      SVGPropertyTraits<Color>::FromString(targetElement, from).ToColorTypeLossy<SRGBA<uint8>>().Resolved();
    auto simpleTo =
      SVGPropertyTraits<Color>::FromString(targetElement, to).ToColorTypeLossy<SRGBA<uint8>>().Resolved();

    float red = static_cast<float>(simpleFrom.Red - simpleTo.Red);
    float green = static_cast<float>(simpleFrom.Green - simpleTo.Green);
    float blue = static_cast<float>(simpleFrom.Blue - simpleTo.Blue);

    return std::hypot(red, green, blue);
  }

  Maybe<float> SVGAnimationIntegerFunction::CalculateDistance(SVGElement &, dom::DOMStringView from,
                                                              dom::DOMStringView to) const noexcept
  {
    return static_cast<float>(
      std::abs(text::ParseInteger<int32>(to).value_or(0) - text::ParseInteger<int32>(from).value_or(0)));
  }

  Maybe<float> SVGAnimationNumberFunction::CalculateDistance(SVGElement &, dom::DOMStringView from,
                                                             dom::DOMStringView to) const noexcept
  {
    return std::abs(ParseNumberFromString(to).value_or(0) - ParseNumberFromString(from).value_or(0));
  }
}
