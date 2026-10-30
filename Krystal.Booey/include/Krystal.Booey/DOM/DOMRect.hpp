#pragma once

#include "Krystal.Booey/DOM/DOMRectReadOnly.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"

namespace krys::boo::dom
{
  /// @brief A rectangle whose coordinates can be changed.
  /// @see https://drafts.fxtf.org/geometry/#domrect
  class DOMRect : public DOMRectReadOnly
  {
    DOMRect() noexcept = default;
    DOMRect(double x, double y, double width, double height) noexcept : DOMRectReadOnly(x, y, width, height)
    {
    }

  public:
    KRYS_NODISCARD static Ref<DOMRect> Create() noexcept
    {
      return AdoptRef(*new DOMRect());
    }
    KRYS_NODISCARD static Ref<DOMRect> Create(double x, double y, double width, double height) noexcept
    {
      return AdoptRef(*new DOMRect(x, y, width, height));
    }
    KRYS_NODISCARD static Ref<DOMRect> Create(const FloatRect &rect) noexcept
    {
      return AdoptRef(*new DOMRect(rect.X(), rect.Y(), rect.Width(), rect.Height()));
    }
    KRYS_NODISCARD static Ref<DOMRect> Create(const IntRect &rect) noexcept
    {
      return AdoptRef(*new DOMRect(rect.X(), rect.Y(), rect.Width(), rect.Height()));
    }
    KRYS_NODISCARD static Ref<DOMRect> FromRect(const DOMRectInit &init) noexcept
    {
      return Create(init.X, init.Y, init.Width, init.Height);
    }

    void SetX(double x) noexcept
    {
      _x = x;
    }
    void SetY(double y) noexcept
    {
      _y = y;
    }
    void SetWidth(double width) noexcept
    {
      _width = width;
    }
    void SetHeight(double height) noexcept
    {
      _height = height;
    }
  };

  static_assert(sizeof(DOMRect) == sizeof(DOMRectReadOnly));
}
