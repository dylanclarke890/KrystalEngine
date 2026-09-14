#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Concepts.hpp"

namespace krys::boo::css::calc
{
  class Value;
}

namespace krys::boo::css
{
  void UnevaluatedCalcAddRef(calc::Value *) noexcept;

  void UnevaluatedCalcSubRef(calc::Value *) noexcept;

  struct UnevaluatedCalcBase
  {
  private:
    Ref<calc::Value> calc;

  public:
    ~UnevaluatedCalcBase();

    UnevaluatedCalcBase(calc::Value &calc);
    UnevaluatedCalcBase(Ref<calc::Value> &&calc);

    UnevaluatedCalcBase(const UnevaluatedCalcBase &);
    UnevaluatedCalcBase(UnevaluatedCalcBase &&);
    UnevaluatedCalcBase &operator=(const UnevaluatedCalcBase &);
    UnevaluatedCalcBase &operator=(UnevaluatedCalcBase &&);

     KRYS_NODISCARD calc::Value* get() const noexcept;

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