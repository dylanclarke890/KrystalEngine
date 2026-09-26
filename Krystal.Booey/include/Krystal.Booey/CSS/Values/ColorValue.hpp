#pragma once

#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo
{
  class Color;
}

namespace krys::boo::css
{
  class ColorValue final : public css::Value
  {
    friend class ValuePool;
    friend class StaticValuePool;
    friend LazyNeverDestroyed<ColorValue>;

  private:
    css::Color _color;

    ColorValue(css::Color color) noexcept;

    ColorValue(boo::Color color) noexcept;

    ColorValue(StaticValueTag, boo::Color color) noexcept;

  public:
    ~ColorValue() noexcept = default;

    KRYS_NODISCARD static Ref<ColorValue> Create(css::Color) noexcept;

    KRYS_NODISCARD static Ref<ColorValue> Create(boo::Color) noexcept;

    KRYS_NODISCARD static boo::Color AbsoluteColor(const Value &value) noexcept;

    KRYS_NODISCARD const css::Color &Color() const noexcept
    {
      return _color;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ColorValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsColorValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()