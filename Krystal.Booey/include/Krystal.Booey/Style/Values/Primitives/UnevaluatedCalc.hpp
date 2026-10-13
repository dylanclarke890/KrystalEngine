#pragma once

#include "Krystal.Booey/Style/Values/Primitives/Concepts.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/IntrusivePtr.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style::calc
{
  class Value;
  struct Child;
}

namespace krys::boo::style
{
  // Non-generic base type to allow code sharing and out-of-line definitions.
  struct UnevaluatedCalcBase
  {
  private:
    Ref<calc::Value> _calc;

  public:
    explicit UnevaluatedCalcBase(calc::Value &) noexcept;
    explicit UnevaluatedCalcBase(Ref<calc::Value> &&) noexcept;
    explicit UnevaluatedCalcBase(calc::Child &&, css::Category, css::Range) noexcept;

    UnevaluatedCalcBase(const UnevaluatedCalcBase &) noexcept;
    UnevaluatedCalcBase(UnevaluatedCalcBase &&) noexcept;
    UnevaluatedCalcBase &operator=(const UnevaluatedCalcBase &) noexcept;
    UnevaluatedCalcBase &operator=(UnevaluatedCalcBase &&) noexcept;

    ~UnevaluatedCalcBase() noexcept;

    KRYS_NODISCARD Ref<calc::Value> ProtectedCalculation() const noexcept;

    KRYS_NODISCARD bool Equal(const UnevaluatedCalcBase &) const noexcept;
  };

  // Wrapper for `Ref<calc::Value>` that includes range and category as part of the type.
  template <css::Numeric CSSType>
  struct UnevaluatedCalc : UnevaluatedCalcBase
  {
    using UnevaluatedCalcBase::UnevaluatedCalcBase;
    using UnevaluatedCalcBase::operator=;

    using CSS = CSSType;
    constexpr static auto Range = CSS::Range;
    constexpr static auto Category = CSS::Category;

    explicit UnevaluatedCalc(calc::Value &calculationValue) noexcept : UnevaluatedCalcBase(calculationValue)
    {
    }

    explicit UnevaluatedCalc(calc::Child &&child) noexcept
        : UnevaluatedCalcBase(krys::move(child), Category, Range)
    {
    }

    KRYS_NODISCARD bool operator==(const UnevaluatedCalc &other) const noexcept
    {
      return UnevaluatedCalcBase::Equal(static_cast<const UnevaluatedCalcBase &>(other));
    }
  };
}

namespace krys
{
  template <boo::style::Calc T>
  constexpr bool IsSmartPtr<T> = true;
}
