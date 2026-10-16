#pragma once

#include "Krystal.Booey/Core/IterationStatus.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Concepts.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::style
{
  class BuilderState;
}

namespace krys::boo::css::calc
{
  class Value;
}

namespace krys::boo::css
{
  class SymbolTable;
  class ToLengthConversionData;
  struct ComputedStyleDependencies;
  struct NoConversionDataRequiredTag;

  enum class Category : uint8;

  // Type-erased helpers to allow for shared code.

  void UnevaluatedCalcRef(calc::Value *) noexcept;
  void UnevaluatedCalcDeref(calc::Value *) noexcept;

  // `UnevaluatedCalc` annotates a `calc::Value` with the raw value type that it
  // will be evaluated to, allowing the processing of calc in generic code.

  // Non-generic base type to allow code sharing and out-of-line definitions.
  struct UnevaluatedCalcBase
  {
  private:
    Ref<calc::Value> _calc;

  public:
    UnevaluatedCalcBase(calc::Value &) noexcept;
    UnevaluatedCalcBase(Ref<calc::Value> &&) noexcept;

    UnevaluatedCalcBase(const UnevaluatedCalcBase &) noexcept;
    UnevaluatedCalcBase(UnevaluatedCalcBase &&) noexcept;
    UnevaluatedCalcBase &operator=(const UnevaluatedCalcBase &) noexcept;
    UnevaluatedCalcBase &operator=(UnevaluatedCalcBase &&) noexcept;
    ~UnevaluatedCalcBase() noexcept;

    KRYS_NODISCARD Ref<calc::Value> ProtectedCalc() const noexcept;
    KRYS_NODISCARD calc::Value &LeakRef() noexcept;

    KRYS_NODISCARD bool RequiresConversionData() const noexcept;

    void SerialisationForCSS(StringBuilder &, const SerialisationContext &) const noexcept;
    void CollectComputedStyleDependencies(ComputedStyleDependencies &) const noexcept;

    KRYS_NODISCARD UnevaluatedCalcBase SimplifyBase(const ToLengthConversionData &,
                                                    const SymbolTable &) const noexcept;

    KRYS_NODISCARD double Evaluate(Category, const style::BuilderState &) const noexcept;
    KRYS_NODISCARD double Evaluate(Category, const style::BuilderState &, const SymbolTable &) const noexcept;
    KRYS_NODISCARD double Evaluate(Category, const ToLengthConversionData &) const noexcept;
    KRYS_NODISCARD double Evaluate(Category, const ToLengthConversionData &,
                                   const SymbolTable &) const noexcept;
    KRYS_NODISCARD double Evaluate(Category, NoConversionDataRequiredTag) const noexcept;
    KRYS_NODISCARD double Evaluate(Category, NoConversionDataRequiredTag, const SymbolTable &) const noexcept;

    KRYS_NODISCARD bool Equal(const UnevaluatedCalcBase &) const noexcept;
  };

  template <NumericRaw RawType>
  struct UnevaluatedCalc : UnevaluatedCalcBase
  {
    using UnevaluatedCalcBase::UnevaluatedCalcBase;
    using UnevaluatedCalcBase::operator=;

    using Raw = RawType;
    using UnitType = typename Raw::UnitType;
    using UnitTraits = typename Raw::UnitTraits;
    using ResolvedValueType = typename Raw::ResolvedValueType;
    constexpr static auto Range = Raw::Range;
    constexpr static auto Category = Raw::Category;

    KRYS_NODISCARD bool operator==(const UnevaluatedCalc &other) const noexcept
    {
      return UnevaluatedCalcBase::Equal(static_cast<const UnevaluatedCalcBase &>(other));
    }

    KRYS_NODISCARD UnevaluatedCalc Simplify(const ToLengthConversionData &conversionData,
                                            const SymbolTable &symbolTable) const noexcept
    {
      return static_cast<UnevaluatedCalc>(SimplifyBase(conversionData, symbolTable));
    }
  };

  // MARK: - Requires Conversion Data

  template <Calc T>
  KRYS_NODISCARD bool RequiresConversionData(const T &unevaluatedCalc) noexcept
  {
    return unevaluatedCalc.RequiresConversionData();
  }

  template <typename T>
  KRYS_NODISCARD bool RequiresConversionData(const T &) noexcept
  {
    static_assert(!Calc<T>);
    return false;
  }

  template <typename... Ts>
  KRYS_NODISCARD bool RequiresConversionData(const Variant<Ts...> &component) noexcept
  {
    return krys::SwitchOn(component,
                          [&](auto alternative) -> bool { return RequiresConversionData(alternative); });
  }

  template <typename T>
  KRYS_NODISCARD bool RequiresConversionData(const Maybe<T> &component) noexcept
  {
    return component && RequiresConversionData(*component);
  }

  // MARK: - Is UnevaluatedCalc

  template <typename T>
  KRYS_NODISCARD constexpr bool IsUnevaluatedCalc(const T &) noexcept
  {
    return Calc<T>;
  }

  template <typename... Ts>
  KRYS_NODISCARD constexpr bool IsUnevaluatedCalc(const Variant<Ts...> &component) noexcept
  {
    return krys::SwitchOn(component,
                          [&](auto alternative) -> bool { return IsUnevaluatedCalc(alternative); });
  }

  template <typename T>
  KRYS_NODISCARD constexpr bool IsUnevaluatedCalc(const Maybe<T> &component) noexcept
  {
    return component && IsUnevaluatedCalc(*component);
  }

  // MARK: Simplify

  template <Calc T>
  KRYS_NODISCARD T SimplifyUnevaluatedCalc(const T &unevaluatedCalc,
                                           const ToLengthConversionData &conversionData,
                                           const SymbolTable &symbolTable) noexcept
  {
    return unevaluatedCalc.Simplify(conversionData, symbolTable);
  }

  template <typename T>
  KRYS_NODISCARD T SimplifyUnevaluatedCalc(const T &component, const ToLengthConversionData &,
                                           const SymbolTable &) noexcept
  {
    return component;
  }

  template <typename... Ts>
  KRYS_NODISCARD Variant<Ts...> SimplifyUnevaluatedCalc(const Variant<Ts...> &component,
                                                        const ToLengthConversionData &conversionData,
                                                        const SymbolTable &symbolTable) noexcept
  {
    return krys::SwitchOn(component, [&](auto alternative) -> Variant<Ts...>
                          { return SimplifyUnevaluatedCalc(alternative, conversionData, symbolTable); });
  }

  template <typename T>
  KRYS_NODISCARD Maybe<T> SimplifyUnevaluatedCalc(const Maybe<T> &component,
                                                  const ToLengthConversionData &conversionData,
                                                  const SymbolTable &symbolTable) noexcept
  {
    if (!component)
    {
      return null;
    }

    return SimplifyUnevaluatedCalc(*component, conversionData, symbolTable);
  }

  // MARK: - Serialisation

  template <Calc T>
  struct Serialiser<T>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context, const T &value) noexcept
    {
      value.SerialisationForCSS(builder, context);
    }
  };

  // MARK: - Computed Style Dependencies

  template <Calc T>
  struct ComputedStyleDependenciesCollector<T>
  {
    void operator()(ComputedStyleDependencies &dependencies, const T &value) noexcept
    {
      value.CollectComputedStyleDependencies(dependencies);
    }
  };

  // MARK: - Value Visitation

  template <Calc T>
  struct ValueChildrenVisitor<T>
  {
    IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &, const T &) noexcept
    {
      return IterationStatus::Continue;
    }
  };
}
