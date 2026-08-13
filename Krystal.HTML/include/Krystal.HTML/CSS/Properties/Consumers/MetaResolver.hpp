#pragma once

#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumer.hpp"

namespace Krys::HTML
{
  template <typename R, typename Base, typename T, typename... Ts>
  struct MetaResolver : Base
  {
    using ResultType = R;

    KRYS_NODISCARD static ResultType Resolve(Variant<T, Ts...> &&consumeResult,
                                             CSSPropertyParserOptions options = {}) noexcept
    requires(sizeof...(Ts) > 0)
    {
      return SwitchOn(Krys::Move(consumeResult),
                      [&](auto &&value) -> ResultType { return Base::Resolve(Krys::Move(value), options); });
    }

    KRYS_NODISCARD static ResultType Resolve(T &&consumeResult,
                                             CSSPropertyParserOptions options = {}) noexcept
    requires(sizeof...(Ts) == 0)
    {
      return Base::Resolve(Krys::Move(consumeResult), options);
    }

    KRYS_NODISCARD static ResultType ConsumeAndResolve(CSSTokenRange &range, CSSPropertyParserState &state,
                                                       CSSPropertyParserOptions options = {}) noexcept
    {
      auto result = MetaConsumer<T, Ts...>::Consume(range, state, options);
      if (!result)
      {
        return {};
      }

      return Resolve(Krys::Move(*result), options);
    }
  };
}