/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Paths/PathSegment.hpp"

#include "Krystal.Booey/Graphics/Paths/PathElement.hpp"
#include "Krystal.Booey/Graphics/Paths/PathImpl.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  FloatPoint PathSegment::CalculateEndPoint(const FloatPoint &currentPoint,
                                            FloatPoint &lastMoveToPoint) const noexcept
  {
    return krys::SwitchOn(_data,
                          [&](auto &data) { return data.CalculateEndPoint(currentPoint, lastMoveToPoint); });
  }

  Maybe<FloatPoint> PathSegment::TryGetEndPointWithoutContext() const noexcept
  {
    return krys::SwitchOn(_data, [&](auto &data) { return data.TryGetEndPointWithoutContext(); });
  }

  FloatRect PathSegment::FastBoundingRect() const noexcept
  {
    FloatPoint currentPoint;
    FloatPoint lastMoveToPoint;

    auto boundingRect = FloatRect::SmallestRect();
    ExtendFastBoundingRect(currentPoint, lastMoveToPoint, boundingRect);

    if (boundingRect.IsSmallest())
    {
      currentPoint = CalculateEndPoint(currentPoint, lastMoveToPoint);
      boundingRect.Extend(currentPoint);
    }

    return boundingRect;
  }

  void PathSegment::ExtendFastBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                           FloatRect &boundingRect) const noexcept
  {
    krys::SwitchOn(_data, [&](auto &data)
                   { data.ExtendFastBoundingRect(currentPoint, lastMoveToPoint, boundingRect); });
  }

  void PathSegment::ExtendBoundingRect(const FloatPoint &currentPoint, const FloatPoint &lastMoveToPoint,
                                       FloatRect &boundingRect) const noexcept
  {
    krys::SwitchOn(_data,
                   [&](auto &data) { data.ExtendBoundingRect(currentPoint, lastMoveToPoint, boundingRect); });
  }

  bool PathSegment::CanApplyElements() const noexcept
  {
    return krys::SwitchOn(_data, [&](auto &data) { return data.CanApplyElements; });
  }

  bool PathSegment::ApplyElements(const PathElementApplier &applier) const noexcept
  {
    return krys::SwitchOn(_data,
                          [&]<typename DataType>(DataType &data) -> bool
                          {
                            if constexpr (DataType::CanApplyElements)
                            {
                              data.ApplyElements(applier);
                              return true;
                            }
                            else
                            {
                              return false;
                            }
                          });
  }

  bool PathSegment::CanTransform() const noexcept
  {
    return krys::SwitchOn(_data, [&](auto &data) { return data.CanTransform; });
  }

  bool PathSegment::Transform(const AffineTransform &transform) noexcept
  {
    return krys::SwitchOn(_data,
                          [&]<typename DataType>(DataType &data)
                          {
                            if constexpr (DataType::CanTransform)
                            {
                              data.Transform(transform);
                              return true;
                            }
                            else
                            {
                              return false;
                            }
                          });
  }

  TextStream &operator<<(TextStream &ts, const PathSegment &segment) noexcept
  {
    return std::visit([&](const auto &data) -> TextStream & { return ts << data; }, segment.GetData());
  }
}
