#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"

namespace Krys::HTML
{
  // for CSS value type algorithms.
  // Requirements: Types must be able to be passed to WTF::switchOn().
  template <typename>
  inline constexpr auto TreatAsVariantLike = false;

  // The `VariantLike` concept can be used to filter to types that specialize `TreatAsVariantLike`.
  template <typename T>
  concept VariantLike = TreatAsVariantLike<T>;
}