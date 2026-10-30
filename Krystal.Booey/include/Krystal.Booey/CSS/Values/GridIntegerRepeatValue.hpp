#pragma once

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief Stores the track sizes and line numbers when the integer-repeat syntax is used.
  /// @note Right now the integer-repeat syntax is as follows:
  /// `<track-repeat> = repeat( [ <positive-integer> ], [ <line-names>? <track-size> ]+ <line-names>? )`
  /// `<fixed-repeat> = repeat( [ <positive-integer> ], [ <line-names>? <fixed-size> ]+ <line-names>? )`
  class GridIntegerRepeatValue final : public ValueContainingList
  {
  private:
    const Ref<PrimitiveValue> _repetitions;

    GridIntegerRepeatValue(Ref<PrimitiveValue> repetitions, ValueListBuilder builder) noexcept;

  public:
    KRYS_NODISCARD static Ref<GridIntegerRepeatValue> Create(Ref<PrimitiveValue> repetitions,
                                                             ValueListBuilder builder) noexcept;

    KRYS_NODISCARD const PrimitiveValue &Repetitions() const noexcept
    {
      return *_repetitions;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const GridIntegerRepeatValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      if (ValueContainingList::CustomVisitChildren(func) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      if (func(*_repetitions) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      return IterationStatus::Continue;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::GridIntegerRepeatValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsGridIntegerRepeatValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
