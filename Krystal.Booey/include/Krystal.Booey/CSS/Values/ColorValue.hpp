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
  /// @brief A `<color>` value.
  class ColorValue final : public css::Value
  {
    friend class ValuePool;
    friend class StaticValuePool;
    friend LazyNeverDestroyed<ColorValue>;

  private:
    css::Color _color;

    explicit ColorValue(css::Color color) noexcept;
    explicit ColorValue(boo::Color color) noexcept;
    ColorValue(StaticValueTag, boo::Color color) noexcept;

  public:
    KRYS_NODISCARD static Ref<ColorValue> Create(css::Color color) noexcept;
    KRYS_NODISCARD static Ref<ColorValue> Create(boo::Color color) noexcept;

    KRYS_NODISCARD const css::Color &GetColor() const noexcept
    {
      return _color;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const ColorValue &other) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(css::Value &)> &func) const noexcept;

    /// @brief The absolute colour of a `ColorValue` or an absolute colour keyword; otherwise an invalid
    /// colour.
    KRYS_NODISCARD static boo::Color GetAbsoluteColor(const Value &value) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ColorValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsColorValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
