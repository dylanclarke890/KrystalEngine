#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Quad.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class BorderImageWidthValue final : public Value
  {
  private:
    Quad _widths;
    bool _overridesBorderWidths {false};

    BorderImageWidthValue(Quad widths, bool overridesBorderWidths) noexcept
        : Value(ValueType::BorderImageWidth), _widths(krys::move(widths)),
          _overridesBorderWidths(overridesBorderWidths)
    {
    }

  public:
    KRYS_NODISCARD static Ref<BorderImageWidthValue> Create(Quad widths, bool overridesBorderWidths) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const BorderImageWidthValue &other) const noexcept;

    KRYS_NODISCARD const Quad &Widths() const noexcept
    {
      return _widths;
    }

    KRYS_NODISCARD bool OverridesBorderWidths() const noexcept
    {
      return _overridesBorderWidths;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::BorderImageWidthValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsBorderImageWidthValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()