#pragma once

#include "Krystal.Text/InlineContainers/Forward.hpp"
#include <cstddef>
#include <type_traits>

namespace Krys
{
  namespace Impl
  {
    template <typename>
    class IsInlineVector : public std::false_type
    {
    };

    template <typename T, std::size_t Capacity>
    class IsInlineVector<InlineVector<T, Capacity>> : public std::true_type
    {
    };

    template <typename>
    class IsInlineBasicString : public std::false_type
    {
    };

    template <typename T, std::size_t Capacity>
    class IsInlineBasicString<InlineBasicString<T, Capacity>> : public std::true_type
    {
    };
  }

  template <typename T>
  concept IsInlineVector = Impl::IsInlineVector<T>::value;

  template <typename T>
  concept IsInlineBasicString = Impl::IsInlineBasicString<T>::value;
}
