#pragma once

#include "Krystal.Booey/CSS/Values/Rect.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `rect()` value, e.g. `clip: rect(1px, 2px, 3px, 4px)`.
  class RectValue final : public Value
  {
  private:
    Rect _rect;

    explicit RectValue(Rect rect) noexcept;

  public:
    KRYS_NODISCARD static Ref<RectValue> Create(Rect rect) noexcept;

    KRYS_NODISCARD const Rect &GetRect() const noexcept
    {
      return _rect;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const RectValue &other) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::RectValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsRectValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

namespace krys::boo::css
{
  inline const Rect &Value::GetRect() const noexcept
  {
    return Downcast<RectValue>(*this).GetRect();
  }
}
