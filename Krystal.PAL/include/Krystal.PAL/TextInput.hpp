#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::pal
{
  /// @brief What happened to the text an input method composes.
  enum class CompositionAction : uint8
  {
    /// @brief A composition started.
    Started,
    /// @brief The text being composed changed: empty when it was all deleted, or the composition was
    /// cancelled.
    Updated,
    /// @brief The input method produced its result, which takes the place of the text being composed. A
    /// new composition may follow before it ends.
    Committed,
    /// @brief The composition ended. It follows a commit, or an update with empty text.
    Ended,
  };

  /// @brief A clause of the text being composed: a run the input method converts as one.
  struct CompositionClause
  {
    /// @brief Where it starts and ends in the text, in UTF-8 code units.
    uint32 Start {0};
    uint32 End {0};
    /// @brief Whether it's the clause being converted, which the candidates are for.
    bool Target {false};
  };

  /// @brief A change to the text an input method composes, as the window reports it.
  struct CompositionInput
  {
    CompositionAction Action {CompositionAction::Updated};
    /// @brief The text being composed (for updates), or the result (for commits).
    utf8_string Text;
    /// @brief The clauses of the text being composed, in order and covering it (for updates).
    List<CompositionClause> Clauses;
    /// @brief The caret in the text being composed, in UTF-8 code units (for updates).
    uint32 Caret {0};
  };

  /// @brief A rect in a window's client area, in pixels.
  struct TextInputRect
  {
    int32 X {0};
    int32 Y {0};
    int32 Width {0};
    int32 Height {0};
  };
}
