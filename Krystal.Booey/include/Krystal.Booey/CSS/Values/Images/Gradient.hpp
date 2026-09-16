#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Core/Color/AlphaPremultiplication.hpp"
#include "Krystal.Core/Color/ColorInterpolationMethod.hpp"

namespace krys::boo::css
{
  using DeprecatedGradientPosition = SpaceSeparatedArray<NumberOrPercentage<>, 2>;

  using Horizontal = Variant<keywords::Left, keywords::Right>;
  using Vertical = Variant<keywords::Top, keywords::Bottom>;

  using RadialGradientExtent =
    Variant<keywords::ClosestCorner, keywords::ClosestSide, keywords::FarthestCorner, keywords::FarthestSide>;
  using PrefixedRadialGradientExtent =
    Variant<keywords::ClosestCorner, keywords::ClosestSide, keywords::FarthestCorner, keywords::FarthestSide,
            keywords::Contain, keywords::Cover>;

  // MARK: - Gradient Color Interpolation Definitions.

  struct GradientColorInterpolationMethod
  {
    enum class Default : bool
    {
      SRGB,
      OKLab
    };

    ColorInterpolationMethod method;
    Default defaultMethod;

    static GradientColorInterpolationMethod legacyMethod(AlphaPremultiplication alphaPremultiplication)
    {
      return {{ColorInterpolationMethod::SRGB {}, alphaPremultiplication}, Default::SRGB};
    }

    bool operator==(const GradientColorInterpolationMethod &) const = default;
  };

}