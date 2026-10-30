#pragma once

#include "Krystal.Booey/Graphics/WindRule.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // All types that want to expose a generated boo::WindRule must specialize WindRuleComputation the following
  // member function:
  //
  //    template<> struct krys::boo::style::WindRuleComputation<StyleType> {
  //        boo::WindRule operator()(const StyleType&);
  //    };

  template <typename StyleType>
  struct WindRuleComputation;

  struct WindRuleComputationInvoker
  {
    template <typename StyleType>
    KRYS_NODISCARD boo::WindRule operator()(const StyleType &value) const noexcept
    {
      return WindRuleComputation<StyleType> {}(value);
    }
  };
  inline constexpr WindRuleComputationInvoker ComputeWindRule {};

  // Specialization for `css::FunctionNotation`.
  template <css::ValueId Name, typename StyleType>
  struct WindRuleComputation<css::FunctionNotation<Name, StyleType>>
  {
    KRYS_NODISCARD boo::WindRule operator()(const css::FunctionNotation<Name, StyleType> &value) noexcept
    {
      return krys::boo::style::ComputeWindRule(value.Parameters);
    }
  };
}
