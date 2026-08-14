#pragma once

#include "Krystal.HTML/CSS/Calc/CSSCalcValue.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericConcepts.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys::HTML
{
  void UnevaluatedCalcAddRef(RawPtr<CSSCalcValue>) noexcept;

  void UnevaluatedCalcSubRef(RawPtr<CSSCalcValue>) noexcept;

  struct UnevaluatedCalcBase
  {
  private:
    Ref<CSSCalcValue> calc;

  public:
    ~UnevaluatedCalcBase();

    UnevaluatedCalcBase(CSSCalcValue &calc);
    UnevaluatedCalcBase(Ref<CSSCalcValue> &&calc);

    UnevaluatedCalcBase(const UnevaluatedCalcBase &);
    UnevaluatedCalcBase(UnevaluatedCalcBase &&);
    UnevaluatedCalcBase &operator=(const UnevaluatedCalcBase &);
    UnevaluatedCalcBase &operator=(UnevaluatedCalcBase &&);

    // Ref<CSSCalcValue> ProtectedCalc() const;
    // KRYS_NODISCARD CSSCalcValue &LeakRef();

    // bool RequiresConversionData() const;

    // void SerializationForCSS(StringBuilder &, const CSS::SerializationContext &) const;
    // void CollectComputedStyleDependencies(ComputedStyleDependencies &) const;

    // UnevaluatedCalcBase SimplifyBase(const CSSToLengthConversionData &, const CSSCalcSymbolTable &) const;

    // double evaluate(CSS::Category, const Style::BuilderState &) const;
    // double evaluate(CSS::Category, const Style::BuilderState &, const CSSCalcSymbolTable &) const;
    // double evaluate(CSS::Category, const CSSToLengthConversionData &) const;
    // double evaluate(CSS::Category, const CSSToLengthConversionData &, const CSSCalcSymbolTable &) const;
    // double evaluate(CSS::Category, NoConversionDataRequiredToken) const;
    // double evaluate(CSS::Category, NoConversionDataRequiredToken, const CSSCalcSymbolTable &) const;

    // bool equal(const UnevaluatedCalcBase &) const;
  };

  template <NumericRaw RawType>
  struct UnevaluatedCalc : UnevaluatedCalcBase
  {
    using Raw = RawType;
    using Calc = UnevaluatedCalc<Raw>;
    using UnitType = typename Raw::UnitType;
    using UnitTraits = typename Raw::UnitTraits;
    using ResolvedValueType = typename Raw::ResolvedValueType;

    constexpr static auto Range = Raw::Range;
    constexpr static auto Category = Raw::Category;
  };
}