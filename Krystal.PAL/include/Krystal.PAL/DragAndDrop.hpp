#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.PAL/KeyModifiers.hpp"

namespace krys::pal
{
  /// @brief What a drop does with what's dragged.
  enum class DropOperation : uint8
  {
    /// @brief The target takes a copy, and the source keeps it.
    Copy,
    /// @brief The target takes it, and the source removes it.
    Move,
    /// @brief The target takes a link to it.
    Link,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::pal::DropOperation, 3uz)

namespace krys::pal
{
  using DropOperations = EnumSet<DropOperation>;

  /// @brief What's dragged over a window from another application, in the formats Krystal reads.
  struct DragContent
  {
    /// @brief UTF-8 text, with LF line breaks.
    Maybe<utf8_string> Text;
    /// @brief UTF-8 markup: the fragment dragged.
    Maybe<utf8_string> HTML;
    /// @brief A URL (a link, a browser's address or an internet shortcut).
    Maybe<utf8_string> URL;
    /// @brief The URL's title, if the source gave one.
    utf8_string URLTitle;
    /// @brief The paths of the files dragged.
    List<utf8_string> Files;
  };

  /// @brief What happened to a drag over a window.
  enum class DragAction : uint8
  {
    /// @brief The drag came over the window.
    Entered,
    /// @brief The drag moved over the window, or the keys held changed.
    Moved,
    /// @brief The drag left the window, or was cancelled, or was released where the host's last answer
    /// was that a drop would do nothing.
    Left,
    /// @brief The drag was dropped on the window.
    Dropped,
  };

  /// @brief A drag from another application over a window, as the window reports it.
  struct DragInput
  {
    DragAction Action {DragAction::Moved};
    /// @brief The cursor's position in client space.
    float ClientX {0};
    float ClientY {0};
    /// @brief The modifier keys held.
    KeyModifiers Modifiers;
    /// @brief What the source allows a drop to do.
    DropOperations AllowedOperations;
    /// @brief What's dragged, read as the drag enters; empty for the drag's other input.
    DragContent Content;
  };
}
