#pragma once

#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/SVG/SVGTransformValue.hpp"

namespace krys::boo::svg
{
  /// @brief The difference between two transforms of the same type, which `<animateTransform>` scales by its
  /// progress and adds to its `from` transform.
  class SVGTransformDistance
  {
  private:
    SVGTransformValue::SVGTransformType _type {SVGTransformValue::SVG_TRANSFORM_UNKNOWN};
    float _angle {0};
    float _cx {0};
    float _cy {0};
    AffineTransform _transform; // for storing scale, translation or matrix transforms

  public:
    SVGTransformDistance() noexcept = default;
    SVGTransformDistance(const SVGTransformValue &fromTransform,
                         const SVGTransformValue &toTransform) noexcept;

    KRYS_NODISCARD SVGTransformDistance ScaledDistance(float scaleFactor) const noexcept;
    KRYS_NODISCARD SVGTransformValue AddToSVGTransform(const SVGTransformValue &) const noexcept;

    KRYS_NODISCARD static SVGTransformValue
      AddSVGTransforms(const SVGTransformValue &, const SVGTransformValue &, uint32 repeatCount = 1) noexcept;

    KRYS_NODISCARD float Distance() const noexcept;

  private:
    SVGTransformDistance(SVGTransformValue::SVGTransformType, float angle, float cx, float cy,
                         const AffineTransform &) noexcept;
  };
}
