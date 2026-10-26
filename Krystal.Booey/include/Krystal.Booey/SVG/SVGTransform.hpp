#pragma once

#include "Krystal.Booey/CSS/DOMMatrix2DInit.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/SVG/Properties/SVGValueProperty.hpp"
#include "Krystal.Booey/SVG/SVGMatrix.hpp"
#include "Krystal.Booey/SVG/SVGTransformValue.hpp"

namespace krys::boo::svg
{
  /// @see https://svgwg.org/svg2-draft/coords.html#InterfaceSVGTransform
  class SVGTransform : public SVGValueProperty<SVGTransformValue>, public SVGPropertyOwner
  {
    using Base = SVGValueProperty<SVGTransformValue>;

  public:
    KRYS_NODISCARD static Ref<SVGTransform> Create(SVGTransformValue::SVGTransformType type) noexcept
    {
      return AdoptRef(*new SVGTransform(type));
    }

    KRYS_NODISCARD static Ref<SVGTransform> Create(const AffineTransform &transform = {}) noexcept
    {
      return AdoptRef(*new SVGTransform(SVGTransformValue::SVG_TRANSFORM_MATRIX, transform));
    }

    KRYS_NODISCARD static Ref<SVGTransform> Create(const SVGTransformValue &value) noexcept
    {
      return AdoptRef(
        *new SVGTransform(value.Type(), value.Matrix().Value(), value.Angle(), value.RotationCenter()));
    }

    KRYS_NODISCARD static Ref<SVGTransform> Create(SVGTransformValue &&value) noexcept
    {
      return AdoptRef(*new SVGTransform(krys::move(value)));
    }

    template <typename T>
    KRYS_NODISCARD static dom::ExceptionOr<Ref<SVGTransform>> Create(dom::ExceptionOr<T> &&value) noexcept
    {
      if (value.HasException())
      {
        return value.ReleaseException();
      }
      return Create(value.ReleaseValue());
    }

    ~SVGTransform() noexcept override
    {
      _value.ProtectedMatrix()->Detach();
    }

    KRYS_NODISCARD Ref<SVGTransform> Clone() const noexcept
    {
      return SVGTransform::Create(_value);
    }

    KRYS_NODISCARD uint16 Type() noexcept
    {
      return _value.Type();
    }

    KRYS_NODISCARD float Angle() noexcept
    {
      return _value.Angle();
    }

    KRYS_NODISCARD SVGMatrix &Matrix() noexcept
    {
      return _value.Matrix();
    }

    KRYS_NODISCARD Ref<SVGMatrix> ProtectedMatrix() noexcept
    {
      return ShareRef(Matrix());
    }

    dom::ExceptionOr<void> SetMatrix(css::DOMMatrix2DInit &&matrixInit) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      AffineTransform transform;
      if (matrixInit.A)
      {
        transform.SetA(matrixInit.A.value());
      }
      if (matrixInit.B)
      {
        transform.SetB(matrixInit.B.value());
      }
      if (matrixInit.C)
      {
        transform.SetC(matrixInit.C.value());
      }
      if (matrixInit.D)
      {
        transform.SetD(matrixInit.D.value());
      }
      if (matrixInit.E)
      {
        transform.SetE(matrixInit.E.value());
      }
      if (matrixInit.F)
      {
        transform.SetF(matrixInit.F.value());
      }

      _value.SetMatrix(transform);
      CommitChange();
      return {};
    }

    dom::ExceptionOr<void> SetTranslate(float tx, float ty) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value.SetTranslate(tx, ty);
      CommitChange();
      return {};
    }

    dom::ExceptionOr<void> SetScale(float sx, float sy) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value.SetScale(sx, sy);
      CommitChange();
      return {};
    }

    dom::ExceptionOr<void> SetRotate(float angle, float cx, float cy) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value.SetRotate(angle, cx, cy);
      CommitChange();
      return {};
    }

    dom::ExceptionOr<void> SetSkewX(float angle) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value.SetSkewX(angle);
      CommitChange();
      return {};
    }

    dom::ExceptionOr<void> SetSkewY(float angle) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value.SetSkewY(angle);
      CommitChange();
      return {};
    }

    void Attach(SVGPropertyOwner *owner, SVGPropertyAccess access) noexcept override
    {
      Base::Attach(owner, access);
      // Reattach the SVGMatrix to the SVGTransformValue with the new SVGPropertyAccess.
      _value.ProtectedMatrix()->Reattach(this, access);
    }

    void Detach() noexcept override
    {
      Base::Detach();
      // Reattach the SVGMatrix to the SVGTransformValue with the SVGPropertyAccess::ReadWrite.
      _value.ProtectedMatrix()->Reattach(this, Access());
    }

  private:
    SVGTransform(SVGTransformValue::SVGTransformType type, const AffineTransform &transform = {},
                 float angle = 0, const FloatPoint &rotationCenter = {}) noexcept
        : Base(SVGTransformValue(type, SVGMatrix::Create(this, SVGPropertyAccess::ReadWrite, transform),
                                 angle, rotationCenter))
    {
    }

    SVGTransform(SVGTransformValue &&value) noexcept : Base(krys::move(value))
    {
      _value.ProtectedMatrix()->Attach(this, SVGPropertyAccess::ReadWrite);
    }

    KRYS_NODISCARD SVGPropertyOwner *Owner() const noexcept override
    {
      return _owner;
    }

    void CommitPropertyChange(KRYS_MAYBE_UNUSED SVGProperty *property) noexcept override
    {
      krys_assert(property == &_value.Matrix());
      if (Owner())
      {
        Owner()->CommitPropertyChange(this);
      }
      _value.MatrixDidChange();
    }

    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept override
    {
      return _value.ValueAsString();
    }
  };
}
