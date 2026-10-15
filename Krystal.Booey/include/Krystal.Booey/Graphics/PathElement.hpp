#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo
{
  struct PathElement
  {
    enum class Type : uint8
    {
      MoveToPoint,
      AddLineToPoint,
      AddQuadCurveToPoint,
      AddCurveToPoint,
      CloseSubpath
    };
    Type ElementType;
    std::array<FloatPoint, 3> Points;
  };

  using PathElementApplier = Func<void(const PathElement &)>;
}
