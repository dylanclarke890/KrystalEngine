#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @brief Why an element with `content-visibility: auto` is relevant to the user.
  /// @see https://drafts.csswg.org/css-contain/#relevant-to-the-user
  enum class ContentRelevancy : uint8
  {
    OnScreen,
    Focused,
    IsInTopLayer,
    Selected,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::ContentRelevancy, 4uz);
