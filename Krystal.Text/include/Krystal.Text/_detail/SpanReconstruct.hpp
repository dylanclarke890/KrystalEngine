#pragma once

#include "Krystal.Lib/Ranges/Reconstruct.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/TypeTraits.hpp"
#include <string_view>

namespace Krys::Text::detail
{
  template <typename T, typename TSentinel>
  constexpr size_t cstringPtrSize(T *ptr, const TSentinel &sentinel) noexcept
  {
    if (ptr == nullptr)
    {
      return 0;
    }
    size_t len = 0;
    while (ptr[len] != sentinel)
    {
      ++len;
    }
    return len;
  }

  template <typename T>
  constexpr size_t cstringPtrSize(T *ptr) noexcept
  {
    constexpr T sentinel {};
    return cstringPtrSize(ptr, sentinel);
  }

  template <typename TInputTag, bool IsMutable, typename TInput>
  constexpr auto SpanReconstructAs(TInput &&input) noexcept
  {
    using TCVInput = remove_ref_t<TInput>;
    using TUInput = remove_cvref_t<TInput>;
    using TUInputTag = remove_cvref_t<TInputTag>;

    // try to catch string literal_ts / arrays
    if constexpr (IsMutable)
    {
      if constexpr (Krys::Ranges::Reconstructible<std::in_place_type_t<TUInputTag>, TInput>)
      {
        return Krys::Ranges::reconstruct(std::in_place_type<TUInputTag>, std::forward<TInput>(input));
      }
      else if constexpr (Krys::IsSpan<TUInput>)
      {
        using T = typename TUInput::element_type;
        return Span<T, TUInput::extent>(input);
      }
      else if constexpr (IsArray<TUInput> && IsConst<remove_extent_t<TCVInput>> && LValueRef<TInput>)
      {
        using TChar = remove_extent_t<TUInput>;
        if constexpr (IsCharTraitable<TChar>)
        {
          return Krys::Ranges::reconstruct(std::in_place_type<Span<const TChar>>, std::ranges::begin(input),
                                           std::ranges::begin(input) + cstringPtrSize(input));
        }
        else
        {
          using T = remove_extent_t<TCVInput>;
          return Krys::Ranges::reconstruct(std::in_place_type<Span<const T>>, std::forward<TInput>(input));
        }
      }
      else
      {
        if constexpr (std::ranges::contiguous_range<TCVInput> && std::ranges::sized_range<TCVInput>)
        {
          using T = std::ranges::range_value_t<TCVInput>;
          return Krys::Ranges::reconstruct(std::in_place_type<Span<T>>, std::forward<TInput>(input));
        }
        else
        {
          return Krys::Ranges::reconstruct(std::in_place_type<TUInputTag>, std::forward<TInput>(input));
        }
      }
    }
    else
    {
      if constexpr (Krys::Ranges::ConstReconstructible<std::in_place_type_t<TUInputTag>, TInput>)
      {
        return Krys::Ranges::const_reconstruct(std::in_place_type<TUInputTag>, std::forward<TInput>(input));
      }
      else if constexpr (Krys::IsSpan<TUInput>)
      {
        using T = const typename TUInput::value_type;
        return Span<T, TUInput::extent>(input);
      }
      else if constexpr (IsArray<TUInput> && IsConst<remove_extent_t<TCVInput>> && LValueRef<TInput>)
      {
        using TChar = remove_extent_t<TUInput>;
        if constexpr (IsCharTraitable<TChar>)
        {
          return Krys::Ranges::const_reconstruct(std::in_place_type<Span<const TChar>>,
                                                 std::ranges::cbegin(input),
                                                 std::ranges::cbegin(input) + cstringPtrSize(input));
        }
        else
        {
          using T = remove_extent_t<TCVInput>;
          return Krys::Ranges::const_reconstruct(std::in_place_type<Span<const T>>,
                                                 std::forward<TInput>(input));
        }
      }
      else
      {
        if constexpr (std::ranges::contiguous_range<TCVInput> && std::ranges::sized_range<TCVInput>)
        {
          using T = std::ranges::range_value_t<TCVInput>;
          return Krys::Ranges::const_reconstruct(std::in_place_type<Span<T>>, std::forward<TInput>(input));
        }
        else
        {
          return Krys::Ranges::const_reconstruct(std::in_place_type<TUInputTag>, std::forward<TInput>(input));
        }
      }
    }
  }

  template <typename TInputTag, typename TInput>
  constexpr decltype(auto) SpanReconstruct(TInput &&input) noexcept
  {
    return SpanReconstructAs<TInputTag, false>(std::forward<TInput>(input));
  }

  template <typename TInputTag, typename TInput>
  constexpr decltype(auto) SpanReconstructMutable(TInput &&input) noexcept
  {
    return SpanReconstructAs<TInputTag, true>(std::forward<TInput>(input));
  }

  template <typename TInputTag, typename TInput>
  using span_reconstruct_t = decltype(SpanReconstruct<TInputTag>(std::declval<TInput>()));

  template <typename TInputTag, typename TInput>
  using span_reconstruct_mutable_t = decltype(SpanReconstructMutable<TInputTag>(std::declval<TInput>()));
}
