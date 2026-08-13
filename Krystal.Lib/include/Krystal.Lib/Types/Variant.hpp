#pragma once

#include <variant>
#include <tuple>
#include <type_traits>

namespace Krys
{
  template <typename... Types>
  using Variant = std::variant<Types...>;

  template <typename... Types>
  using VariantOrSingle =
    std::conditional_t<(sizeof...(Types) == 1), std::tuple_element_t<0, std::tuple<Types...>>,
                       Variant<Types...>>;
}