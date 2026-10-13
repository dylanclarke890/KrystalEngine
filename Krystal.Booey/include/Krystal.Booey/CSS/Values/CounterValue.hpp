#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `counter()` or `counters()` value, e.g. `counters(item, ".", upper-roman)`.
  class CounterValue final : public Value
  {
  private:
    CSSOMStringAtom _identifier;
    CSSOMStringAtom _separator;
    Ref<Value> _counterStyle;

    CounterValue(CSSOMStringAtom &&identifier, CSSOMStringAtom &&separator,
                 Ref<Value> &&counterStyle) noexcept;

  public:
    /// @param separator The `counters()` separator; null or empty for `counter()`.
    KRYS_NODISCARD static Ref<CounterValue> Create(CSSOMStringAtom identifier, CSSOMStringAtom separator,
                                                   Ref<Value> counterStyle) noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &Identifier() const noexcept
    {
      return _identifier;
    }

    KRYS_NODISCARD const CSSOMStringAtom &Separator() const noexcept
    {
      return _separator;
    }

    KRYS_NODISCARD Ref<Value> CounterStyle() const noexcept
    {
      return _counterStyle;
    }

    KRYS_NODISCARD CSSOMString CounterStyleCSSText() const noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const CounterValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      if (func(*_counterStyle) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      return IterationStatus::Continue;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CounterValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsCounterValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
