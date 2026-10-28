#pragma once

#include "Krystal.Booey/CSS/DOMMatrixReadOnly.hpp"

namespace krys::boo::css
{
  /// @brief A 4 by 4 matrix (or a 2D one) that can be changed.
  /// @see https://drafts.fxtf.org/geometry/#dommatrix
  class DOMMatrix : public DOMMatrixReadOnly
  {
    friend class DOMMatrixReadOnly;

  public:
    /// @see https://drafts.fxtf.org/geometry/#dom-dommatrixreadonly-dommatrixreadonly
    KRYS_NODISCARD static dom::ExceptionOr<Ref<DOMMatrix>>
      Create(Maybe<Variant<CSSOMString, List<double>>> &&init = null) noexcept;

    KRYS_NODISCARD static Ref<DOMMatrix> Create(const TransformationMatrix &matrix, Is2D is2D) noexcept
    {
      return AdoptRef(*new DOMMatrix(matrix, is2D));
    }

    KRYS_NODISCARD static dom::ExceptionOr<Ref<DOMMatrix>> FromMatrix(DOMMatrixInit &&) noexcept;
    KRYS_NODISCARD static dom::ExceptionOr<Ref<DOMMatrix>> FromFloat32Array(Span<const float>) noexcept;
    KRYS_NODISCARD static dom::ExceptionOr<Ref<DOMMatrix>> FromFloat64Array(Span<const double>) noexcept;

    dom::ExceptionOr<Ref<DOMMatrix>> MultiplySelf(DOMMatrixInit &&other) noexcept;
    dom::ExceptionOr<Ref<DOMMatrix>> PreMultiplySelf(DOMMatrixInit &&other) noexcept;
    Ref<DOMMatrix> TranslateSelf(double tx = 0, double ty = 0, double tz = 0) noexcept;
    Ref<DOMMatrix> ScaleSelf(double scaleX = 1, Maybe<double> scaleY = null, double scaleZ = 1,
                             double originX = 0, double originY = 0, double originZ = 0) noexcept;
    Ref<DOMMatrix> Scale3dSelf(double scale = 1, double originX = 0, double originY = 0,
                               double originZ = 0) noexcept;
    /// @note Angles are in degrees.
    Ref<DOMMatrix> RotateSelf(double rotX = 0, Maybe<double> rotY = null, Maybe<double> rotZ = null) noexcept;
    Ref<DOMMatrix> RotateFromVectorSelf(double x = 0, double y = 0) noexcept;
    /// @note The angle is in degrees.
    Ref<DOMMatrix> RotateAxisAngleSelf(double x = 0, double y = 0, double z = 0, double angle = 0) noexcept;
    /// @note The angle is in degrees.
    Ref<DOMMatrix> SkewXSelf(double sx = 0) noexcept;
    /// @note The angle is in degrees.
    Ref<DOMMatrix> SkewYSelf(double sy = 0) noexcept;
    Ref<DOMMatrix> InvertSelf() noexcept;

    void SetA(double f) noexcept
    {
      _matrix.SetA(f);
    }
    void SetB(double f) noexcept
    {
      _matrix.SetB(f);
    }
    void SetC(double f) noexcept
    {
      _matrix.SetC(f);
    }
    void SetD(double f) noexcept
    {
      _matrix.SetD(f);
    }
    void SetE(double f) noexcept
    {
      _matrix.SetE(f);
    }
    void SetF(double f) noexcept
    {
      _matrix.SetF(f);
    }

    void SetM11(double f) noexcept
    {
      _matrix.SetM11(f);
    }
    void SetM12(double f) noexcept
    {
      _matrix.SetM12(f);
    }
    void SetM13(double f) noexcept;
    void SetM14(double f) noexcept;
    void SetM21(double f) noexcept
    {
      _matrix.SetM21(f);
    }
    void SetM22(double f) noexcept
    {
      _matrix.SetM22(f);
    }
    void SetM23(double f) noexcept;
    void SetM24(double f) noexcept;
    void SetM31(double f) noexcept;
    void SetM32(double f) noexcept;
    void SetM33(double f) noexcept;
    void SetM34(double f) noexcept;
    void SetM41(double f) noexcept
    {
      _matrix.SetM41(f);
    }
    void SetM42(double f) noexcept
    {
      _matrix.SetM42(f);
    }
    void SetM43(double f) noexcept;
    void SetM44(double f) noexcept;

  private:
    DOMMatrix() noexcept = default;
    DOMMatrix(const TransformationMatrix &, Is2D) noexcept;
  };

  static_assert(sizeof(DOMMatrix) == sizeof(DOMMatrixReadOnly));

  inline void DOMMatrix::SetM13(double f) noexcept
  {
    _matrix.SetM13(f);
    if (f)
    {
      _is2D = false;
    }
  }

  inline void DOMMatrix::SetM14(double f) noexcept
  {
    _matrix.SetM14(f);
    if (f)
    {
      _is2D = false;
    }
  }

  inline void DOMMatrix::SetM23(double f) noexcept
  {
    _matrix.SetM23(f);
    if (f)
    {
      _is2D = false;
    }
  }

  inline void DOMMatrix::SetM24(double f) noexcept
  {
    _matrix.SetM24(f);
    if (f)
    {
      _is2D = false;
    }
  }

  inline void DOMMatrix::SetM31(double f) noexcept
  {
    _matrix.SetM31(f);
    if (f)
    {
      _is2D = false;
    }
  }

  inline void DOMMatrix::SetM32(double f) noexcept
  {
    _matrix.SetM32(f);
    if (f)
    {
      _is2D = false;
    }
  }

  inline void DOMMatrix::SetM33(double f) noexcept
  {
    _matrix.SetM33(f);
    if (f != 1)
    {
      _is2D = false;
    }
  }

  inline void DOMMatrix::SetM34(double f) noexcept
  {
    _matrix.SetM34(f);
    if (f)
    {
      _is2D = false;
    }
  }

  inline void DOMMatrix::SetM43(double f) noexcept
  {
    _matrix.SetM43(f);
    if (f)
    {
      _is2D = false;
    }
  }

  inline void DOMMatrix::SetM44(double f) noexcept
  {
    _matrix.SetM44(f);
    if (f != 1)
    {
      _is2D = false;
    }
  }
}
