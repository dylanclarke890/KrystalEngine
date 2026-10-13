#pragma once

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief The `font` shorthand's components, each optional.
  class FontValue final : public Value
  {
  public:
    RefPtr<Value> Style;
    RefPtr<PrimitiveValue> Variant;
    RefPtr<PrimitiveValue> Weight;
    RefPtr<PrimitiveValue> Width;
    RefPtr<PrimitiveValue> Size;
    RefPtr<PrimitiveValue> LineHeight;
    RefPtr<Value> Family;

  private:
    FontValue() noexcept : Value(ValueType::Font)
    {
    }

  public:
    KRYS_NODISCARD static Ref<FontValue> Create() noexcept
    {
      return AdoptRef(*new FontValue);
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const FontValue &other) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FontValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsFontValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
