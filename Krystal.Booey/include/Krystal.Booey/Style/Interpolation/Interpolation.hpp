/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
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
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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

#pragma once

#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/Graphics/Animation/CompositeOperation.hpp"
#include "Krystal.Booey/Graphics/Animation/IterationCompositeOperation.hpp"

namespace krys::boo
{
  class RenderStyle;
  struct Settings;

  namespace dom
  {
    class Document;
  }
}

namespace krys::boo::style::interpolation
{
  class Client;

  // MARK: - RenderStyle independent

  /// @brief Whether the property can ever be interpolated.
  /// @note For properties other than custom properties, the "animation-type" key in the properties' JSON
  /// decides.
  KRYS_NODISCARD bool CanInterpolate(const AnimatableCSSProperty &property) noexcept;

  /// @brief Whether the property supports additive or cumulative interpolation.
  /// @note For properties other than custom properties, the
  /// "animation-wrapper-requires-non-additive-or-cumulative-interpolation" key decides.
  KRYS_NODISCARD bool IsAdditiveOrCumulative(const AnimatableCSSProperty &property) noexcept;

  /// @brief Whether the property supports accelerated animation.
  /// @note For properties other than custom properties, the "animation-wrapper-acceleration" key decides.
  KRYS_NODISCARD bool IsAccelerated(const AnimatableCSSProperty &property, const Settings &settings) noexcept;

  // MARK: - RenderStyle dependent

  /// @brief Whether the property's values in @p a and @p b are equal.
  KRYS_NODISCARD bool Equals(const AnimatableCSSProperty &property, const RenderStyle &a,
                             const RenderStyle &b, const dom::Document &document) noexcept;

  /// @brief Whether the property can be interpolated between its values in @p a and @p b.
  KRYS_NODISCARD bool CanInterpolate(const AnimatableCSSProperty &property, const RenderStyle &a,
                                     const RenderStyle &b, const dom::Document &document) noexcept;

  /// @brief Interpolates the property from its value in @p from to its value in @p to into @p destination.
  void Interpolate(const AnimatableCSSProperty &property, RenderStyle &destination, const RenderStyle &from,
                   const RenderStyle &to, double progress, CompositeOperation compositeOperation,
                   IterationCompositeOperation iterationCompositeOperation, double currentIteration,
                   const Client &client) noexcept;
  void Interpolate(const AnimatableCSSProperty &property, RenderStyle &destination, const RenderStyle &from,
                   const RenderStyle &to, double progress, CompositeOperation compositeOperation,
                   const Client &client) noexcept;

  /// @brief Whether the property needs interpolating for accumulative iteration between its values in @p a
  /// and @p b.
  KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const AnimatableCSSProperty &property,
                                                                    const RenderStyle &a,
                                                                    const RenderStyle &b,
                                                                    const Client &client) noexcept;
}
