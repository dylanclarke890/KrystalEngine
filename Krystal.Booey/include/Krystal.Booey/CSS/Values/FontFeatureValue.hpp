#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/Graphics/FontTaggedSettings.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief One `font-feature-settings` entry: a feature tag and its value, e.g. `"liga" 0`.
  class FontFeatureValue final : public Value
  {
  private:
    FontTag _tag;
    const Ref<Value> _value;

    FontFeatureValue(FontTag tag, Ref<Value> value) noexcept;

  public:
    KRYS_NODISCARD static Ref<FontFeatureValue> Create(FontTag tag, Ref<Value> value) noexcept
    {
      return AdoptRef(*new FontFeatureValue(tag, krys::move(value)));
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
    KRYS_NODISCARD bool Equals(const FontFeatureValue &other) const noexcept;

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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FontFeatureValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsFontFeatureValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
