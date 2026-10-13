#pragma once

#include "Krystal.Booey/Style/Calc/Value.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Utils/Tags.hpp"
#include <utility>

namespace krys::boo::style
{
  enum class LengthWrapperDataKind : uint8
  {
    Default,
    Calculation,
    Empty,
    HashTableEmpty,
    HashTableDeleted
  };

  enum class LengthWrapperDataEvaluationKind : uint8
  {
    Fixed,
    Percentage,
    Calculation,
    Flag
  };

  /// @brief A length, percentage, flag or calculation tagged with an opaque type, stored in 8 bytes.
  /// Calculations are stored as a handle into `calc::ValueMap`.
  struct LengthWrapperData
  {
  private:
    union
    {
      float _floatValue {0.0f};
      uint32 _calculationValueHandle;
    };
    uint8 _opaqueType {0};
    LengthWrapperDataKind _kind;
    bool _hasQuirk {false};

  public:
    LengthWrapperData(uint8 opaqueType) noexcept;
    LengthWrapperData(uint8 opaqueType, float value, bool hasQuirk = false) noexcept;
    explicit LengthWrapperData(uint8 opaqueType, Ref<calc::Value> &&) noexcept;

    explicit LengthWrapperData(HashTableEmptyValueTag) noexcept;
    explicit LengthWrapperData(HashTableDeletedValueTag) noexcept;

    LengthWrapperData(const LengthWrapperData &) noexcept;
    LengthWrapperData(LengthWrapperData &&) noexcept;
    LengthWrapperData &operator=(const LengthWrapperData &) noexcept;
    LengthWrapperData &operator=(LengthWrapperData &&) noexcept;

    ~LengthWrapperData() noexcept;

    KRYS_NODISCARD bool operator==(const LengthWrapperData &) const noexcept;

    KRYS_NODISCARD uint8 Type() const noexcept
    {
      return _opaqueType;
    }

    KRYS_NODISCARD bool HasQuirk() const noexcept
    {
      return _hasQuirk;
    }

    KRYS_NODISCARD float Value() const noexcept
    {
      krys_debug_assert(_kind != LengthWrapperDataKind::Calculation);
      return _floatValue;
    }

    KRYS_NODISCARD calc::Value &CalculationValue() const noexcept;
    KRYS_NODISCARD Ref<calc::Value> ProtectedCalculationValue() const noexcept;

    KRYS_NODISCARD bool IsKnownZero(LengthWrapperDataEvaluationKind) const noexcept;
    KRYS_NODISCARD bool IsKnownPositive(LengthWrapperDataEvaluationKind) const noexcept;
    KRYS_NODISCARD bool IsKnownNegative(LengthWrapperDataEvaluationKind) const noexcept;

    KRYS_NODISCARD bool IsPossiblyZero(LengthWrapperDataEvaluationKind) const noexcept;
    KRYS_NODISCARD bool IsPossiblyPositive(LengthWrapperDataEvaluationKind) const noexcept;
    KRYS_NODISCARD bool IsPossiblyNegative(LengthWrapperDataEvaluationKind) const noexcept;

    template <typename ReturnType, typename MaximumType, typename F>
    requires InvocableReturns<MaximumType, const F &>
    KRYS_NODISCARD ReturnType MinimumValueForLengthWrapperDataWithLazyMaximum(
      LengthWrapperDataEvaluationKind, const F &lazyMaximumValueFunctor, ZoomNeeded) const noexcept;

    template <typename ReturnType, typename MaximumType, typename F>
    requires InvocableReturns<MaximumType, const F &>
    KRYS_NODISCARD ReturnType MinimumValueForLengthWrapperDataWithLazyMaximum(
      LengthWrapperDataEvaluationKind, const F &lazyMaximumValueFunctor, ZoomFactor) const noexcept;

    template <typename ReturnType, typename MaximumType, typename F>
    requires InvocableReturns<MaximumType, const F &>
    KRYS_NODISCARD ReturnType ValueForLengthWrapperDataWithLazyMaximum(LengthWrapperDataEvaluationKind,
                                                                       const F &lazyMaximumValueFunctor,
                                                                       ZoomNeeded) const noexcept;

