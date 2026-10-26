#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/SVG/Properties/SVGValueProperty.hpp"

namespace krys::boo::svg
{
  // FIXME: Remove this class once SVGMatrix becomes an alias to DOMMatrix.
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGMatrix
  class SVGMatrix : public SVGValueProperty<AffineTransform>
  {
    using Base = SVGValueProperty<AffineTransform>;
    using Base::Base;

  public:
    KRYS_NODISCARD static Ref<SVGMatrix> Create(const AffineTransform &value = {}) noexcept
    {
      return AdoptRef(*new SVGMatrix(value));
    }

    KRYS_NODISCARD static Ref<SVGMatrix> Create(SVGPropertyOwner *owner, SVGPropertyAccess access,
                                                const AffineTransform &value = {}) noexcept
    {
      return AdoptRef(*new SVGMatrix(owner, access, value));
    }

    template <typename T>
    KRYS_NODISCARD static dom::ExceptionOr<Ref<SVGMatrix>> Create(dom::ExceptionOr<T> &&value) noexcept
    {
      if (value.HasException())
      {
        return value.ReleaseException();
      }
      return Create(value.ReleaseValue());
    }

    KRYS_NODISCARD double A() const noexcept
    {
      return _value.A();
    }

    dom::ExceptionOr<void> SetA(double value) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value.SetA(value);
      CommitChange();
      return {};
    }

    KRYS_NODISCARD double B() const noexcept
    {
      return _value.B();
    }

    dom::ExceptionOr<void> SetB(double value) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value.SetB(value);
      CommitChange();
      return {};
    }

    KRYS_NODISCARD double C() const noexcept
    {
      return _value.C();
    }

    dom::ExceptionOr<void> SetC(double value) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value.SetC(value);
      CommitChange();
      return {};
    }

    KRYS_NODISCARD double D() const noexcept
    {
      return _value.D();
    }

    dom::ExceptionOr<void> SetD(double value) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value.SetD(value);
      CommitChange();
      return {};
    }

    KRYS_NODISCARD double E() const noexcept
    {
      return _value.E();
    }

    dom::ExceptionOr<void> SetE(double value) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value.SetE(value);
      CommitChange();
      return {};
    }

    KRYS_NODISCARD double F() const noexcept
    {
      return _value.F();
    }

    dom::ExceptionOr<void> SetF(double value) noexcept
    {
      if (IsReadOnly())
      {
        return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
      }

      _value.SetF(value);
      CommitChange();
      return {};
    }

    KRYS_NODISCARD Ref<SVGMatrix> Multiply(SVGMatrix &secondMatrix) const noexcept
    {
      auto copy = _value;
      copy.Multiply(secondMatrix.Value());
      return SVGMatrix::Create(copy);
    }

    KRYS_NODISCARD dom::ExceptionOr<Ref<SVGMatrix>> Inverse() const noexcept
    {
      auto inverse = _value.Inverse();
      if (!inverse)
      {
        return dom::Exception {dom::ExceptionCode::InvalidStateError, u8"Matrix is not invertible"};
      }

      return SVGMatrix::Create(*inverse);
    }

    KRYS_NODISCARD Ref<SVGMatrix> Translate(float x, float y) const noexcept
    {
      auto copy = _value;
      copy.Translate(x, y);
      return SVGMatrix::Create(copy);
    }

    KRYS_NODISCARD Ref<SVGMatrix> Scale(float scaleFactor) const noexcept
    {
      auto copy = _value;
      copy.Scale(scaleFactor);
      return SVGMatrix::Create(copy);
    }

    KRYS_NODISCARD Ref<SVGMatrix> ScaleNonUniform(float scaleFactorX, float scaleFactorY) const noexcept
    {
      auto copy = _value;
      copy.ScaleNonUniform(scaleFactorX, scaleFactorY);
      return SVGMatrix::Create(copy);
    }

    KRYS_NODISCARD Ref<SVGMatrix> Rotate(float angle) const noexcept
    {
      auto copy = _value;
      copy.Rotate(angle);
      return SVGMatrix::Create(copy);
    }

    KRYS_NODISCARD dom::ExceptionOr<Ref<SVGMatrix>> RotateFromVector(float x, float y) const noexcept
    {
      if (!x || !y)
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }

      auto copy = _value;
      copy.RotateFromVector(x, y);
      return SVGMatrix::Create(copy);
    }

    KRYS_NODISCARD Ref<SVGMatrix> FlipX() const noexcept
    {
      auto copy = _value;
      copy.FlipX();
      return SVGMatrix::Create(copy);
    }

    KRYS_NODISCARD Ref<SVGMatrix> FlipY() const noexcept
    {
      auto copy = _value;
      copy.FlipY();
      return SVGMatrix::Create(copy);
    }

    KRYS_NODISCARD Ref<SVGMatrix> SkewX(float angle) const noexcept
    {
      auto copy = _value;
      copy.SkewX(angle);
      return SVGMatrix::Create(copy);
    }

    KRYS_NODISCARD Ref<SVGMatrix> SkewY(float angle) const noexcept
    {
      auto copy = _value;
      copy.SkewY(angle);
      return SVGMatrix::Create(copy);
    }
  };
}
