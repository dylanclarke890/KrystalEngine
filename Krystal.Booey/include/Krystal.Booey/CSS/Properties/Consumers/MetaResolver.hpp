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

    KRYS_NODISCARD static ResultType Resolve(T &&consumeResult,
                                             PropertyParserOptions options = {}) noexcept
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