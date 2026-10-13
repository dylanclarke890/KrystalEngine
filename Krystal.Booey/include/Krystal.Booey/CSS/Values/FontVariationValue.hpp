#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/Graphics/FontTaggedSettings.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief One `font-variation-settings` entry: an axis tag and its value, e.g. `"wght" 700`.
  class FontVariationValue final : public Value
  {
  private:
    FontTag _tag;
    const Ref<Value> _value;

    FontVariationValue(FontTag tag, Ref<Value> value) noexcept;

  public:
    KRYS_NODISCARD static Ref<FontVariationValue> Create(FontTag tag, Ref<Value> value) noexcept
    {
      return AdoptRef(*new FontVariationValue(tag, krys::move(value)));
    }

    KRYS_NODISCARD const FontTag &Tag() const noexcept
    {
      return _tag;
    }

    KRYS_NODISCARD const Value &GetValue() const noexcept
    {
      return *_value;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const FontVariationValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      if (func(*_value) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      return IterationStatus::Continue;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FontVariationValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsFontVariationValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
