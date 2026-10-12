#pragma once

#include <tuple>

namespace krys
{
  namespace detail
  {
    template <std::size_t I, typename... TupleLikes>
    using zip_tuple_at_index_t = std::tuple<std::tuple_element_t<I, std::decay_t<TupleLikes>>...>;

    template <std::size_t I, typename... TupleLikes>
    auto zip_tuple_at_index(TupleLikes &&...tupleLikes) noexcept
    {
      return zip_tuple_at_index_t<I, TupleLikes...> {get<I>(std::forward<TupleLikes>(tupleLikes))...};
    }

    template <typename... TupleLikes, std::size_t... I>
    auto tuple_zip_impl(TupleLikes &&...tupleLikes, std::index_sequence<I...>) noexcept
    {
      return std::tuple<zip_tuple_at_index_t<I, TupleLikes...>...> {
        zip_tuple_at_index<I>(std::forward<TupleLikes>(tupleLikes)...)...};
    }

  } // namespace detail

  template <typename Head, typename... Tail>
  auto ZipTuple(Head &&head, Tail &&...tail) noexcept
  {
    constexpr std::size_t size = std::tuple_size_v<std::decay_t<Head>>;

    static_assert(((std::tuple_size_v<std::decay_t<Tail>> == size) && ...),
                  "Tuple size mismatch, can not zip.");

    return detail::tuple_zip_impl<Head, Tail...>(std::forward<Head>(head), std::forward<Tail>(tail)...,
                                                 std::make_index_sequence<size>());
  }
}