#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Quad.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class BorderImageSliceValue final : public Value
  {
  private:
    // These four values are used to make "cuts" in the border image. They can be numbers
    // or percentages.
    Quad _slices;
    bool _fill {false};

    BorderImageSliceValue(Quad slices, bool fill) noexcept
        : Value(ValueType::BorderImageSlice), _slices(krys::move(slices)), _fill(fill)
    {
    }

  public:
    KRYS_NODISCARD static Ref<BorderImageSliceValue> Create(Quad slices, bool fill) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const BorderImageSliceValue &other) const noexcept;

    KRYS_NODISCARD const Quad &Slices() const noexcept
    {
      return _slices;
    }

    KRYS_NODISCARD bool Fill() const noexcept
    {
      return _fill;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::BorderImageSliceValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsBorderImageSliceValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()