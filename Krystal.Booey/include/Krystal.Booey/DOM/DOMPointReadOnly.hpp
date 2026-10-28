#pragma once

#include "Krystal.Booey/DOM/Dicts/DOMPointInit.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Graphics/FloatPoint3D.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  struct DOMMatrixInit;
}

namespace krys::boo::dom
{
  class DOMPoint;

  /// @brief A point whose coordinates can't be changed through it.
  /// @see https://drafts.fxtf.org/geometry/#dompointreadonly
  class DOMPointReadOnly : public RefCounted<DOMPointReadOnly>
  {
  protected:
    // Any of these can be NaN or Inf.
    double _x;
    double _y;
    double _z;
    double _w;

  public:
    KRYS_NODISCARD static Ref<DOMPointReadOnly> Create(double x, double y, double z, double w) noexcept
    {
      return AdoptRef(*new DOMPointReadOnly(x, y, z, w));
    }
    KRYS_NODISCARD static Ref<DOMPointReadOnly> Create(const DOMPointInit &init) noexcept
    {
      return Create(init.X, init.Y, init.Z, init.W);
    }
    KRYS_NODISCARD static Ref<DOMPointReadOnly> FromPoint(const DOMPointInit &init) noexcept
    {
      return Create(init.X, init.Y, init.Z, init.W);
    }
    KRYS_NODISCARD static Ref<DOMPointReadOnly> FromFloatPoint(const FloatPoint3D &p) noexcept
    {
      return Create(p.X(), p.Y(), p.Z(), 1);
    }

    KRYS_NODISCARD double X() const noexcept
    {
      return _x;
    }
    KRYS_NODISCARD double Y() const noexcept
    {
      return _y;
    }
    KRYS_NODISCARD double Z() const noexcept
    {
      return _z;
    }
    KRYS_NODISCARD double W() const noexcept
    {
      return _w;
    }

    /// @see https://drafts.fxtf.org/geometry/#dom-dompointreadonly-matrixtransform
    KRYS_NODISCARD ExceptionOr<Ref<DOMPoint>> MatrixTransform(css::DOMMatrixInit &&) const noexcept;

  protected:
    DOMPointReadOnly(double x, double y, double z, double w) noexcept : _x(x), _y(y), _z(z), _w(w)
    {
    }
  };
}
