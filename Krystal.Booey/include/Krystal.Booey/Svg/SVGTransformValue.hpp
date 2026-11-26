/*
 * Copyright (C) 2004, 2005, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2019-2024 Apple Inc. All rights reserved.
 * Copyright (C) 2014 Google Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Dom/Types/DOMString.hpp"
#include "Krystal.Booey/Svg/SVGMatrix.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"

namespace krys::boo
{
  /// @brief One transform of a `transform` attribute: its kind, its matrix, and the angle and centre it was
  /// given.
  class SVGTransformValue
  {
  public:
    enum SVGTransformType : uint8
    {
      SVG_TRANSFORM_UNKNOWN = 0,
      SVG_TRANSFORM_MATRIX = 1,
      SVG_TRANSFORM_TRANSLATE = 2,
      SVG_TRANSFORM_SCALE = 3,
      SVG_TRANSFORM_ROTATE = 4,
      SVG_TRANSFORM_SKEWX = 5,
      SVG_TRANSFORM_SKEWY = 6,
    };

    enum ConstructionMode : uint8
    {
      ConstructIdentityTransform,
      ConstructZeroTransform,
    };

  private:
    SVGTransformType _type {SVG_TRANSFORM_UNKNOWN};
    Ref<SVGMatrix> _matrix;
    float _angle {0};
    PointF _rotationCenter;

  public:
    KRYS_NODISCARD static SVGTransformValue TranslateTransformValue(SizeF delta) noexcept
    {
      return SVGTransformValue(SVG_TRANSFORM_TRANSLATE, AffineTransform::CreateTranslation(delta));
    }

    KRYS_NODISCARD static SVGTransformValue RotateTransformValue(float angleInDegrees, PointF center) noexcept
    {
      auto result =
        SVGTransformValue(SVG_TRANSFORM_ROTATE, AffineTransform::CreateRotation(angleInDegrees, center));
      result._angle = angleInDegrees;
      result._rotationCenter = center;
      return result;
    }

    KRYS_NODISCARD static SVGTransformValue ScaleTransformValue(SizeF scale) noexcept
    {
      return SVGTransformValue(SVG_TRANSFORM_SCALE, AffineTransform::CreateScale(scale));
    }

    SVGTransformValue(SVGTransformType type = SVG_TRANSFORM_MATRIX,
                      const AffineTransform &transform = {}) noexcept
        : _type(type), _matrix(SVGMatrix::Create(transform))
    {
    }

    SVGTransformValue(const SVGTransformValue &other) noexcept
        : _type(other._type), _matrix(SVGMatrix::Create(other.Matrix().Value())), _angle(other._angle),
          _rotationCenter(other._rotationCenter)
    {
    }

    SVGTransformValue(SVGTransformType type, Ref<SVGMatrix> &&matrix, float angle,
                      const PointF &rotationCenter) noexcept
        : _type(type), _matrix(krys::move(matrix)), _angle(angle), _rotationCenter(rotationCenter)
    {
    }

    SVGTransformValue(SVGTransformValue &&other) noexcept
        : _type(other._type), _matrix(other._matrix), _angle(other._angle),
          _rotationCenter(other._rotationCenter)
    {
    }

    SVGTransformValue &operator=(const SVGTransformValue &other) noexcept
    {
      _type = other._type;
      _matrix->SetValue(other._matrix->Value());
      _angle = other._angle;
      _rotationCenter = other._rotationCenter;
      return *this;
    }

    KRYS_NODISCARD SVGTransformType Type() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD SVGMatrix &Matrix() const noexcept
    {
      return *_matrix;
    }

    KRYS_NODISCARD float Angle() const noexcept
    {
      return _angle;
    }

    KRYS_NODISCARD PointF RotationCenter() const noexcept
    {
      return _rotationCenter;
    }

    KRYS_NODISCARD bool IsValid() const noexcept
    {
      return _type != SVG_TRANSFORM_UNKNOWN;
    }

    void SetMatrix(const AffineTransform &matrix) noexcept
    {
      _type = SVG_TRANSFORM_MATRIX;
      _angle = 0;
      _rotationCenter = {};
      Matrix().SetValue(matrix);
    }

    void MatrixDidChange() noexcept
    {
      // The underlying matrix has been changed, alter the transformation type.
      // Spec: In case the matrix object is changed directly (i.e., without using the methods on the
      // SVGTransform interface itself) then the type of the SVGTransform changes to SVG_TRANSFORM_MATRIX.
      _type = SVG_TRANSFORM_MATRIX;
      _angle = 0;
      _rotationCenter = {};
    }

    KRYS_NODISCARD PointF Translate() const noexcept
    {
      return PointF::NarrowPrecision(_matrix->E(), _matrix->F());
    }

    void SetTranslate(float tx, float ty) noexcept
    {
      _type = SVG_TRANSFORM_TRANSLATE;
      _angle = 0;
      _rotationCenter = {};

      _matrix->Value() = AffineTransform::CreateTranslation({tx, ty});
    }

    KRYS_NODISCARD SizeF Scale() const noexcept
    {
      return SizeF::NarrowPrecision(_matrix->A(), _matrix->D());
    }

    void SetScale(float sx, float sy) noexcept
    {
      _type = SVG_TRANSFORM_SCALE;
      _angle = 0;
      _rotationCenter = {};

      _matrix->Value() = AffineTransform::CreateScale({sx, sy});
    }

    void SetRotate(float angleInDegrees, float cx, float cy) noexcept
    {
      _type = SVG_TRANSFORM_ROTATE;
      _angle = angleInDegrees;
      _rotationCenter = {cx, cy};

      _matrix->Value() = AffineTransform::CreateRotation(angleInDegrees, {cx, cy});
    }

    void SetSkewX(float angle) noexcept
    {
      _type = SVG_TRANSFORM_SKEWX;
      _angle = angle;
      _rotationCenter = PointF();

      _matrix->Value().MakeIdentity();
      _matrix->Value().SkewX(angle);
    }

    void SetSkewY(float angle) noexcept
    {
      _type = SVG_TRANSFORM_SKEWY;
      _angle = angle;
      _rotationCenter = PointF();

      _matrix->Value().MakeIdentity();
      _matrix->Value().SkewY(angle);
    }

    KRYS_NODISCARD DOMString ValueAsString() const noexcept;

    KRYS_NODISCARD static utf8_stringview PrefixForTransformType(SVGTransformType) noexcept;

  private:
    static void AppendFixedPrecisionNumbers(StringBuilder &) noexcept;

    template <typename TNumber, typename... TNumbers>
    static void AppendFixedPrecisionNumbers(StringBuilder &, TNumber, TNumbers...) noexcept;

    void AppendMatrix(StringBuilder &) const noexcept;

    void AppendTranslate(StringBuilder &) const noexcept;

    void AppendScale(StringBuilder &) const noexcept;

    void AppendRotate(StringBuilder &) const noexcept;

    void AppendSkewX(StringBuilder &) const noexcept;

    void AppendSkewY(StringBuilder &) const noexcept;
  };
}
