#include "Krystal.Booey/Rendering/HitTestingTransformState.hpp"

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
