#pragma once

#include "Krystal.Core/Text/Encodings/Base.hpp"
#include <tuple>

namespace krys::text
{
  template <Encoding... TEncodings>
  struct EncodingRegistry
  {
    constexpr static size_t size = sizeof...(TEncodings);

    using Encodings = std::tuple<TEncodings...>;

    template <size_t I>
    using Encoding = std::tuple_element_t<I, Encodings>;

  private:
    template <typename Predicate>
    KRYS_NODISCARD constexpr static size_t FindIf(Predicate pred) noexcept
    {
      return FindIfImpl<0uz>(pred);
    }

    template <size_t I, typename Predicate>
    KRYS_NODISCARD constexpr static size_t FindIfImpl(Predicate pred) noexcept
    {
      if constexpr (I < size)
      {
        using E = std::tuple_element_t<I, Encodings>;
        if (pred(E {}))
        {
          return I;
        }
        
        return FindIfImpl<I + 1uz>(pred);
      }
      else
      {
        return static_cast<size_t>(-1);
      }
    }
  };
}