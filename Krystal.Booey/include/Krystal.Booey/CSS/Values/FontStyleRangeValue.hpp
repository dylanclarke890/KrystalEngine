#pragma once

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `font-style` descriptor value in `@font-face`: a style keyword, plus an optional range of
  /// oblique angles.
  class FontStyleRangeValue final : public Value
  {
  public:
    Ref<PrimitiveValue> FontStyleValue;
    RefPtr<ValueList> ObliqueValues;

  private:
    FontStyleRangeValue(Ref<PrimitiveValue> fontStyleValue, RefPtr<ValueList> obliqueValues) noexcept
        : Value(ValueType::FontStyleRange), FontStyleValue(krys::move(fontStyleValue)),
          ObliqueValues(krys::move(obliqueValues))
    {
    }

  public:
    KRYS_NODISCARD static Ref<FontStyleRangeValue> Create(Ref<PrimitiveValue> fontStyleValue) noexcept
    {
      return AdoptRef(*new FontStyleRangeValue(krys::move(fontStyleValue), nullptr));
    }

    KRYS_NODISCARD static Ref<FontStyleRangeValue> Create(Ref<PrimitiveValue> fontStyleValue,
                                                          RefPtr<ValueList> obliqueValues) noexcept
    {
      return AdoptRef(*new FontStyleRangeValue(krys::move(fontStyleValue), krys::move(obliqueValues)));
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const FontStyleRangeValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      if (func(*FontStyleValue) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      if (RefPtr obliqueValues = ObliqueValues)
      {
        if (func(*obliqueValues) == IterationStatus::Done)
        {
          return IterationStatus::Done;
        }
      }
      return IterationStatus::Continue;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FontStyleRangeValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsFontStyleRangeValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
