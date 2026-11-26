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

#include "Krystal.Web/Style/Values/Shapes/CircleFunction.hpp"

#include "Krystal.Core/Containers/TinyLRUCache.hpp"
#include "Krystal.Core/Geometry/GeometryUtilities.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Web/Graphics/Paths/Path.hpp"
#include "Krystal.Web/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Web/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Web/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include <numbers>

namespace krys::web::style
{
  // MARK: - Path Caching

  struct CirclePathPolicy final : public TinyLRUCachePolicy<RectF, web::Path>
  {
  public:
    KRYS_NODISCARD static bool IsKeyNull(const RectF &rect) noexcept
    {
      return rect.IsEmpty();
    }

    KRYS_NODISCARD static web::Path CreateValueForKey(const RectF &rect) noexcept
    {
      web::Path path;
      path.AddEllipseInRect(rect);
      return path;
    }
  };

  static const web::Path &CachedCirclePath(const RectF &rect)
  {
    static NeverDestroyed<TinyLRUCache<RectF, web::Path, 4, CirclePathPolicy>> cache;
    return cache->Get(rect);
  }

  // MARK: - Path Generation

  KRYS_NODISCARD PointF ResolvePosition(const Circle &value, SizeF boxSize) noexcept
  {
    return value.Position ? Evaluate<PointF>(*value.Position, boxSize, style::ZoomNeeded {})
                          : PointF {boxSize.Width() / 2, boxSize.Height() / 2};
  }

  KRYS_NODISCARD float ResolveRadius(const Circle &value, SizeF boxSize, PointF center) noexcept
  {
    return krys::SwitchOn(
      value.Radius,
      [&](const Circle::Length &length) -> float
      {
        return Evaluate<float>(length, boxSize.DiagonalLength() / std::numbers::sqrt2_v<float>,
                               style::ZoomNeeded {});
      },
      [&](const Circle::Extent &extent) -> float
      {
        return krys::SwitchOn(
          extent, [&](css::keywords::ClosestSide) -> float { return DistanceToClosestSide(center, boxSize); },
          [&](css::keywords::FarthestSide) -> float { return DistanceToFarthestSide(center, boxSize); },
          [&](css::keywords::ClosestCorner) -> float { return DistanceToClosestCorner(center, boxSize); },
          [&](css::keywords::FarthestCorner) -> float { return DistanceToFarthestCorner(center, boxSize); });
      });
  }

  KRYS_NODISCARD web::Path PathForCenterCoordinate(const Circle &value, const RectF &boundingBox,
                                                   PointF center) noexcept
  {
    auto radius = ResolveRadius(value, boundingBox.Size(), center);
    auto bounding = RectF {center.X() - radius + boundingBox.X(), center.Y() - radius + boundingBox.Y(),
                           radius * 2, radius * 2};
    return CachedCirclePath(bounding);
  }

  web::Path PathComputation<Circle>::operator()(const Circle &value, const RectF &boundingBox) noexcept
  {
    return PathForCenterCoordinate(value, boundingBox, ResolvePosition(value, boundingBox.Size()));
  }

  // MARK: - Blending

  bool Blending<Circle>::CanBlend(const Circle &a, const Circle &b) noexcept
  {
    auto canBlendRadius = [](const auto &radiusA, const auto &radiusB)
    {
      return std::visit(krys::CreateVisitor([](const Circle::Length &, const Circle::Length &)
                                            { return true; },
                                            [](const auto &, const auto &)
                                            {
                                              // FIXME: Determine how to interpolate between keywords. See
                                              // bug 125108.
                                              return false;
                                            }),
                        radiusA, radiusB);
    };

    return canBlendRadius(a.Radius, b.Radius) && style::CanBlend(a.Position, b.Position);
  }

  Circle Blending<Circle>::Blend(const Circle &a, const Circle &b, const BlendingContext &context) noexcept
  {
    auto blendRadius = [](const auto &radiusA, const auto &radiusB,
                          const BlendingContext &context) -> Circle::RadialSize
    {
      return std::visit(krys::CreateVisitor([&](const Circle::Length &lengthA,
                                                const Circle::Length &lengthB) -> Circle::RadialSize
                                            { return krys::web::style::Blend(lengthA, lengthB, context); },
                                            [&](const auto &a, const auto &) -> Circle::RadialSize
                                            { return a; }),
                        radiusA, radiusB);
    };

    return {
      .Radius = blendRadius(a.Radius, b.Radius, context),
      .Position = krys::web::style::Blend(a.Position, b.Position, context),
    };
  }
}
