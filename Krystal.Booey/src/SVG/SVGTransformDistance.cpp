/*
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/SVG/SVGTransformDistance.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"
#include <cmath>

namespace krys::boo::svg
{
  SVGTransformDistance::SVGTransformDistance(SVGTransformValue::SVGTransformType type, float angle, float cx,
                                             float cy, const AffineTransform &transform) noexcept
      : _type(type), _angle(angle), _cx(cx), _cy(cy), _transform(transform)
  {
  }

  SVGTransformDistance::SVGTransformDistance(const SVGTransformValue &fromSVGTransform,
                                             const SVGTransformValue &toSVGTransform) noexcept
      : _type(fromSVGTransform.Type())
  {
    krys_debug_assert(_type == toSVGTransform.Type());

    switch (_type)
    {
      case SVGTransformValue::SVG_TRANSFORM_MATRIX:
        krys_debug_assert_msg(false, "Matrix transforms aren't animated by distance");
        KRYS_FALLTHROUGH;
      case SVGTransformValue::SVG_TRANSFORM_UNKNOWN: break;
      case SVGTransformValue::SVG_TRANSFORM_ROTATE:
      {
        FloatSize centerDistance = toSVGTransform.RotationCenter() - fromSVGTransform.RotationCenter();
        _angle = toSVGTransform.Angle() - fromSVGTransform.Angle();
        _cx = centerDistance.Width();
        _cy = centerDistance.Height();
        break;
      }
      case SVGTransformValue::SVG_TRANSFORM_TRANSLATE:
      {
        FloatSize translationDistance = toSVGTransform.Translate() - fromSVGTransform.Translate();
        _transform.Translate(translationDistance.Width(), translationDistance.Height());
        break;
      }
      case SVGTransformValue::SVG_TRANSFORM_SCALE:
      {
        float scaleX = toSVGTransform.Scale().Width() - fromSVGTransform.Scale().Width();
        float scaleY = toSVGTransform.Scale().Height() - fromSVGTransform.Scale().Height();
        _transform.ScaleNonUniform(scaleX, scaleY);
        break;
      }
      case SVGTransformValue::SVG_TRANSFORM_SKEWX:
      case SVGTransformValue::SVG_TRANSFORM_SKEWY:
        _angle = toSVGTransform.Angle() - fromSVGTransform.Angle();
        break;
    }
  }

  SVGTransformDistance SVGTransformDistance::ScaledDistance(float scaleFactor) const noexcept
  {
    switch (_type)
    {
      case SVGTransformValue::SVG_TRANSFORM_MATRIX:
        krys_debug_assert_msg(false, "Matrix transforms aren't animated by distance");
        KRYS_FALLTHROUGH;
      case SVGTransformValue::SVG_TRANSFORM_UNKNOWN: return {};
      case SVGTransformValue::SVG_TRANSFORM_ROTATE:
        return {_type, _angle * scaleFactor, _cx * scaleFactor, _cy * scaleFactor, AffineTransform()};
      case SVGTransformValue::SVG_TRANSFORM_SCALE:
        return {_type, _angle * scaleFactor, _cx * scaleFactor, _cy * scaleFactor,
                AffineTransform(_transform).Scale(scaleFactor)};
      case SVGTransformValue::SVG_TRANSFORM_TRANSLATE:
      {
        AffineTransform newTransform(_transform);
        newTransform.SetE(_transform.E() * scaleFactor);
        newTransform.SetF(_transform.F() * scaleFactor);
        return {_type, 0, 0, 0, newTransform};
      }
      case SVGTransformValue::SVG_TRANSFORM_SKEWX:
      case SVGTransformValue::SVG_TRANSFORM_SKEWY:
        return {_type, _angle * scaleFactor, _cx * scaleFactor, _cy * scaleFactor, AffineTransform()};
    }

    krys_unreachable();
  }

  SVGTransformValue SVGTransformDistance::AddSVGTransforms(const SVGTransformValue &first,
                                                           const SVGTransformValue &second,
                                                           uint32 repeatCount) noexcept
  {
    krys_debug_assert(first.Type() == second.Type());

    SVGTransformValue transform;
    auto repeat = static_cast<float>(repeatCount);

    switch (first.Type())
    {
      case SVGTransformValue::SVG_TRANSFORM_MATRIX:
        krys_debug_assert_msg(false, "Matrix transforms aren't animated by distance");
        KRYS_FALLTHROUGH;
      case SVGTransformValue::SVG_TRANSFORM_UNKNOWN: return {};
      case SVGTransformValue::SVG_TRANSFORM_ROTATE:
      {
        transform.SetRotate(first.Angle() + second.Angle() * repeat,
                            first.RotationCenter().X() + second.RotationCenter().X() * repeat,
                            first.RotationCenter().Y() + second.RotationCenter().Y() * repeat);
        return transform;
      }
      case SVGTransformValue::SVG_TRANSFORM_TRANSLATE:
      {
        float dx = first.Translate().X() + second.Translate().X() * repeat;
        float dy = first.Translate().Y() + second.Translate().Y() * repeat;
        transform.SetTranslate(dx, dy);
        return transform;
      }
      case SVGTransformValue::SVG_TRANSFORM_SCALE:
      {
        FloatSize scale = second.Scale();
        scale.Scale(repeat);
        scale += first.Scale();
        transform.SetScale(scale.Width(), scale.Height());
        return transform;
      }
      case SVGTransformValue::SVG_TRANSFORM_SKEWX:
        transform.SetSkewX(first.Angle() + second.Angle() * repeat);
        return transform;
      case SVGTransformValue::SVG_TRANSFORM_SKEWY:
        transform.SetSkewY(first.Angle() + second.Angle() * repeat);
        return transform;
    }

    krys_unreachable();
  }

  SVGTransformValue SVGTransformDistance::AddToSVGTransform(const SVGTransformValue &transform) const noexcept
  {
    krys_debug_assert(_type == transform.Type() || !transform.IsValid());

    SVGTransformValue newTransform(transform);

    switch (_type)
    {
      case SVGTransformValue::SVG_TRANSFORM_MATRIX:
        krys_debug_assert_msg(false, "Matrix transforms aren't animated by distance");
        KRYS_FALLTHROUGH;
      case SVGTransformValue::SVG_TRANSFORM_UNKNOWN: return {};
      case SVGTransformValue::SVG_TRANSFORM_TRANSLATE:
      {
        FloatPoint translation = transform.Translate();
        translation += FloatSize::NarrowPrecision(_transform.E(), _transform.F());
        newTransform.SetTranslate(translation.X(), translation.Y());
        return newTransform;
      }
      case SVGTransformValue::SVG_TRANSFORM_SCALE:
      {
        FloatSize scale = transform.Scale();
        scale += FloatSize::NarrowPrecision(_transform.A(), _transform.D());
        newTransform.SetScale(scale.Width(), scale.Height());
        return newTransform;
      }
      case SVGTransformValue::SVG_TRANSFORM_ROTATE:
      {
        FloatPoint center = transform.RotationCenter();
        newTransform.SetRotate(transform.Angle() + _angle, center.X() + _cx, center.Y() + _cy);
        return newTransform;
      }
      case SVGTransformValue::SVG_TRANSFORM_SKEWX:
        newTransform.SetSkewX(transform.Angle() + _angle);
        return newTransform;
      case SVGTransformValue::SVG_TRANSFORM_SKEWY:
        newTransform.SetSkewY(transform.Angle() + _angle);
        return newTransform;
    }

    krys_unreachable();
  }

  float SVGTransformDistance::Distance() const noexcept
  {
    switch (_type)
    {
      case SVGTransformValue::SVG_TRANSFORM_MATRIX:
        krys_debug_assert_msg(false, "Matrix transforms aren't animated by distance");
        KRYS_FALLTHROUGH;
      case SVGTransformValue::SVG_TRANSFORM_UNKNOWN: return 0;
      case SVGTransformValue::SVG_TRANSFORM_ROTATE:  return std::hypot(_angle, _cx, _cy);
      case SVGTransformValue::SVG_TRANSFORM_SCALE:
        return static_cast<float>(std::hypot(_transform.A(), _transform.D()));
      case SVGTransformValue::SVG_TRANSFORM_TRANSLATE:
        return static_cast<float>(std::hypot(_transform.E(), _transform.F()));
      case SVGTransformValue::SVG_TRANSFORM_SKEWX:
      case SVGTransformValue::SVG_TRANSFORM_SKEWY: return _angle;
    }

    krys_unreachable();
  }
}
