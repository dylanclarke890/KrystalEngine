#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Concepts.hpp"
#include "Krystal.Lib/Ranges/Iterator.hpp"

namespace Krys::Ranges::Impl
{
  template <typename TOutputContainer, typename TInsertion>
  constexpr void ContainerInsertBulk(TOutputContainer &output, TInsertion &&insertion) noexcept
  {
    using TIterator = ::std::ranges::iterator_t<remove_cvref_t<TInsertion>>;
    if constexpr (has_insert_bulk<TOutputContainer, TIterator, TIterator>)
    {
      // inserting in bulk can be faster, more performant, save us some coding too
      output.insert(output.cend(), ::std::ranges::cbegin(insertion), ::std::ranges::cend(insertion));
    }
    else
    {
      // O O F! we have to insert one at a time.
      for (auto &&value : insertion)
      {
        if constexpr (has_push_back<TOutputContainer, std::iter_reference_t<TIterator>>)
        {
          output.push_back(std::forward<decltype(value)>(value));
        }
        else
        {
          output.insert(output.cend(), std::forward<decltype(value)>(value));
        }
      }
    }
  }
}