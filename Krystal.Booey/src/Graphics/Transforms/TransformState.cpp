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

#include "Krystal.Booey/Graphics/Transforms/TransformState.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo
{
  TransformState &TransformState::operator=(const TransformState &other) noexcept
  {
    _accumulatedOffset = other._accumulatedOffset;
    _tracking = other._tracking;
    _inputPoint = other._inputPoint;
    _inputQuad = other._inputQuad;
    _inputSecondaryQuad = other._inputSecondaryQuad;
    _direction = other._direction;

    _accumulatedTransform = nullptr;

    if (other._accumulatedTransform)
    {
      _accumulatedTransform = CreateUnique<TransformationMatrix>(*other._accumulatedTransform);
    }

    return *this;
  }

  void TransformState::TranslateTransform(const LayoutSize &offset, TransformDirection direction) noexcept
  {
    if (direction == TransformDirection::ApplyTransform)
    {
      _accumulatedTransform->TranslateRight(offset.Width().ToDouble(), offset.Height().ToDouble());
    }
    else
    {
      _accumulatedTransform->Translate(offset.Width().ToDouble(), offset.Height().ToDouble());
    }
  }

  void TransformState::Move(const LayoutSize &offset, TransformAccumulation accumulate) noexcept
  {
    if (ShouldFlattenBefore(accumulate))
    {
      Flatten();
    }

    if (_accumulatedTransform)
    {
      // If we're accumulating into an existing transform, apply the translation.
      TranslateTransform(offset, _direction);
    }
    else
    {
      _accumulatedOffset += offset;
    }

    if (ShouldFlattenAfter(accumulate))
    {
      Flatten();
    }
  }

  // FIXME: We transform AffineTransform to TransformationMatrix. This is rather inefficient.
  void TransformState::ApplyTransform(const AffineTransform &transformFromContainer,
                                      TransformAccumulation accumulate, bool *wasClamped) noexcept
  {
    ApplyTransform(transformFromContainer.ToTransformationMatrix(), accumulate, wasClamped);
  }

  void TransformState::ApplyTransform(const TransformationMatrix &transformFromContainer,
                                      TransformAccumulation accumulate, bool *wasClamped) noexcept
  {
    if (wasClamped)
    {
      *wasClamped = false;
    }

    if (transformFromContainer.IsIntegerTranslation())
    {
      Move(LayoutSize(LayoutUnit(transformFromContainer.E()), LayoutUnit(transformFromContainer.F())),
           accumulate);
      return;
    }

    if (ShouldFlattenBefore(accumulate))
    {
      Flatten();
    }

    if (!_accumulatedTransform)
    {
      _accumulatedTransform = CreateUnique<TransformationMatrix>(_accumulatedOffset.Width().ToDouble(),
                                                                 _accumulatedOffset.Height().ToDouble());
      _accumulatedOffset = {};
    }
    krys_debug_assert(_accumulatedOffset.IsZero());

    if (_direction == TransformDirection::ApplyTransform)
    {
      *_accumulatedTransform = transformFromContainer * *_accumulatedTransform;
    }
    else
    {
      *_accumulatedTransform *= transformFromContainer;
    }

    if (ShouldFlattenAfter(accumulate))
    {
      Flatten();
    }
  }

  void TransformState::Flatten() noexcept
  {
    if (_accumulatedTransform)
    {
      _accumulatedTransform->Flatten();
    }
  }

  FloatPoint TransformState::MappedPoint() const noexcept
  {
    auto point = _inputPoint;
    auto offset =
      (_direction == TransformDirection::ApplyTransform) ? _accumulatedOffset : -_accumulatedOffset;
    point.Move(offset.Width().ToFloat(), offset.Height().ToFloat());
    if (!_accumulatedTransform)
    {
      return point;
    }

    if (_direction == TransformDirection::ApplyTransform)
    {
      return _accumulatedTransform->MapPoint(point);
    }

    return _accumulatedTransform->Inverse().value_or(TransformationMatrix()).ProjectPoint(point);
  }

  FloatQuad TransformState::MappedQuad(bool *wasClamped) const noexcept
  {
    if (wasClamped)
    {
      *wasClamped = false;
    }

    auto quad = _inputQuad;
    MapQuad(quad, _direction, wasClamped);
    return quad;
  }

  Maybe<FloatQuad> TransformState::MappedSecondaryQuad(bool *wasClamped) const noexcept
  {
    if (wasClamped)
    {
      *wasClamped = false;
    }

    if (!_inputSecondaryQuad)
    {
      return null;
    }

    auto quad = *_inputSecondaryQuad;
    MapQuad(quad, _direction, wasClamped);
    return quad;
  }

  void TransformState::Reset(const FloatQuad &quad, const Maybe<FloatQuad> &secondaryQuad) noexcept
  {
    _inputQuad = quad;
    _inputSecondaryQuad = secondaryQuad;
    _accumulatedTransform = nullptr;
    _accumulatedOffset = {};
  }

  void TransformState::SetSecondaryQuadInMappedSpace(const Maybe<FloatQuad> &quad) noexcept
  {
    if (!quad)
    {
      _inputSecondaryQuad = null;
      return;
    }

    // Map the quad back through any transform or offset back into the last flattening coordinate space.
    auto backMappedQuad = *quad;
    MapQuad(backMappedQuad, InverseDirection());
    _inputSecondaryQuad = backMappedQuad;
  }

  void TransformState::MapQuad(FloatQuad &quad, TransformDirection direction, bool *wasClamped) const noexcept
  {
    auto offset =
      (direction == TransformDirection::ApplyTransform) ? _accumulatedOffset : -_accumulatedOffset;
    quad.Move(offset.Width().ToFloat(), offset.Height().ToFloat());
    if (!_accumulatedTransform)
    {
      return;
    }

    if (direction == TransformDirection::ApplyTransform)
    {
      quad = _accumulatedTransform->MapQuad(quad);
      return;
    }

    quad = _accumulatedTransform->Inverse().value_or(TransformationMatrix()).ProjectQuad(quad, wasClamped);
  }

  UniquePtr<TransformationMatrix> TransformState::ReleaseTrackedTransform() const noexcept
  {
    if (_accumulatedTransform)
    {
      return CreateUnique<TransformationMatrix>(*_accumulatedTransform);
    }
    return CreateUnique<TransformationMatrix>(_accumulatedOffset.Width().ToDouble(),
                                              _accumulatedOffset.Height().ToDouble());
  }
}
