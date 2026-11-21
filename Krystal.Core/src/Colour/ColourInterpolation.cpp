/*
 * Copyright (C) 2021-2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Core/Colour/ColourInterpolation.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys
{
  std::pair<float, float> FixupHueComponentsPriorToInterpolation(HueInterpolationMethod method,
                                                                 float component1, float component2) noexcept
  {
    auto NormalizeAnglesUsingShorterAlgorithm = [](auto theta1, auto theta2) -> std::pair<float, float>
    {
      // https://drafts.csswg.org/css-color-4/#hue-shorter
      auto difference = theta2 - theta1;
      if (difference > 180.0f)
      {
        return {theta1 + 360.0f, theta2};
      }

      if (difference < -180.0f)
      {
        return {theta1, theta2 + 360.0f};
      }

      return {theta1, theta2};
    };

    auto NormalizeAnglesUsingLongerAlgorithm = [](auto theta1, auto theta2) -> std::pair<float, float>
    {
      // https://drafts.csswg.org/css-color-4/#hue-longer
      auto difference = theta2 - theta1;
      if (difference > 0 && difference < 180.f)
      {
        return {theta1 + 360.f, theta2};
      }

      if (difference > -180.f && difference <= 0)
      {
        return {theta1, theta2 + 360.f};
      }

      return {theta1, theta2};
    };

    auto NormalizeAnglesUsingIncreasingAlgorithm = [](auto theta1, auto theta2) -> std::pair<float, float>
    {
      // https://drafts.csswg.org/css-color-4/#hue-increasing
      if (theta2 < theta1)
      {
        return {theta1, theta2 + 360.f};
      }

      return {theta1, theta2};
    };

    auto NormalizeAnglesUsingDecreasingAlgorithm = [](auto theta1, auto theta2) -> std::pair<float, float>
    {
      // https://drafts.csswg.org/css-color-4/#hue-decreasing
      if (theta1 < theta2)
      {
        return {theta1 + 360.f, theta2};
      }

      return {theta1, theta2};
    };

    // https://www.w3.org/TR/css-color-4/#hue-interpolation
    //    "Both angles need to be constrained to [0, 360] prior to interpolation.
    //     One way to do this is θ = ((θ % 360) + 360) % 360."

    switch (method)
    {
      case HueInterpolationMethod::Shorter:
      {
        return NormalizeAnglesUsingShorterAlgorithm(NormalizeHue(component1), NormalizeHue(component2));
      }
      case HueInterpolationMethod::Longer:
      {
        return NormalizeAnglesUsingLongerAlgorithm(NormalizeHue(component1), NormalizeHue(component2));
      }
      case HueInterpolationMethod::Increasing:
      {
        return NormalizeAnglesUsingIncreasingAlgorithm(NormalizeHue(component1), NormalizeHue(component2));
      }
      case HueInterpolationMethod::Decreasing:
      {
        return NormalizeAnglesUsingDecreasingAlgorithm(NormalizeHue(component1), NormalizeHue(component2));
      }
    }

    krys_unreachable();
  }
}