    template <typename ReturnType, typename MaximumType, typename F>
    requires InvocableReturns<MaximumType, const F &>
    KRYS_NODISCARD ReturnType ValueForLengthWrapperDataWithLazyMaximum(LengthWrapperDataEvaluationKind,
                                                                       const F &lazyMaximumValueFunctor,
                                                                       ZoomFactor) const noexcept;

  private:
    KRYS_NODISCARD float NonNanCalculatedValue(float maxValue, const ZoomFactor &usedZoom) const noexcept;
    KRYS_NODISCARD float NonNanCalculatedValue(float maxValue, const ZoomNeeded &) const noexcept;
    KRYS_NODISCARD bool IsCalculatedEqual(const LengthWrapperData &) const noexcept;

    void Initialise(const LengthWrapperData &) noexcept;
    void Initialise(LengthWrapperData &&) noexcept;

    void AddRef() const noexcept;
    void SubRef() const noexcept;
  };

  inline LengthWrapperData::LengthWrapperData(uint8 opaqueType) noexcept
      : _floatValue {0.0f}, _opaqueType {opaqueType}, _kind {LengthWrapperDataKind::Default},
        _hasQuirk {false}
  {
  }

  inline LengthWrapperData::LengthWrapperData(uint8 opaqueType, float value, bool hasQuirk) noexcept
      : _floatValue {value}, _opaqueType {opaqueType}, _kind {LengthWrapperDataKind::Default},
        _hasQuirk {hasQuirk}
  {
  }

  inline LengthWrapperData::LengthWrapperData(HashTableEmptyValueTag) noexcept
      : _kind {LengthWrapperDataKind::HashTableEmpty}
  {
  }

  inline LengthWrapperData::LengthWrapperData(HashTableDeletedValueTag) noexcept
      : _kind {LengthWrapperDataKind::HashTableDeleted}
  {
  }

  inline LengthWrapperData::LengthWrapperData(const LengthWrapperData &other) noexcept
  {
    Initialise(other);
  }

  inline LengthWrapperData::LengthWrapperData(LengthWrapperData &&other) noexcept
  {
    Initialise(krys::move(other));
  }

  inline LengthWrapperData &LengthWrapperData::operator=(const LengthWrapperData &other) noexcept
  {
    if (this == &other)
    {
      return *this;
    }

    if (_kind == LengthWrapperDataKind::Calculation)
    {
      SubRef();
    }

    Initialise(other);
    return *this;
  }

  inline LengthWrapperData &LengthWrapperData::operator=(LengthWrapperData &&other) noexcept
  {
    if (this == &other)
    {
      return *this;
    }

    if (_kind == LengthWrapperDataKind::Calculation)
    {
      SubRef();
    }

    Initialise(krys::move(other));
    return *this;
  }

  inline void LengthWrapperData::Initialise(const LengthWrapperData &other) noexcept
  {
    _opaqueType = other._opaqueType;
    _hasQuirk = other._hasQuirk;
    _kind = other._kind;

    switch (_kind)
    {
      case LengthWrapperDataKind::Calculation:
        _calculationValueHandle = other._calculationValueHandle;
        AddRef();
        break;
      case LengthWrapperDataKind::Default:
      case LengthWrapperDataKind::Empty:
      case LengthWrapperDataKind::HashTableEmpty:
      case LengthWrapperDataKind::HashTableDeleted: _floatValue = other._floatValue; break;
    }
  }

  inline void LengthWrapperData::Initialise(LengthWrapperData &&other) noexcept
  {
    _opaqueType = other._opaqueType;
    _hasQuirk = other._hasQuirk;
    _kind = other._kind;

    switch (_kind)
    {
      case LengthWrapperDataKind::Calculation:
        _calculationValueHandle = std::exchange(other._calculationValueHandle, 0);
        break;
      case LengthWrapperDataKind::Default:
      case LengthWrapperDataKind::Empty:
      case LengthWrapperDataKind::HashTableEmpty:
      case LengthWrapperDataKind::HashTableDeleted: _floatValue = other._floatValue; break;
    }

    other._kind = LengthWrapperDataKind::Default;
  }

