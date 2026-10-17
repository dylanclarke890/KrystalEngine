#pragma once

#include "Krystal.Booey/DOM/Dicts/DOMRectInit.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <cmath>

namespace krys::boo::dom
{
  /// @brief A rectangle whose coordinates can't be changed through it.
  /// @see https://drafts.fxtf.org/geometry/#domrectreadonly
  class DOMRectReadOnly : public RefCounted<DOMRectReadOnly>
  {
  protected:
    // Any of these can be NaN or Inf.
    double _x {0};
    double _y {0};
    double _width {0};  // Can be negative.
    double _height {0}; // Can be negative.

    DOMRectReadOnly() noexcept = default;
    DOMRectReadOnly(double x, double y, double width, double height) noexcept
        : _x(x), _y(y), _width(width), _height(height)
    {
    }

  public:
    KRYS_NODISCARD static Ref<DOMRectReadOnly> Create(double x, double y, double width,
                                                      double height) noexcept
    {
      return AdoptRef(*new DOMRectReadOnly(x, y, width, height));
    }

    KRYS_NODISCARD static Ref<DOMRectReadOnly> FromRect(const DOMRectInit &init) noexcept
    {
      return Create(init.X, init.Y, init.Width, init.Height);
    }

    KRYS_NODISCARD double X() const noexcept
    {
      return _x;
    }
    KRYS_NODISCARD double Y() const noexcept
    {
      return _y;
    }
    KRYS_NODISCARD double Width() const noexcept
    {
      return _width;
    }
    KRYS_NODISCARD double Height() const noexcept
    {
      return _height;
    }

    // Model NaN handling after Math.min, Math.max.
    KRYS_NODISCARD double Top() const noexcept
    {
      return NanPropagatingMin(_y, _y + _height);
    }
    KRYS_NODISCARD double Right() const noexcept
    {
      return NanPropagatingMax(_x, _x + _width);
    }
    KRYS_NODISCARD double Bottom() const noexcept
    {
      return NanPropagatingMax(_y, _y + _height);
    }
    KRYS_NODISCARD double Left() const noexcept
    {
      return NanPropagatingMin(_x, _x + _width);
    }

    KRYS_NODISCARD FloatRect ToFloatRect() const noexcept
    {
      return FloatRect(static_cast<float>(_x), static_cast<float>(_y), static_cast<float>(_width),
                       static_cast<float>(_height));
    }

  private:
    KRYS_NODISCARD static double NanPropagatingMin(double a, double b) noexcept
    {
      if (std::isnan(a) || std::isnan(b))
      {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return a < b ? a : b;
    }

    KRYS_NODISCARD static double NanPropagatingMax(double a, double b) noexcept
    {
      if (std::isnan(a) || std::isnan(b))
      {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return a > b ? a : b;
    }
  };
}
