#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Iterator.hpp"
#include "Krystal.Lib/Ranges/Range.hpp"

namespace Krys::Ranges
{
  namespace Impl
  {
    struct range_iterator_unsaveable_t
    {
    } constexpr range_iterator_unsaveable;
  }

  /// @brief Clones an iterator if it is not aan input or output iterator.
  /// @param it The iterator to save.
  /// @returns Either an implementation-defined object that indicates the save could not be done, or a
  /// move/copy of the given iterator.
  template <typename TIt>
  constexpr auto SaveIterator(TIt &&it) noexcept
  {
    if constexpr (SameType<remove_cvref_t<TIt>, Impl::range_iterator_unsaveable_t>)
    {
      return Impl::range_iterator_unsaveable;
    }
    else if constexpr (Krys::Ranges::InputOrOutputIteratorExact<remove_cvref_t<TIt>>)
    {
      return Impl::range_iterator_unsaveable;
    }
    else
    {
      return std::forward<TIt>(it);
    }
  }

  /// @brief Clones an iterator if it is not aan input or output iterator.
  /// @param it An object returned by a previous call to ztd::ranges::SaveIterator.
  /// @param fallbackIt The iterator to return if the given iterator is either an input/output iterator
  /// or is the implementation-defined "unsaveable" object.
  /// @returns Either a clone of the `it` if possible, or if not returns the `fallbackIt`.
  template <typename TIt, typename TFallbackIt>
  constexpr auto RestoreIterator(TIt &&it, TFallbackIt &&fallbackIt) noexcept
  {
    if constexpr (SameType<remove_cvref_t<TIt>, Impl::range_iterator_unsaveable_t>)
    {
      return std::forward<TFallbackIt>(fallbackIt);
    }
    else if constexpr (Krys::Ranges::InputOrOutputIteratorExact<remove_cvref_t<TIt>>)
    {
      return std::forward<TFallbackIt>(fallbackIt);
    }
    else
    {
      return std::forward<TIt>(it);
    }
  }

  template <typename TIt>
  concept UnsaveableIterator = SameType<Impl::range_iterator_unsaveable_t, remove_cvref_t<TIt>>;
}
