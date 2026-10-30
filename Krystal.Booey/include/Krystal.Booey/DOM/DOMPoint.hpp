#pragma once

#include "Krystal.Booey/DOM/DOMPointReadOnly.hpp"

namespace krys::boo::dom
{
  /// @brief A point whose coordinates can be changed.
  /// @see https://drafts.fxtf.org/geometry/#dompoint
  class DOMPoint final : public DOMPointReadOnly
  {
  public:
    KRYS_NODISCARD static Ref<DOMPoint> Create(double x, double y, double z = 0, double w = 1) noexcept
    {
      return AdoptRef(*new DOMPoint(x, y, z, w));
    }
    KRYS_NODISCARD static Ref<DOMPoint> Create(const DOMPointInit &init) noexcept
    {
      return Create(init.X, init.Y, init.Z, init.W);
    }
    KRYS_NODISCARD static Ref<DOMPoint> FromPoint(const DOMPointInit &init) noexcept
    {
      return Create(init.X, init.Y, init.Z, init.W);
    }

    void SetX(double x) noexcept
    {
      _x = x;
    }
    void SetY(double y) noexcept
    {
      _y = y;
    }
    void SetZ(double z) noexcept
    {
      _z = z;
    }
    void SetW(double w) noexcept
    {
      _w = w;
    }

  private:
    DOMPoint(double x, double y, double z, double w) noexcept : DOMPointReadOnly(x, y, z, w)
    {
    }
  };

  static_assert(sizeof(DOMPoint) == sizeof(DOMPointReadOnly));
}
