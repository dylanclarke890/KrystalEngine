/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"

namespace krys::boo::css
{
  template <typename R, typename Base, typename T, typename... Ts>
  struct MetaResolver : Base
  {
    using ResultType = R;

    KRYS_NODISCARD static ResultType Resolve(Variant<T, Ts...> &&consumeResult,
                                             PropertyParserOptions options = {}) noexcept
    requires(sizeof...(Ts) > 0)
    {
      return SwitchOn(krys::move(consumeResult),
                      [&](auto &&value) -> ResultType { return Base::Resolve(krys::move(value), options); });
    }

    KRYS_NODISCARD static ResultType Resolve(T &&consumeResult, PropertyParserOptions options = {}) noexcept
    requires(sizeof...(Ts) == 0)
    {
      return Base::Resolve(krys::move(consumeResult), options);
    }

    KRYS_NODISCARD static ResultType ConsumeAndResolve(TokenRange &tokens, PropertyParserState &state,
                                                       PropertyParserOptions options = {}) noexcept
    {
      auto result = MetaConsumer<T, Ts...>::Consume(tokens, state, options);
      if (!result)
      {
        return {};
      }

      return Resolve(krys::move(*result), options);
    }
  };
}