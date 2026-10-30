#include "Krystal.Booey/Style/TransformResolver.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatPoint3D.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Rendering/MotionPath.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Rendering/TransformOperationData.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  TransformResolver::TransformResolver(TransformationMatrix &transform, const ComputedStyle &style) noexcept
      : _transform {transform}, _style {style}
  {
  }

  TransformResolver::TransformResolver(TransformationMatrix &transform, const RenderStyle &style) noexcept
      : TransformResolver {transform, style.ComputedStyle()}
  {
  }

  bool TransformResolver::AffectedByTransformOrigin(const ComputedStyle &style) noexcept
  {
    return style.GetRotate().AffectedByTransformOrigin() || style.GetScale().AffectedByTransformOrigin()
           || style.GetTransform().AffectedByTransformOrigin()
           || style.GetOffsetPath().AffectedByTransformOrigin();
  }

  bool TransformResolver::AffectedByTransformOrigin(const RenderStyle &style) noexcept
  {
    return AffectedByTransformOrigin(style.ComputedStyle());
  }

  bool TransformResolver::AffectedByTransformOrigin() const noexcept
  {
    return AffectedByTransformOrigin(_style);
  }

  FloatPoint3D TransformResolver::ComputeTransformOrigin(const ComputedStyle &style,
                                                         const FloatRect &boundingBox) noexcept
  {
    FloatPoint3D originTranslate;
    originTranslate.SetXY(
      boundingBox.Location()
      + Evaluate<FloatPoint>(style.GetTransformOrigin().XY(), boundingBox.Size(), ZoomNeeded {}));
    originTranslate.SetZ(style.GetTransformOriginZ().ResolveZoom(ZoomNeeded {}));
    return originTranslate;
  }

  FloatPoint3D TransformResolver::ComputeTransformOrigin(const RenderStyle &style,
                                                         const FloatRect &boundingBox) noexcept
  {
    return ComputeTransformOrigin(style.ComputedStyle(), boundingBox);
  }

  FloatPoint3D TransformResolver::ComputeTransformOrigin(const FloatRect &boundingBox) const noexcept
  {
    return ComputeTransformOrigin(_style, boundingBox);
  }

  FloatPoint TransformResolver::ComputePerspectiveOrigin(const ComputedStyle &style,
                                                         const FloatRect &boundingBox) noexcept
  {
    return boundingBox.Location()
           + Evaluate<FloatPoint>(style.GetPerspectiveOrigin(), boundingBox.Size(), ZoomNeeded {});
  }

  FloatPoint TransformResolver::ComputePerspectiveOrigin(const RenderStyle &style,
                                                         const FloatRect &boundingBox) noexcept
  {
    return ComputePerspectiveOrigin(style.ComputedStyle(), boundingBox);
  }

  FloatPoint TransformResolver::ComputePerspectiveOrigin(const FloatRect &boundingBox) const noexcept
  {
    return ComputePerspectiveOrigin(_style, boundingBox);
  }

  void TransformResolver::ApplyPerspective(const FloatPoint &originTranslate) noexcept
  {
    // https://www.w3.org/TR/css-transforms-2/#perspective
    // The perspective matrix is computed as follows:
    // 1. Start with the identity matrix.

    // 2. Translate by the computed X and Y values of perspective-origin
    _transform.Translate(originTranslate.X(), originTranslate.Y());

    // 3. Multiply by the matrix that would be obtained from the perspective() transform function, where the
    // length is provided by the value of the perspective property
    _transform.ApplyPerspective(_style.GetPerspective().UsedPerspective());

    // 4. Translate by the negated computed X and Y values of perspective-origin
    _transform.Translate(-originTranslate.X(), -originTranslate.Y());
  }

  void TransformResolver::ApplyTransformOrigin(const FloatPoint3D &originTranslate) noexcept
  {
    if (!originTranslate.IsZero())
    {
      _transform.Translate3d(originTranslate.X(), originTranslate.Y(), originTranslate.Z());
    }
  }

  void TransformResolver::UnapplyTransformOrigin(const FloatPoint3D &originTranslate) noexcept
  {
    if (!originTranslate.IsZero())
    {
      _transform.Translate3d(-originTranslate.X(), -originTranslate.Y(), -originTranslate.Z());
    }
  }

  void TransformResolver::ApplyCSSTransform(const TransformOperationData &transformData,
                                            EnumSet<Option> options) noexcept
  {
    // https://www.w3.org/TR/css-transforms-2/#ctm
    // The transformation matrix is computed from the transform, transform-origin, translate, rotate, scale,
    // and offset properties as follows:
    // 1. Start with the identity matrix.

    // 2. Translate by the computed X, Y, and Z values of transform-origin.
    // (implemented in ApplyTransformOrigin)
    auto &boundingBox = transformData.BoundingBox;

    // 3. Translate by the computed X, Y, and Z values of translate.
    if (options.Contains(Option::Translate))
    {
      _style.GetTranslate().Apply(_transform, boundingBox.Size());
    }

    // 4. Rotate by the computed <angle> about the specified axis of rotate.
    if (options.Contains(Option::Rotate))
    {
      _style.GetRotate().Apply(_transform, boundingBox.Size());
    }

    // 5. Scale by the computed X, Y, and Z values of scale.
    if (options.Contains(Option::Scale))
    {
      _style.GetScale().Apply(_transform, boundingBox.Size());
    }

    // 6. Translate and rotate by the transform specified by offset.
    if (options.Contains(Option::Offset))
    {
      ApplyMotionPathTransform(transformData);
    }

    // 7. Multiply by each of the transform functions in transform from left to right.
    _style.GetTransform().Apply(_transform, boundingBox.Size());

    // 8. Translate by the negated computed X, Y and Z values of transform-origin.
    // (implemented in UnapplyTransformOrigin)
  }

  void TransformResolver::ApplyTransform(const TransformOperationData &transformData,
                                         EnumSet<Option> options) noexcept
  {
    if (!options.Contains(Option::TransformOrigin) || !AffectedByTransformOrigin())
    {
      ApplyCSSTransform(transformData, options);
      return;
    }

    auto originTranslate = ComputeTransformOrigin(transformData.BoundingBox);
    ApplyTransformOrigin(originTranslate);
    ApplyCSSTransform(transformData, options);
    UnapplyTransformOrigin(originTranslate);
  }

  void TransformResolver::ApplyTransform(TransformationMatrix &transform, const ComputedStyle &style,
                                         const TransformOperationData &transformData,
                                         EnumSet<Option> options) noexcept
  {
    TransformResolver {transform, style}.ApplyTransform(transformData, options);
  }

  void TransformResolver::ApplyTransform(TransformationMatrix &transform, const RenderStyle &style,
                                         const TransformOperationData &transformData,
                                         EnumSet<Option> options) noexcept
  {
    ApplyTransform(transform, style.ComputedStyle(), transformData, options);
  }

  TransformationMatrix TransformResolver::ComputeTransform(const ComputedStyle &style,
                                                           const TransformOperationData &transformData,
                                                           EnumSet<Option> options) noexcept
  {
    TransformationMatrix transform;
    TransformResolver::ApplyTransform(transform, style, transformData, options);
    return transform;
  }

  TransformationMatrix TransformResolver::ComputeTransform(const RenderStyle &style,
                                                           const TransformOperationData &transformData,
                                                           EnumSet<Option> options) noexcept
  {
    return ComputeTransform(style.ComputedStyle(), transformData, options);
  }

  void TransformResolver::ApplyMotionPathTransform(const TransformOperationData &transformData) noexcept
  {
    auto offsetPath = TryPath(_style.GetOffsetPath(), transformData);
    if (!offsetPath)
    {
      return;
    }

    auto &boundingBox = transformData.BoundingBox;

    auto transformOrigin = ComputeTransformOrigin(boundingBox).XY();
    auto transformBox = _style.GetTransformBox();

    auto offsetDistance = Evaluate<float>(_style.GetOffsetDistance(), offsetPath->Length(), ZoomNeeded {});
    auto offsetAnchor = krys::SwitchOn(
      _style.GetOffsetAnchor(), [&](const Position &position) -> Maybe<FloatPoint>
      { return Evaluate<FloatPoint>(position, boundingBox.Size(), ZoomNeeded {}); },
      [&](const css::keywords::Auto &) -> Maybe<FloatPoint> { return null; });
    auto offsetRotate = _style.GetOffsetRotate().GetAngle().Value;
    auto offsetRotateHasAuto = _style.GetOffsetRotate().HasAuto();

    MotionPath::ApplyMotionPathTransform(_transform, transformData, transformOrigin, transformBox,
                                         *offsetPath, offsetAnchor, offsetDistance, offsetRotate,
                                         offsetRotateHasAuto);
  }
}
