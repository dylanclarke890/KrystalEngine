#include "Krystal.Booey/CSS/Values/Primitives/UnevaluatedCalc.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"

namespace krys::boo::css
{
  void UnevaluatedCalcRef(calc::Value *calc) noexcept
  {
    calc->AddRef();
  }

  void UnevaluatedCalcDeref(calc::Value *calc) noexcept
  {
    calc->SubRef();
  }

  UnevaluatedCalcBase::UnevaluatedCalcBase(calc::Value &value) noexcept : _calc {ShareRef(value)}
  {
  }

  UnevaluatedCalcBase::UnevaluatedCalcBase(Ref<calc::Value> &&value) noexcept : _calc {krys::move(value)}
  {
  }

  UnevaluatedCalcBase::UnevaluatedCalcBase(const UnevaluatedCalcBase &) noexcept = default;
  UnevaluatedCalcBase::UnevaluatedCalcBase(UnevaluatedCalcBase &&) noexcept = default;
  UnevaluatedCalcBase &UnevaluatedCalcBase::operator=(const UnevaluatedCalcBase &) noexcept = default;
  UnevaluatedCalcBase &UnevaluatedCalcBase::operator=(UnevaluatedCalcBase &&) noexcept = default;

  UnevaluatedCalcBase::~UnevaluatedCalcBase() noexcept = default;

  Ref<calc::Value> UnevaluatedCalcBase::ProtectedCalc() const noexcept
  {
    return _calc;
  }

  calc::Value &UnevaluatedCalcBase::LeakRef() noexcept
  {
    return *_calc.release();
  }

  bool UnevaluatedCalcBase::Equal(const UnevaluatedCalcBase &other) const noexcept
  {
    return ProtectedCalc()->Equals(*other._calc);
  }

  bool UnevaluatedCalcBase::RequiresConversionData() const noexcept
  {
    return ProtectedCalc()->RequiresConversionData();
  }

  void UnevaluatedCalcBase::SerialisationForCSS(CSSOMString &builder,
                                                const SerialisationContext &context) const noexcept
  {
    builder.append(ProtectedCalc()->CssText(context));
  }

  void UnevaluatedCalcBase::CollectComputedStyleDependencies(
    ComputedStyleDependencies &dependencies) const noexcept
  {
    ProtectedCalc()->CollectComputedStyleDependencies(dependencies);
  }
}