  inline LengthWrapperData::~LengthWrapperData() noexcept
  {
    if (_kind == LengthWrapperDataKind::Calculation)
    {
      SubRef();
    }
  }

  inline bool LengthWrapperData::operator==(const LengthWrapperData &other) const noexcept
  {
    if (Type() != other.Type() || HasQuirk() != other.HasQuirk())
    {
      return false;
    }

    if (_kind == LengthWrapperDataKind::Calculation)
    {
      return IsCalculatedEqual(other);
    }

    return Value() == other.Value();
  }

  inline bool LengthWrapperData::IsKnownZero(LengthWrapperDataEvaluationKind evaluationKind) const noexcept
  {
    if (evaluationKind == LengthWrapperDataEvaluationKind::Fixed
        || evaluationKind == LengthWrapperDataEvaluationKind::Percentage)
    {
      return !_floatValue;
    }

    return false;
  }

  inline bool
    LengthWrapperData::IsKnownPositive(LengthWrapperDataEvaluationKind evaluationKind) const noexcept
  {
    if (evaluationKind == LengthWrapperDataEvaluationKind::Fixed
        || evaluationKind == LengthWrapperDataEvaluationKind::Percentage)
    {
      return _floatValue > 0;
    }

    return false;
  }

  inline bool
    LengthWrapperData::IsKnownNegative(LengthWrapperDataEvaluationKind evaluationKind) const noexcept
  {
    if (evaluationKind == LengthWrapperDataEvaluationKind::Fixed
        || evaluationKind == LengthWrapperDataEvaluationKind::Percentage)
    {
      return _floatValue < 0;
    }

    return false;
  }

  inline bool LengthWrapperData::IsPossiblyZero(LengthWrapperDataEvaluationKind evaluationKind) const noexcept
  {
    if (evaluationKind == LengthWrapperDataEvaluationKind::Fixed
        || evaluationKind == LengthWrapperDataEvaluationKind::Percentage)
    {
      return !_floatValue;
    }

    return true;
  }

  inline bool
    LengthWrapperData::IsPossiblyPositive(LengthWrapperDataEvaluationKind evaluationKind) const noexcept
  {
    if (evaluationKind == LengthWrapperDataEvaluationKind::Fixed
        || evaluationKind == LengthWrapperDataEvaluationKind::Percentage)
    {
      return _floatValue > 0;
    }

    return true;
  }

  inline bool
    LengthWrapperData::IsPossiblyNegative(LengthWrapperDataEvaluationKind evaluationKind) const noexcept
  {
    if (evaluationKind == LengthWrapperDataEvaluationKind::Fixed
        || evaluationKind == LengthWrapperDataEvaluationKind::Percentage)
    {
      return _floatValue < 0;
    }

    return true;
  }

  template <typename ReturnType, typename MaximumType, typename F>
  requires InvocableReturns<MaximumType, const F &>
  ReturnType LengthWrapperData::MinimumValueForLengthWrapperDataWithLazyMaximum(
    LengthWrapperDataEvaluationKind evaluationKind, const F &lazyMaximumValueFunctor,
    ZoomNeeded token) const noexcept
  {
    switch (evaluationKind)
    {
      case LengthWrapperDataEvaluationKind::Fixed:
        krys_debug_assert(_kind == LengthWrapperDataKind::Default);
        return ReturnType(_floatValue);
      case LengthWrapperDataEvaluationKind::Percentage:
        krys_debug_assert(_kind == LengthWrapperDataKind::Default);
        return ReturnType(static_cast<float>(lazyMaximumValueFunctor() * _floatValue / 100.0f));
      case LengthWrapperDataEvaluationKind::Calculation:
        krys_debug_assert(_kind == LengthWrapperDataKind::Calculation);
        return ReturnType(NonNanCalculatedValue(lazyMaximumValueFunctor(), token));
      case LengthWrapperDataEvaluationKind::Flag:
        krys_debug_assert(_kind == LengthWrapperDataKind::Default);
        return ReturnType(0);
    }

    krys_unreachable();
  }

