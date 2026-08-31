#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  using DeprecatedGradientPosition = SpaceSeparatedArray<NumberOrPercentage<>, 2>;

  using Horizontal = Variant<Keywords::Left, Keywords::Right>;
  using Vertical = Variant<Keywords::Top, Keywords::Bottom>;

  using RadialGradientExtent =
    Variant<Keywords::ClosestCorner, Keywords::ClosestSide, Keywords::FarthestCorner, Keywords::FarthestSide>;
  using PrefixedRadialGradientExtent =
    Variant<Keywords::ClosestCorner, Keywords::ClosestSide, Keywords::FarthestCorner, Keywords::FarthestSide,
            Keywords::Contain, Keywords::Cover>;

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