#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"

namespace Krys::HTML
{
  // Types can specialize this and set the value to true to be treated as "empty-like"
  // for CSS value type algorithms.
  // Requirements: None.
  template <typename>
  constexpr bool TreatAsEmptyLike = false;

  // The `EmptyLike` concept can be used to filter to types that specialize `TreatAsEmptyLike`.
  template <typename T>
  concept EmptyLike = TreatAsEmptyLike<T>;

  // Types can specialize this and set the value to true to be treated as "optional-like"
  // for CSS value type algorithms.
  // Requirements: Types be comparable to bool and have a operator* function.
  template <typename>
  constexpr bool TreatAsOptionalLike = false;

  // The `OptionalLike` concept can be used to filter to types that specialize `TreatAsOptionalLike`.
  template <typename T>
  concept OptionalLike = TreatAsOptionalLike<T>;

  // Types can specialize this and set the value to true to be treated as "tuple-like"
  // for CSS value type algorithms.
  // NOTE: This gets automatically specialized when using the *_TUPLE_LIKE_CONFORMANCE macros.
  // Requirements: Types must have conform the to the standard tuple-like pseudo-protocol.
  template <typename>
  constexpr bool TreatAsTupleLike = false;

  // The `TupleLike` concept can be used to filter to types that specialize `TreatAsTupleLike`.
  template <typename T>
  concept TupleLike = TreatAsTupleLike<T>;

  // Types can specialize this and set the value to true to be treated as "range-like"
  // for CSS value type algorithms.
  // Requirements: Types must have valid begin()/end() functions.
  template <typename>
  constexpr bool TreatAsRangeLike = false;

  // The `RangeLike` concept can be used to filter to types that specialize `TreatAsRangeLike`.
  template <typename T>
  concept RangeLike = TreatAsRangeLike<T>;

  // for CSS value type algorithms.
  // Requirements: Types must be able to be passed to Krys::SwitchOn().
  template <typename>
  constexpr bool TreatAsVariantLike = false;

  // The `VariantLike` concept can be used to filter to types that specialize `TreatAsVariantLike`.
  template <typename T>
  concept VariantLike = TreatAsVariantLike<T>;
}