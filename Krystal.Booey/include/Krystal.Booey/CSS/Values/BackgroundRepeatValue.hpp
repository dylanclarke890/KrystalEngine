#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Quad.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  enum class ValueId : uint16;

  class BackgroundRepeatValue final : public Value
  {
  private:
    css::ValueId _x;
    css::ValueId _y;

    BackgroundRepeatValue(css::ValueId repeatXValue, css::ValueId repeatYValue) noexcept;

  public:
    KRYS_NODISCARD static Ref<BackgroundRepeatValue> Create(css::ValueId repeatX,
                                                            css::ValueId repeatY) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const BackgroundRepeatValue &other) const noexcept;

    KRYS_NODISCARD css::ValueId x() const noexcept
    {
      return _x;
    }

    KRYS_NODISCARD css::ValueId y() const noexcept
    {
      return _y;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::BackgroundRepeatValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsBackgroundRepeatValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()