#pragma once

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleImage;
}

namespace krys::boo::style
{
  class BuilderState;
}

namespace krys::boo::css
{
  /// @brief The value of a `cross-fade()` or `-webkit-cross-fade()` function.
  class CrossFadeValue final : public Value
  {
  private:
    const Ref<Value> _fromValueOrNone;
    const Ref<Value> _toValueOrNone;
    const Ref<PrimitiveValue> _percentageValue;
    bool _isPrefixed;

    CrossFadeValue(Ref<Value> &&fromValueOrNone, Ref<Value> &&toValueOrNone,
                   Ref<PrimitiveValue> &&percentageValue, bool isPrefixed) noexcept;

  public:
    KRYS_NODISCARD static Ref<CrossFadeValue> Create(Ref<Value> &&fromValueOrNone, Ref<Value> &&toValueOrNone,
                                                     Ref<PrimitiveValue> &&percentageValue,
                                                     bool isPrefixed = false) noexcept;

    KRYS_NODISCARD bool Equals(const CrossFadeValue &other) const noexcept;
    KRYS_NODISCARD bool EqualInputImages(const CrossFadeValue &other) const noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool IsPrefixed() const noexcept
    {
      return _isPrefixed;
    }

    // FIXME: not defined until style::BuilderState and StyleImage are ported.
    KRYS_NODISCARD RefPtr<StyleImage> CreateStyleImage(const style::BuilderState &state) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      if (func(*_fromValueOrNone) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      if (func(*_toValueOrNone) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      if (func(*_percentageValue) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      return IterationStatus::Continue;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CrossFadeValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsCrossFadeValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