  template <typename ReturnType, typename MaximumType, typename F>
  requires InvocableReturns<MaximumType, const F &>
  ReturnType LengthWrapperData::MinimumValueForLengthWrapperDataWithLazyMaximum(
    LengthWrapperDataEvaluationKind evaluationKind, const F &lazyMaximumValueFunctor,
    ZoomFactor zoom) const noexcept
  {
    switch (evaluationKind)
    {
      case LengthWrapperDataEvaluationKind::Fixed:
        krys_debug_assert(_kind == LengthWrapperDataKind::Default);
        return ReturnType(_floatValue * zoom.Value);
      case LengthWrapperDataEvaluationKind::Percentage:
        krys_debug_assert(_kind == LengthWrapperDataKind::Default);
        return ReturnType(static_cast<float>(lazyMaximumValueFunctor() * _floatValue / 100.0f));
      case LengthWrapperDataEvaluationKind::Calculation:
        krys_debug_assert(_kind == LengthWrapperDataKind::Calculation);
        return ReturnType(NonNanCalculatedValue(lazyMaximumValueFunctor(), zoom));
      case LengthWrapperDataEvaluationKind::Flag:
        krys_debug_assert(_kind == LengthWrapperDataKind::Default);
        return ReturnType(0);
    }

    krys_unreachable();
  }

  template <typename ReturnType, typename MaximumType, typename F>
  requires InvocableReturns<MaximumType, const F &>
  ReturnType LengthWrapperData::ValueForLengthWrapperDataWithLazyMaximum(
    LengthWrapperDataEvaluationKind evaluationKind, const F &lazyMaximumValueFunctor,
    ZoomNeeded token) const noexcept
  {
    switch (evaluationKind)
    {
      case LengthWrapperDataEvaluationKind::Fixed:
        krys_debug_assert(_kind == LengthWrapperDataKind::Default);
        return ReturnType(_floatValue);
      case LengthWrapperDataEvaluationKind::Percentage:
        krys_debug_assert(_kind == LengthWrapperDataKind::Default);
        return ReturnType(static_cast<float>(lazyMaximumValueFunctor() * _floatValue / 100.0f));
      case LengthWrapperDataEvaluationKind::Calculation:
        krys_debug_assert(_kind == LengthWrapperDataKind::Calculation);
        return ReturnType(NonNanCalculatedValue(lazyMaximumValueFunctor(), token));
      case LengthWrapperDataEvaluationKind::Flag:
        krys_debug_assert(_kind == LengthWrapperDataKind::Default);
        return ReturnType(lazyMaximumValueFunctor());
    }

    krys_unreachable();
  }

  template <typename ReturnType, typename MaximumType, typename F>
  requires InvocableReturns<MaximumType, const F &>
  ReturnType LengthWrapperData::ValueForLengthWrapperDataWithLazyMaximum(
    LengthWrapperDataEvaluationKind evaluationKind, const F &lazyMaximumValueFunctor,
    ZoomFactor zoom) const noexcept
  {
    switch (evaluationKind)
    {
      case LengthWrapperDataEvaluationKind::Fixed:
        krys_debug_assert(_kind == LengthWrapperDataKind::Default);
        return ReturnType(_floatValue * zoom.Value);
      case LengthWrapperDataEvaluationKind::Percentage:
        krys_debug_assert(_kind == LengthWrapperDataKind::Default);
        return ReturnType(static_cast<float>(lazyMaximumValueFunctor() * _floatValue / 100.0f));
      case LengthWrapperDataEvaluationKind::Calculation:
        krys_debug_assert(_kind == LengthWrapperDataKind::Calculation);
        return ReturnType(NonNanCalculatedValue(lazyMaximumValueFunctor(), zoom));
      case LengthWrapperDataEvaluationKind::Flag:
        krys_debug_assert(_kind == LengthWrapperDataKind::Default);
        return ReturnType(lazyMaximumValueFunctor());
    }

    krys_unreachable();
  }
}
