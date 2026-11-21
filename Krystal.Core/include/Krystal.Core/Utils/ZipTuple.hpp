/*
 * Copyright (C) 2008-2024 Apple Inc. All rights reserved.
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
 * Copyright (C) 2013 Patrick Gansterer <paroga@paroga.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

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
        // NOLINTNEXTLINE(bugprone-use-after-move): each call moves out only element I of each tuple.
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
