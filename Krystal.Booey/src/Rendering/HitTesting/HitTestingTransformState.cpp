/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/HitTesting/HitTestingTransformState.hpp"

namespace krys::boo
{
  void HitTestingTransformState::Translate(int32 x, int32 y) noexcept
  {
    AccumulatedTransform.Translate(x, y);
  }

  void HitTestingTransformState::ApplyTransform(const TransformationMatrix &transformFromContainer) noexcept
  {
    AccumulatedTransform.Multiply(transformFromContainer);
  }

  void HitTestingTransformState::Flatten() noexcept
  {
    AccumulatedTransform.Flatten();
  }

  FloatPoint HitTestingTransformState::MappedPoint() const noexcept
  {
    if (auto inverse = AccumulatedTransform.Inverse())
    {
      return inverse.value().ProjectPoint(LastPlanarPoint);
    }
    return LastPlanarPoint;
  }

  FloatQuad HitTestingTransformState::MappedQuad() const noexcept
  {
    if (auto inverse = AccumulatedTransform.Inverse())
    {
      return inverse.value().ProjectQuad(LastPlanarQuad);
    }
    return LastPlanarQuad;
  }

  FloatQuad HitTestingTransformState::MappedArea() const noexcept
  {
    if (auto inverse = AccumulatedTransform.Inverse())
    {
      return inverse.value().ProjectQuad(LastPlanarArea);
    }
    return LastPlanarArea;
  }

  LayoutRect HitTestingTransformState::BoundsOfMappedArea() const noexcept
  {
    if (auto inverse = AccumulatedTransform.Inverse())
    {
      return inverse.value().ClampedBoundsOfProjectedQuad(LastPlanarArea);
    }
    TransformationMatrix identity;
    return identity.ClampedBoundsOfProjectedQuad(LastPlanarArea);
  }

  LayoutRect HitTestingTransformState::BoundsOfMappedQuad() const noexcept
  {
    if (auto inverse = AccumulatedTransform.Inverse())
    {
      return inverse.value().ClampedBoundsOfProjectedQuad(LastPlanarQuad);
    }
    TransformationMatrix identity;
    return identity.ClampedBoundsOfProjectedQuad(LastPlanarQuad);
  }
}
