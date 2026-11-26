#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"

namespace krys::web
{
  class DataTransfer;
  class Element;

  /// @brief The Clipboard API's algorithms.
  /// @see https://w3c.github.io/clipboard-apis/#clipboard-actions
  class ClipboardAlgorithms
  {
  public:
    /// @brief Fires the clipboard event @p name (`copy`, `cut` or `paste`) at @p target, carrying
    /// @p clipboardData. If a `copy` or `cut` is cancelled, what its listeners set in @p clipboardData is
    /// written to the system clipboard. Afterwards @p clipboardData can't be read or written.
    /// @return Whether the action's default steps run: the event wasn't cancelled.
    /// @see https://w3c.github.io/clipboard-apis/#fire-a-clipboard-event
    KRYS_NODISCARD static bool FireClipboardEvent(const DOMStringAtom &name, Element &target,
                                                  DataTransfer &clipboardData) noexcept;
  };
}
