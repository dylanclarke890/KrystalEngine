#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief The units a selection moves or extends by.
  // FIXME: This really should be broken up into more than one concept.
  // Frame doesn't need the 3 boundaries in this enum.
  enum class TextGranularity : uint8
  {
    CharacterGranularity,
    WordGranularity,
    SentenceGranularity,
    LineGranularity,
    ParagraphGranularity,
    DocumentGranularity,
    SentenceBoundary,
    LineBoundary,
    ParagraphBoundary,
    DocumentBoundary
  };
}
