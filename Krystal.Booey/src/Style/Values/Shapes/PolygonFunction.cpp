#include "Krystal.Booey/Style/Values/Shapes/PolygonFunction.hpp"

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
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
