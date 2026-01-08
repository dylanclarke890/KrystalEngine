#pragma once

#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Reconstruct.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Size.hpp"
#include "Krystal.Text/CharTraits.hpp"
#include <string_view>

namespace Krys::Impl
  {
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
        else if constexpr (IsArray<TUInput> && Const<remove_extent_t<TCVInput>> && LValueRef<TInput>)
        {
          using TChar = remove_extent_t<TUInput>;
          if constexpr (IsCharTraitable<TChar>)
          {
            return Krys::Ranges::reconstruct(std::in_place_type<Span<const TChar>>,
                                             Krys::Ranges::begin(input),
                                             Krys::Ranges::begin(input) + Krys::cstringPtrSize(input));
          }
          else
          {
            using T = remove_extent_t<TCVInput>;
            return Krys::Ranges::reconstruct(std::in_place_type<Span<const T>>, std::forward<TInput>(input));
          }
        }
        else
        {
          if constexpr (Krys::Ranges::ContiguousRange<TCVInput> && Krys::Ranges::SizedRange<TCVInput>)
          {
            using T = Krys::Ranges::range_element_type_t<TCVInput>;
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
        else if constexpr (IsArray<TUInput> && Const<remove_extent_t<TCVInput>> && LValueRef<TInput>)
        {
          using TChar = remove_extent_t<TUInput>;
          if constexpr (IsCharTraitable<TChar>)
          {
            return Krys::Ranges::const_reconstruct(std::in_place_type<Span<const TChar>>,
                                                   Krys::Ranges::cbegin(input),
                                                   Krys::Ranges::cbegin(input) + Krys::cstringPtrSize(input));
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
          if constexpr (Krys::Ranges::ContiguousRange<TCVInput> && Krys::Ranges::SizedRange<TCVInput>)
          {
            using T = Krys::Ranges::range_element_type_t<TCVInput>;
            return Krys::Ranges::const_reconstruct(std::in_place_type<Span<T>>, std::forward<TInput>(input));
          }
          else
          {
            return Krys::Ranges::const_reconstruct(std::in_place_type<TUInputTag>,
                                                   std::forward<TInput>(input));
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
    using span_reconstruct_mutable_t =
      decltype(SpanReconstructMutable<TInputTag>(std::declval<TInput>()));
}
