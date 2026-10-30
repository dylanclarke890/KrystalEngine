#pragma once

#include "Krystal.Booey/Style/Calc/Tree.hpp"
#include "Krystal.Booey/Style/Calc/Value.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion to `calc::Child`.

  KRYS_NODISCARD inline calc::Child CopyCalculation(Ref<calc::Value> value) noexcept
  {
    return value->CopyRoot();
  }

  KRYS_NODISCARD inline calc::Child CopyCalculation(Calc auto const &value) noexcept
  {
    return value.ProtectedCalculation()->CopyRoot();
  }

  template <auto R, typename V>
  KRYS_NODISCARD calc::Child CopyCalculation(const Number<R, V> &value) noexcept
  {
    return calc::CreateNumber(value.Value);
  }

  template <auto R, typename V>
  KRYS_NODISCARD calc::Child CopyCalculation(const Percentage<R, V> &value) noexcept
  {
    return calc::CreatePercentage(value.Value);
  }

  template <auto R, typename V>
  KRYS_NODISCARD calc::Child CopyCalculation(const Length<R, V> &value) noexcept
  {
    return calc::CreateDimension(value.UnresolvedValue());
  }

  KRYS_NODISCARD inline calc::Child CopyCalculation(Numeric auto const &value) noexcept
  {
    return calc::CreateDimension(value.Value);
  }

  KRYS_NODISCARD inline calc::Child CopyCalculation(DimensionPercentageNumeric auto const &value) noexcept
  {
    return krys::SwitchOn(value, [](const auto &alternative) { return CopyCalculation(alternative); });
  }
}
