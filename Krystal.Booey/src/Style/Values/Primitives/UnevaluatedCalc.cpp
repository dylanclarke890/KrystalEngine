#include "Krystal.Booey/Style/Values/Primitives/UnevaluatedCalc.hpp"
#include "Krystal.Booey/Style/Calc/Value.hpp"

namespace krys::boo::style
{
  UnevaluatedCalcBase::UnevaluatedCalcBase(calc::Value &value) noexcept : _calc {ShareRef(value)}
  {
  }

  UnevaluatedCalcBase::UnevaluatedCalcBase(Ref<calc::Value> &&value) noexcept : _calc {krys::move(value)}
  {
  }

  UnevaluatedCalcBase::UnevaluatedCalcBase(calc::Child &&root, css::Category category,
                                           css::Range range) noexcept
      : _calc {
          calc::Value::Create(category, css::Range {range.Min, range.Max}, calc::Tree {krys::move(root)})}
  {
  }

  UnevaluatedCalcBase::UnevaluatedCalcBase(const UnevaluatedCalcBase &) noexcept = default;
  UnevaluatedCalcBase::UnevaluatedCalcBase(UnevaluatedCalcBase &&) noexcept = default;
  UnevaluatedCalcBase &UnevaluatedCalcBase::operator=(const UnevaluatedCalcBase &) noexcept = default;
  UnevaluatedCalcBase &UnevaluatedCalcBase::operator=(UnevaluatedCalcBase &&) noexcept = default;

  UnevaluatedCalcBase::~UnevaluatedCalcBase() noexcept = default;

  Ref<calc::Value> UnevaluatedCalcBase::ProtectedCalculation() const noexcept
  {
    return _calc;
  }

  bool UnevaluatedCalcBase::Equal(const UnevaluatedCalcBase &other) const noexcept
  {
    return *_calc == *other._calc;
  }
}
