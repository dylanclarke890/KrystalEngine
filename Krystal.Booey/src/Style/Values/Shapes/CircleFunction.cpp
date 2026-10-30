#include "Krystal.Booey/Style/Values/Shapes/CircleFunction.hpp"

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Core/Types/TinyLRUCache.hpp"
#include <numbers>

namespace krys::boo::style
{
  // MARK: - Path Caching

  struct CirclePathPolicy final : public TinyLRUCachePolicy<FloatRect, boo::Path>
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

  static const boo::Path &CachedCirclePath(const FloatRect &rect)
  {
    static NeverDestroyed<TinyLRUCache<FloatRect, boo::Path, 4, CirclePathPolicy>> cache;
    return cache.get().Get(rect);
  }

  // MARK: - Path Generation

  KRYS_NODISCARD FloatPoint ResolvePosition(const Circle &value, FloatSize boundingBox) noexcept
  {
    return value.Position ? Evaluate<FloatPoint>(*value.Position, boundingBox, style::ZoomNeeded {})
                          : FloatPoint {boundingBox.Width() / 2, boundingBox.Height() / 2};
  }

  KRYS_NODISCARD float ResolveRadius(const Circle &value, FloatSize boxSize, FloatPoint center) noexcept
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

  KRYS_NODISCARD boo::Path PathForCenterCoordinate(const Circle &value, const FloatRect &boundingBox,
                                                   FloatPoint center) noexcept
  {
    auto radius = ResolveRadius(value, boundingBox.Size(), center);
    auto bounding = FloatRect {center.X() - radius + boundingBox.X(), center.Y() - radius + boundingBox.Y(),
                               radius * 2, radius * 2};
    return CachedCirclePath(bounding);
  }

  boo::Path PathComputation<Circle>::operator()(const Circle &value, const FloatRect &boundingBox) noexcept
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
                                            { return krys::boo::style::Blend(lengthA, lengthB, context); },
                                            [&](const auto &a, const auto &) -> Circle::RadialSize
                                            { return a; }),
                        radiusA, radiusB);
    };

    return {
      .Radius = blendRadius(a.Radius, b.Radius, context),
      .Position = krys::boo::style::Blend(a.Position, b.Position, context),
    };
  }
}
