#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  enum class ValueId : uint16;

  /// @brief A `background-repeat`/`mask-repeat` value: one repeat keyword per axis.
  class BackgroundRepeatValue final : public Value
  {
  private:
    css::ValueId _xValue;
    css::ValueId _yValue;

    BackgroundRepeatValue(css::ValueId repeatXValue, css::ValueId repeatYValue) noexcept;

  public:
    KRYS_NODISCARD static Ref<BackgroundRepeatValue> Create(css::ValueId repeatXValue,
                                                            css::ValueId repeatYValue) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const BackgroundRepeatValue &other) const noexcept;

    KRYS_NODISCARD css::ValueId XValue() const noexcept
    {
      return _xValue;
    }

    KRYS_NODISCARD css::ValueId YValue() const noexcept
    {
      return _yValue;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::BackgroundRepeatValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsBackgroundRepeatValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
