#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/PrimitiveKeywordList.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperData.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Utils/Tags.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::style
{
  template <typename, css::PrimitiveKeyword...>
  struct LengthWrapperBase;

  template <typename, typename>
  struct MinimumEvaluation;

  template <typename>
  struct LengthWrapperBlendingSupport;

  // Transitionary type acting as a `style::PrimitiveNumericOrKeyword<...>` but implemented by wrapping a
  // `LengthWrapperData`.
  template <typename Numeric, css::PrimitiveKeyword... Ks>
  struct LengthWrapperBase
  {
    template <typename>
    friend struct ToPlatform;

    template <typename, typename>
    friend struct Evaluation;

    template <typename, typename>
    friend struct MinimumEvaluation;

    template <typename>
    friend struct Blending;

    template <typename>
    friend struct LengthWrapperBlendingSupport;

    using Base = LengthWrapperBase<Numeric, Ks...>;
    using Keywords = css::PrimitiveKeywordList<Ks...>;

    constexpr static bool HasKeywords = Keywords::Count > 0;

    constexpr static uint8 IndexForFirstKeyword = 0;
    constexpr static uint8 IndexForLastKeyword = HasKeywords ? Keywords::Count - 1 : 0;
    constexpr static uint8 IndexForFixed = HasKeywords ? IndexForLastKeyword + 1 : 0;
    constexpr static uint8 IndexForPercentage = IndexForFixed + 1;
    constexpr static uint8 IndexForCalc = IndexForFixed + 2;
    constexpr static uint8 MaxIndex = IndexForCalc;

    using Specified = Numeric;
    using Fixed = typename Specified::Dimension;
    using Percentage = typename Specified::Percentage;
    using Calc = typename Specified::Calc;

    constexpr static bool SupportsAuto = Keywords::IsValidKeyword(css::keywords::Auto {});
    constexpr static bool SupportsNormal = Keywords::IsValidKeyword(css::keywords::Normal {});
    constexpr static bool SupportsIntrinsic = Keywords::IsValidKeyword(css::keywords::Intrinsic {});
    constexpr static bool SupportsMinIntrinsic = Keywords::IsValidKeyword(css::keywords::MinIntrinsic {});
    constexpr static bool SupportsMinContent = Keywords::IsValidKeyword(css::keywords::MinContent {});
    constexpr static bool SupportsMaxContent = Keywords::IsValidKeyword(css::keywords::MaxContent {});
    constexpr static bool SupportsWebkitFillAvailable =
      Keywords::IsValidKeyword(css::keywords::WebkitFillAvailable {});
    constexpr static bool SupportsFitContent = Keywords::IsValidKeyword(css::keywords::FitContent {});
    constexpr static bool SupportsContent = Keywords::IsValidKeyword(css::keywords::Content {});
    constexpr static bool SupportsNone = Keywords::IsValidKeyword(css::keywords::None {});

  private:
    LengthWrapperData _value;

  public:
    LengthWrapperBase(css::ValidKeywordForList<Keywords> auto keyword) noexcept
        : _value(static_cast<uint8>(Keywords::OffsetForKeyword(keyword)))
    {
    }

    LengthWrapperBase(Fixed fixed) noexcept : _value(IndexForFixed, fixed.UnresolvedValue())
    {
    }

    LengthWrapperBase(Fixed fixed, bool hasQuirk) noexcept
        : _value(IndexForFixed, fixed.UnresolvedValue(), hasQuirk)
    {
    }

    LengthWrapperBase(Percentage percent) noexcept : _value(IndexForPercentage, percent.Value)
    {
    }

    LengthWrapperBase(Calc &&calc) noexcept : _value(IndexForCalc, calc.ProtectedCalculation())
    {
    }

    LengthWrapperBase(Specified &&specified) noexcept : _value(ToData(specified))
    {
    }

    LengthWrapperBase(const Specified &specified) noexcept : _value(ToData(specified))
    {
    }

    LengthWrapperBase(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept
        : LengthWrapperBase(Fixed {literal})
    {
    }

    LengthWrapperBase(css::ValueLiteral<css::PercentageUnit::Percentage> literal) noexcept
        : LengthWrapperBase(Percentage {literal})
    {
    }

    explicit LengthWrapperBase(HashTableEmptyValueTag token) noexcept : _value(token)
    {
    }

    KRYS_NODISCARD KRYS_ALWAYS_INLINE bool IsFixed() const noexcept
    {
      return HoldsAlternative<Fixed>();
    }

    KRYS_NODISCARD KRYS_ALWAYS_INLINE bool IsPercent() const noexcept
    {
      return HoldsAlternative<Percentage>();
    }

    KRYS_NODISCARD KRYS_ALWAYS_INLINE bool IsCalculated() const noexcept
    {
      return HoldsAlternative<Calc>();
    }

    KRYS_NODISCARD KRYS_ALWAYS_INLINE bool IsPercentOrCalculated() const noexcept
    {
      return IsPercent() || IsCalculated();
    }

    KRYS_NODISCARD KRYS_ALWAYS_INLINE bool IsSpecified() const noexcept
    {
      return IsFixed() || IsPercent() || IsCalculated();
    }

    // `IsKnownZero` returns whether the value can be guaranteed to be `0`. Keywords and calc() return
    // `false`.
    KRYS_NODISCARD KRYS_ALWAYS_INLINE bool IsKnownZero() const noexcept
    requires(Fixed::Range.Min <= 0 && Fixed::Range.Max >= 0)
    {
      return _value.IsKnownZero(EvaluationKind());
    }

    // `IsKnownPositive` returns whether the value can be guaranteed to be more than `0`. Keywords and calc()
    // return `false`.
    KRYS_NODISCARD KRYS_ALWAYS_INLINE bool IsKnownPositive() const noexcept
    requires(Fixed::Range.Max > 0)
    {
      return _value.IsKnownPositive(EvaluationKind());
    }

    // `IsKnownNegative` returns whether the value can be guaranteed to be less than `0`. Keywords and calc()
    // return `false`.
    KRYS_NODISCARD KRYS_ALWAYS_INLINE bool IsKnownNegative() const noexcept
    requires(Fixed::Range.Min < 0)
    {
      return _value.IsKnownNegative(EvaluationKind());
    }

    // `IsPossiblyZero` returns whether the value can possibly be `0`. Keywords and calc() return `true`.
    KRYS_NODISCARD KRYS_ALWAYS_INLINE bool IsPossiblyZero() const noexcept
    requires(Fixed::Range.Min <= 0 && Fixed::Range.Max >= 0)
    {
      return _value.IsPossiblyZero(EvaluationKind());
    }

    // `IsPossiblyPositive` returns whether the value can possibly be more than `0`. Keywords and calc()
    // return `true`.
    KRYS_NODISCARD KRYS_ALWAYS_INLINE bool IsPossiblyPositive() const noexcept
    requires(Fixed::Range.Max > 0)
    {
      return _value.IsPossiblyPositive(EvaluationKind());
    }

    // `IsPossiblyNegative` returns whether the value can possibly be less than `0`. Keywords and calc()
    // return `true`.
    KRYS_NODISCARD KRYS_ALWAYS_INLINE bool IsPossiblyNegative() const noexcept
    requires(Fixed::Range.Min < 0)
    {
      return _value.IsPossiblyNegative(EvaluationKind());
    }

    KRYS_NODISCARD Maybe<Fixed> TryFixed() const noexcept
    {
      return IsFixed() ? Maybe<Fixed>(Fixed {_value.Value()}) : null;
    }

    KRYS_NODISCARD Maybe<Percentage> TryPercentage() const noexcept
    {
      return IsPercent() ? Maybe<Percentage>(Percentage {_value.Value()}) : null;
    }

    KRYS_NODISCARD Maybe<Calc> TryCalc() const noexcept
    {
      return IsCalculated() ? Maybe<Calc>(Calc {_value.CalculationValue()}) : null;
    }

    KRYS_NODISCARD Maybe<Specified> TrySpecified() const noexcept
    {
      auto opaqueType = _value.Type();

      if (opaqueType == IndexForFixed)
      {
        return Specified(Fixed {_value.Value()});
      }

      if (opaqueType == IndexForPercentage)
      {
        return Specified(Percentage {_value.Value()});
      }

      if (opaqueType == IndexForCalc)
      {
        return Specified(Calc {_value.CalculationValue()});
      }

      return null;
    }

    template <typename T>
    KRYS_NODISCARD bool HoldsAlternative() const noexcept
    {
      if constexpr (css::ValidKeywordForList<T, Keywords>)
      {
        return _value.Type() == Keywords::OffsetForKeyword(T {});
      }
      else if constexpr (SameType<T, Fixed>)
      {
        return _value.Type() == IndexForFixed;
      }
      else if constexpr (SameType<T, Percentage>)
      {
        return _value.Type() == IndexForPercentage;
      }
      else if constexpr (SameType<T, Calc>)
      {
        return _value.Type() == IndexForCalc;
      }
      else if constexpr (SameType<T, Specified>)
      {
        return _value.Type() == IndexForFixed || _value.Type() == IndexForPercentage
               || _value.Type() == IndexForCalc;
      }
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      auto opaqueType = _value.Type();

      if constexpr (HasKeywords)
      {
        if (opaqueType <= IndexForLastKeyword)
        {
          return Keywords::VisitKeywordAtOffset(opaqueType, visitor);
        }
      }

      if (opaqueType == IndexForFixed)
      {
        return visitor(Fixed {_value.Value()});
      }

      if (opaqueType == IndexForPercentage)
      {
        return visitor(Percentage {_value.Value()});
      }

      if (opaqueType == IndexForCalc)
      {
        return visitor(Calc {_value.CalculationValue()});
      }

      krys_unreachable();
    }

    KRYS_NODISCARD bool HasQuirk() const noexcept
    {
      return _value.HasQuirk();
    }

    KRYS_NODISCARD bool HasSameType(const LengthWrapperBase &other) const noexcept
    {
      return _value.Type() == other._value.Type();
    }

    KRYS_NODISCARD bool operator==(const LengthWrapperBase &) const noexcept = default;

  private:
    KRYS_NODISCARD static LengthWrapperData ToData(const Specified &specified) noexcept
    {
      return krys::SwitchOn(
        specified,
        [](const Fixed &fixed) { return LengthWrapperData {IndexForFixed, fixed.UnresolvedValue()}; },
        [](const Percentage &percentage) { return LengthWrapperData {IndexForPercentage, percentage.Value}; },
        [](const Calc &calc) { return LengthWrapperData {IndexForCalc, calc.ProtectedCalculation()}; });
    }

    KRYS_NODISCARD LengthWrapperDataEvaluationKind EvaluationKind() const noexcept
    {
      auto opaqueType = _value.Type();

      if constexpr (HasKeywords)
      {
        if (opaqueType <= IndexForLastKeyword)
        {
          return LengthWrapperDataEvaluationKind::Flag;
        }
      }

      if (opaqueType == IndexForFixed)
      {
        return LengthWrapperDataEvaluationKind::Fixed;
      }

      if (opaqueType == IndexForPercentage)
      {
        return LengthWrapperDataEvaluationKind::Percentage;
      }

      if (opaqueType == IndexForCalc)
      {
        return LengthWrapperDataEvaluationKind::Calculation;
      }

      krys_unreachable();
    }
  };

  // MARK: - Concepts

  namespace detail_length_wrapper
  {
    template <typename N, css::PrimitiveKeyword... Ks>
    std::true_type IsLengthWrapperBase(const LengthWrapperBase<N, Ks...> *);
    std::false_type IsLengthWrapperBase(const void *);
  }

  template <typename T>
  concept LengthWrapperBaseDerived =
    decltype(detail_length_wrapper::IsLengthWrapperBase(std::declval<const T *>()))::value
    && css::VariantLike<T>;

  // MARK: - Evaluation

  template <LengthWrapperBaseDerived T, typename Result>
  struct Evaluation<T, Result>
  {
    template <typename F>
    requires InvocableReturns<Result, const F &>
    KRYS_NODISCARD Result operator()(const T &value, const F &lazyMaximumValueFunctor,
                                     ZoomNeeded token) noexcept
    requires(T::Fixed::Range.ZoomOptions == css::RangeZoomOptions::Default)
    {
      return value._value.template ValueForLengthWrapperDataWithLazyMaximum<Result, Result>(
        value.EvaluationKind(), lazyMaximumValueFunctor, token);
    }

    KRYS_NODISCARD Result operator()(const T &value, Result maximumValue, ZoomNeeded token) noexcept
    requires(T::Fixed::Range.ZoomOptions == css::RangeZoomOptions::Default)
    {
      return value._value.template ValueForLengthWrapperDataWithLazyMaximum<Result, Result>(
        value.EvaluationKind(), [&] { return maximumValue; }, token);
    }

    template <typename F>
    requires InvocableReturns<Result, const F &>
    KRYS_NODISCARD Result operator()(const T &value, const F &lazyMaximumValueFunctor,
                                     ZoomFactor zoom) noexcept
    requires(T::Fixed::Range.ZoomOptions == css::RangeZoomOptions::Unzoomed)
    {
      return value._value.template ValueForLengthWrapperDataWithLazyMaximum<Result, Result>(
        value.EvaluationKind(), lazyMaximumValueFunctor, zoom);
    }

    KRYS_NODISCARD Result operator()(const T &value, Result maximumValue, ZoomFactor zoom) noexcept
    requires(T::Fixed::Range.ZoomOptions == css::RangeZoomOptions::Unzoomed)
    {
      return value._value.template ValueForLengthWrapperDataWithLazyMaximum<Result, Result>(
        value.EvaluationKind(), [&] { return maximumValue; }, zoom);
    }
  };

  template <typename Result>
  struct EvaluationMinimumInvoker
  {
    template <typename StyleType, typename Reference, typename Zoom>
    KRYS_NODISCARD decltype(auto) operator()(const StyleType &value, Reference &&reference,
                                             Zoom &&zoom) const noexcept
    {
      return MinimumEvaluation<StyleType, Result> {}(value, std::forward<Reference>(reference),
                                                     std::forward<Zoom>(zoom));
    }
  };

  template <typename Result>
  constexpr EvaluationMinimumInvoker<Result> EvaluateMinimum {};

  template <LengthWrapperBaseDerived T, typename Result>
  struct MinimumEvaluation<T, Result>
  {
    template <typename F>
    requires InvocableReturns<Result, const F &>
    KRYS_NODISCARD Result operator()(const T &value, const F &lazyMaximumValueFunctor,
                                     ZoomNeeded token) noexcept
    requires(T::Fixed::Range.ZoomOptions == css::RangeZoomOptions::Default)
    {
      return value._value.template MinimumValueForLengthWrapperDataWithLazyMaximum<LayoutUnit, LayoutUnit>(
        value.EvaluationKind(), lazyMaximumValueFunctor, token);
    }

    KRYS_NODISCARD Result operator()(const T &value, Result maximumValue, ZoomNeeded token) noexcept
    requires(T::Fixed::Range.ZoomOptions == css::RangeZoomOptions::Default)
    {
      return value._value.template MinimumValueForLengthWrapperDataWithLazyMaximum<LayoutUnit, LayoutUnit>(
        value.EvaluationKind(), [&] { return LayoutUnit(maximumValue); }, token);
    }

    template <typename F>
    requires InvocableReturns<Result, const F &>
    KRYS_NODISCARD Result operator()(const T &value, const F &lazyMaximumValueFunctor,
                                     ZoomFactor zoom) noexcept
    requires(T::Fixed::Range.ZoomOptions == css::RangeZoomOptions::Unzoomed)
    {
      return value._value.template MinimumValueForLengthWrapperDataWithLazyMaximum<LayoutUnit, LayoutUnit>(
        value.EvaluationKind(), lazyMaximumValueFunctor, zoom);
    }

    KRYS_NODISCARD Result operator()(const T &value, Result maximumValue, ZoomFactor zoom) noexcept
    requires(T::Fixed::Range.ZoomOptions == css::RangeZoomOptions::Unzoomed)
    {
      return value._value.template MinimumValueForLengthWrapperDataWithLazyMaximum<LayoutUnit, LayoutUnit>(
        value.EvaluationKind(), [&] { return LayoutUnit(maximumValue); }, zoom);
    }
  };
}
