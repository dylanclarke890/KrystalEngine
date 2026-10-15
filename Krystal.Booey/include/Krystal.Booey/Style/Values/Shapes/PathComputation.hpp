#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // All types that want to expose a generated boo::Path must specialize PathComputation the following member
  // function:
  //
  //    template<> struct krys::boo::style::PathComputation<StyleType> {
  //        boo::Path operator()(const StyleType&, const FloatRect&);
  //    };

  template <typename StyleType>
  struct PathComputation;

  struct PathComputationInvoker
  {
    template <typename StyleType, typename Reference>
    KRYS_NODISCARD boo::Path operator()(const StyleType &value, const Reference &reference) const noexcept
    {
      return PathComputation<StyleType> {}(value, reference);
    }
  };
  inline constexpr PathComputationInvoker ComputePath {};

  // Specialization for `css::FunctionNotation`.
  template <css::ValueId Name, typename StyleType>
  struct PathComputation<css::FunctionNotation<Name, StyleType>>
  {
    template <typename Reference>
    KRYS_NODISCARD boo::Path operator()(const css::FunctionNotation<Name, StyleType> &value,
                                        const Reference &reference) noexcept
    {
      return krys::boo::style::ComputePath(value.Parameters, reference);
    }
  };
}
