#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperData.hpp"
#include "Krystal.Booey/Style/Calc/Value.hpp"
#include "Krystal.Booey/Style/Calc/ValueMap.hpp"
#include <cmath>

namespace krys::boo::style
{
  LengthWrapperData::LengthWrapperData(uint8 opaqueType, Ref<calc::Value> &&value) noexcept
      : _opaqueType {opaqueType}, _kind {LengthWrapperDataKind::Calculation}
  {
    _calculationValueHandle = calc::ValueMap::CalculationValues().Insert(krys::move(value));
  }

  calc::Value &LengthWrapperData::CalculationValue() const noexcept
  {
    krys_debug_assert(_kind == LengthWrapperDataKind::Calculation);
    return calc::ValueMap::CalculationValues().Get(_calculationValueHandle);
  }

  Ref<calc::Value> LengthWrapperData::ProtectedCalculationValue() const noexcept
  {
    return ShareRef(CalculationValue());
  }

  void LengthWrapperData::AddRef() const noexcept
  {
    krys_debug_assert(_kind == LengthWrapperDataKind::Calculation);
    calc::ValueMap::CalculationValues().AddRef(_calculationValueHandle);
  }

  void LengthWrapperData::SubRef() const noexcept
  {
    krys_debug_assert(_kind == LengthWrapperDataKind::Calculation);
    calc::ValueMap::CalculationValues().SubRef(_calculationValueHandle);
  }

  float LengthWrapperData::NonNanCalculatedValue(float maxValue, const ZoomFactor &usedZoom) const noexcept
  {
    krys_debug_assert(_kind == LengthWrapperDataKind::Calculation);
    float result = static_cast<float>(ProtectedCalculationValue()->Evaluate(maxValue, usedZoom));
    if (std::isnan(result))
    {
      return 0;
    }

    return result;
  }

  float LengthWrapperData::NonNanCalculatedValue(float maxValue, const ZoomNeeded &token) const noexcept
  {
    krys_debug_assert(_kind == LengthWrapperDataKind::Calculation);
    float result = static_cast<float>(ProtectedCalculationValue()->Evaluate(maxValue, token));
    if (std::isnan(result))
    {
      return 0;
    }

    return result;
  }

  bool LengthWrapperData::IsCalculatedEqual(const LengthWrapperData &other) const noexcept
  {
    return CalculationValue() == other.CalculationValue();
  }
}
