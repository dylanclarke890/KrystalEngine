#pragma once

#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Web/Css/Calc/Tree.hpp"
#include "Krystal.Web/Css/Values/Units.hpp"
#include <utility>

namespace krys::web::css::calc::tests
{
  /// @brief A numeric leaf of @p value in @p unit.
  KRYS_NODISCARD inline Child Num(double value, UnitType unit = UnitType::Number)
  {
    return CreateNumeric(value, unit);
  }

  /// @brief @p children as a list.
  template <typename... Cs>
  KRYS_NODISCARD List<Child> ChildList(Cs &&...children)
  {
    List<Child> list;
    (list.push_back(std::forward<Cs>(children)), ...);
    return list;
  }

  /// @brief A node for the operation @p op, of @p type.
  template <typename Op>
  KRYS_NODISCARD Child OperationNode(Op op, Type type = Type::CreateLength())
  {
    return CreateChild(krys::move(op), type);
  }
}
