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

#include "Krystal.Booey/Style/Values/Shapes/PolygonFunction.hpp"

#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Core/Types/TinyLRUCache.hpp"

namespace krys::boo::style
{
  // MARK: - Path Caching

  struct PolygonPathPolicy : TinyLRUCachePolicy<List<FloatPoint>, boo::Path>
  {
  public:
    KRYS_NODISCARD static bool IsKeyNull(const List<FloatPoint> &points) noexcept
    {
      return !points.size();
    }

    KRYS_NODISCARD static boo::Path CreateValueForKey(const List<FloatPoint> &points) noexcept
    {
      return boo::Path(points);
    }
  };

  static const boo::Path &CachedPolygonPath(const List<FloatPoint> &points)
  {
    static NeverDestroyed<TinyLRUCache<List<FloatPoint>, boo::Path, 4, PolygonPathPolicy>> cache;
    return cache.get().Get(points);
  }

  // MARK: - Path

  boo::Path PathComputation<Polygon>::operator()(const Polygon &value, const FloatRect &boundingBox) noexcept
  {
    auto boundingLocation = boundingBox.Location();
    auto boundingSize = boundingBox.Size();
    List<FloatPoint> points;
    points.reserve(value.Vertices.size());
    for (const auto &vertex : value.Vertices)
    {
      points.push_back(Evaluate<FloatPoint>(vertex, boundingSize, style::ZoomNeeded {}) + boundingLocation);
    }
    return CachedPolygonPath(points);
  }

  // MARK: - Wind Rule

  boo::WindRule WindRuleComputation<Polygon>::operator()(const Polygon &value) noexcept
  {
    return (!value.FillRule || std::holds_alternative<css::keywords::Nonzero>(*value.FillRule))
             ? WindRule::NonZero
             : WindRule::EvenOdd;
  }

  // MARK: - Blending

  bool Blending<Polygon>::CanBlend(const Polygon &a, const Polygon &b) noexcept
  {
    return ComputeWindRule(a) == ComputeWindRule(b) && a.Vertices.size() == b.Vertices.size();
  }

  Polygon Blending<Polygon>::Blend(const Polygon &a, const Polygon &b,
                                   const BlendingContext &context) noexcept
  {
    return {
      .FillRule = a.FillRule,
      .Vertices = krys::boo::style::Blend(a.Vertices, b.Vertices, context),
    };
  }
}
