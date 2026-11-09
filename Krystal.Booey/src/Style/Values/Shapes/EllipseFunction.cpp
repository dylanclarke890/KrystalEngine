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

#include "Krystal.Booey/Style/Values/Shapes/EllipseFunction.hpp"

#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"
#include "Krystal.Booey/Style/Values/Images/Gradient.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Position.hpp"
#include "Krystal.Core/Types/TinyLRUCache.hpp"

namespace krys::boo::style
{
  // MARK: - Path Caching

  struct EllipsePathPolicy final : public TinyLRUCachePolicy<FloatRect, boo::Path>
  {
  public:
    KRYS_NODISCARD static bool IsKeyNull(const FloatRect &rect) noexcept
    {
      return rect.IsEmpty();
    }

    KRYS_NODISCARD static boo::Path CreateValueForKey(const FloatRect &rect) noexcept
    {
      boo::Path path;
      path.AddEllipseInRect(rect);
      return path;
    }
  };

  static const boo::Path &CachedEllipsePath(const FloatRect &rect)
  {
    static NeverDestroyed<TinyLRUCache<FloatRect, boo::Path, 4, EllipsePathPolicy>> cache;
    return cache.get().Get(rect);
  }

  // MARK: - Path Generation

  KRYS_NODISCARD FloatPoint ResolvePosition(const Ellipse &value, FloatSize boundingBox) noexcept
  {
    return value.Position ? Evaluate<FloatPoint>(*value.Position, boundingBox, style::ZoomNeeded {})
                          : FloatPoint {boundingBox.Width() / 2, boundingBox.Height() / 2};
  }

  KRYS_NODISCARD FloatSize ResolveRadii(const Ellipse &value, FloatSize boxSize, FloatPoint center) noexcept
  {
    auto sizeForAxis = [&](const Ellipse::RadialSize &radius, float centerValue, float dimensionSize)
    {
      return krys::SwitchOn(
        radius, [&](const Ellipse::Length &length) -> float
        { return Evaluate<float>(length, std::abs(dimensionSize), style::ZoomNeeded {}); },
        [&](const Ellipse::Extent &extent) -> float
        {
          return krys::SwitchOn(
            extent, [&](css::keywords::ClosestSide) -> float
            { return std::min(std::abs(centerValue), std::abs(dimensionSize - centerValue)); },
            [&](css::keywords::FarthestSide) -> float
            { return std::max(std::abs(centerValue), std::abs(dimensionSize - centerValue)); },
            [&](css::keywords::ClosestCorner) -> float { return DistanceToClosestCorner(center, boxSize); },
            [&](css::keywords::FarthestCorner) -> float
            { return DistanceToFarthestCorner(center, boxSize); });
        });
    };

    return {sizeForAxis(get<0>(value.Radii), center.X(), boxSize.Width()),
            sizeForAxis(get<1>(value.Radii), center.Y(), boxSize.Height())};
  }

  KRYS_NODISCARD boo::Path PathForCenterCoordinate(const Ellipse &value, const FloatRect &boundingBox,
                                                   FloatPoint center) noexcept
  {
    auto radii = ResolveRadii(value, boundingBox.Size(), center);
    auto bounding =
      FloatRect {center.X() - radii.Width() + boundingBox.X(), center.Y() - radii.Height() + boundingBox.Y(),
                 radii.Width() * 2, radii.Height() * 2};
    return CachedEllipsePath(bounding);
  }

  boo::Path PathComputation<Ellipse>::operator()(const Ellipse &value, const FloatRect &boundingBox) noexcept
  {
    return PathForCenterCoordinate(value, boundingBox, ResolvePosition(value, boundingBox.Size()));
  }

  // MARK: - Blending

  bool Blending<Ellipse>::CanBlend(const Ellipse &a, const Ellipse &b) noexcept
  {
    auto canBlendRadius = [](const auto &radiusA, const auto &radiusB)
    {
      return std::visit(krys::CreateVisitor([](const Ellipse::Length &, const Ellipse::Length &)
                                            { return true; },
                                            [](const auto &, const auto &)
                                            {
                                              // FIXME: Determine how to interpolate between keywords. See
                                              // bug 125108.
                                              return false;
                                            }),
                        radiusA, radiusB);
    };

    return canBlendRadius(get<0>(a.Radii), get<0>(b.Radii))
           && canBlendRadius(get<1>(a.Radii), get<1>(b.Radii)) && style::CanBlend(a.Position, b.Position);
  }

  Ellipse Blending<Ellipse>::Blend(const Ellipse &a, const Ellipse &b,
                                   const BlendingContext &context) noexcept
  {
    auto blendRadius = [](const auto &radiusA, const auto &radiusB,
                          const BlendingContext &context) -> Ellipse::RadialSize
    {
      return std::visit(krys::CreateVisitor([&](const Ellipse::Length &lengthA,
                                                const Ellipse::Length &lengthB) -> Ellipse::RadialSize
                                            { return krys::boo::style::Blend(lengthA, lengthB, context); },
                                            [&](const auto &a, const auto &) -> Ellipse::RadialSize
                                            { return a; }),
                        radiusA, radiusB);
    };

    return {
      .Radii =
        css::SpaceSeparatedPair<Ellipse::RadialSize> {blendRadius(get<0>(a.Radii), get<0>(b.Radii), context),
                                                      blendRadius(get<1>(a.Radii), get<1>(b.Radii), context)},
      .Position = krys::boo::style::Blend(a.Position, b.Position, context),
    };
  }
}
