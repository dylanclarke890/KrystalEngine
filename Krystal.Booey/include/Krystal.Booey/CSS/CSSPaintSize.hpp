#pragma once

#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSPaintSize : public RefCounted<CSSPaintSize>
  {
  private:
    double _width;
    double _height;

    CSSPaintSize(double width, double height) noexcept : _width(width), _height(height)
    {
    }

  public:
    KRYS_NODISCARD static Ref<CSSPaintSize> Create(double width, double height) noexcept
    {
      return AdoptRef(*new CSSPaintSize(width, height));
    }

    KRYS_NODISCARD double width() const noexcept
    {
      return _width;
    }

    KRYS_NODISCARD double height() const noexcept
    {
      return _height;
    }
  };
}
