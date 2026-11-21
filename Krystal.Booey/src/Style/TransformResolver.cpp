/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/TransformResolver.hpp"
#include "Krystal.Booey/Rendering/MotionPath.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Rendering/TransformOperationData.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Geometry/Point3.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Geometry/TransformationMatrix.hpp"

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
    return style.Rotate().AffectedByTransformOrigin() || style.Scale().AffectedByTransformOrigin()
           || style.Transform().AffectedByTransformOrigin() || style.OffsetPath().AffectedByTransformOrigin();
  }

  bool TransformResolver::AffectedByTransformOrigin(const RenderStyle &style) noexcept
  {
    return AffectedByTransformOrigin(style.ComputedStyle());
  }

  bool TransformResolver::AffectedByTransformOrigin() const noexcept
  {
    return AffectedByTransformOrigin(_style);
  }

  Point3F TransformResolver::ComputeTransformOrigin(const ComputedStyle &style,
                                                    const RectF &boundingBox) noexcept
  {
    Point3F originTranslate;
    originTranslate.SetXY(
      boundingBox.Location()
      + Evaluate<PointF>(style.TransformOrigin().XY(), boundingBox.Size(), ZoomNeeded {}));
    originTranslate.SetZ(style.TransformOriginZ().ResolveZoom(ZoomNeeded {}));
    return originTranslate;
  }

  Point3F TransformResolver::ComputeTransformOrigin(const RenderStyle &style,
                                                    const RectF &boundingBox) noexcept
  {
    return ComputeTransformOrigin(style.ComputedStyle(), boundingBox);
  }

  Point3F TransformResolver::ComputeTransformOrigin(const RectF &boundingBox) const noexcept
  {
    return ComputeTransformOrigin(_style, boundingBox);
  }

  PointF TransformResolver::ComputePerspectiveOrigin(const ComputedStyle &style,
                                                     const RectF &boundingBox) noexcept
  {
    return boundingBox.Location()
           + Evaluate<PointF>(style.PerspectiveOrigin(), boundingBox.Size(), ZoomNeeded {});
  }

  PointF TransformResolver::ComputePerspectiveOrigin(const RenderStyle &style,
                                                     const RectF &boundingBox) noexcept
  {
    return ComputePerspectiveOrigin(style.ComputedStyle(), boundingBox);
  }

  PointF TransformResolver::ComputePerspectiveOrigin(const RectF &boundingBox) const noexcept
  {
    return ComputePerspectiveOrigin(_style, boundingBox);
  }

  void TransformResolver::ApplyPerspective(const PointF &originTranslate) noexcept
  {
    // https://www.w3.org/TR/css-transforms-2/#perspective
    // The perspective matrix is computed as follows:
    // 1. Start with the identity matrix.

    // 2. Translate by the computed X and Y values of perspective-origin
    _transform.Translate(originTranslate.X(), originTranslate.Y());

    // 3. Multiply by the matrix that would be obtained from the perspective() transform function, where the
    // length is provided by the value of the perspective property
    _transform.ApplyPerspective(_style.Perspective().UsedPerspective());

    // 4. Translate by the negated computed X and Y values of perspective-origin
    _transform.Translate(-originTranslate.X(), -originTranslate.Y());
  }

  void TransformResolver::ApplyTransformOrigin(const Point3F &originTranslate) noexcept
  {
    if (!originTranslate.IsZero())
    {
      _transform.Translate3d(originTranslate.X(), originTranslate.Y(), originTranslate.Z());
    }
  }

  void TransformResolver::UnapplyTransformOrigin(const Point3F &originTranslate) noexcept
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
    if (options.contains(Option::Translate))
    {
      _style.Translate().Apply(_transform, boundingBox.Size());
    }

    // 4. Rotate by the computed <angle> about the specified axis of rotate.
    if (options.contains(Option::Rotate))
    {
      _style.Rotate().Apply(_transform, boundingBox.Size());
    }

    // 5. Scale by the computed X, Y, and Z values of scale.
    if (options.contains(Option::Scale))
    {
      _style.Scale().Apply(_transform, boundingBox.Size());
    }

    // 6. Translate and rotate by the transform specified by offset.
    if (options.contains(Option::Offset))
    {
      ApplyMotionPathTransform(transformData);
    }

    // 7. Multiply by each of the transform functions in transform from left to right.
    _style.Transform().Apply(_transform, boundingBox.Size());

    // 8. Translate by the negated computed X, Y and Z values of transform-origin.
    // (implemented in UnapplyTransformOrigin)
  }

  void TransformResolver::ApplyTransform(const TransformOperationData &transformData,
                                         EnumSet<Option> options) noexcept
  {
    if (!options.contains(Option::TransformOrigin) || !AffectedByTransformOrigin())
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
    auto offsetPath = TryPath(_style.OffsetPath(), transformData);
    if (!offsetPath)
    {
      return;
    }

    auto &boundingBox = transformData.BoundingBox;

    auto transformOrigin = ComputeTransformOrigin(boundingBox).XY();
    auto transformBox = _style.TransformBox();

    auto offsetDistance = Evaluate<float>(_style.OffsetDistance(), offsetPath->Length(), ZoomNeeded {});
    auto offsetAnchor = krys::SwitchOn(
      _style.OffsetAnchor(), [&](const Position &position) -> Maybe<PointF>
      { return Evaluate<PointF>(position, boundingBox.Size(), ZoomNeeded {}); },
      [&](const css::keywords::Auto &) -> Maybe<PointF> { return null; });
    auto offsetRotate = _style.OffsetRotate().GetAngle().Value;
    auto offsetRotateHasAuto = _style.OffsetRotate().HasAuto();

    MotionPath::ApplyMotionPathTransform(_transform, transformData, transformOrigin, transformBox,
                                         *offsetPath, offsetAnchor, offsetDistance, offsetRotate,
                                         offsetRotateHasAuto);
  }
}
