#pragma once

namespace krys::web
{
  class Document;
  class HTMLTextFormControlElement;

  /// @brief The Selection API's algorithms.
  /// @see https://w3c.github.io/selection-api/
  class SelectionAlgorithms
  {
  public:
    /// @brief Queues a `selectionchange` event at @p document, unless one is queued already.
    /// @see https://w3c.github.io/selection-api/#selectionchange-event
    static void ScheduleSelectionChangeEvent(Document &document) noexcept;

    /// @brief Queues a `selectionchange` event at @p textControl, for a change to its own selection, unless
    /// one is queued already.
    /// @see https://w3c.github.io/selection-api/#selectionchange-event
    static void ScheduleSelectionChangeEvent(HTMLTextFormControlElement &textControl) noexcept;
  };
}
