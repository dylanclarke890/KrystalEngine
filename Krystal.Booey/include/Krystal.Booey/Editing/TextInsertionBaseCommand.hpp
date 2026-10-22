#pragma once

#include "Krystal.Booey/Editing/CompositeEditCommand.hpp"

namespace krys::boo
{
  class LocalFrame;

  /// @brief The base of commands that insert text, which can apply themselves to a selection other than the
  /// frame's.
  class TextInsertionBaseCommand : public CompositeEditCommand
  {
  public:
    ~TextInsertionBaseCommand() noexcept override = default;

  protected:
    explicit TextInsertionBaseCommand(dom::Document &document,
                                      EditAction editingAction = EditAction::Unspecified) noexcept;

    static void ApplyTextInsertionCommand(LocalFrame *frame, TextInsertionBaseCommand &command,
                                          const VisibleSelection &selectionForInsertion,
                                          const VisibleSelection &endingSelection) noexcept;
  };

  KRYS_NODISCARD utf8_string DispatchBeforeTextInsertedEvent(utf8_stringview text,
                                                             const VisibleSelection &selectionForInsertion,
                                                             bool insertionIsForUpdatingComposition) noexcept;
  KRYS_NODISCARD bool CanAppendNewLineFeedToSelection(const VisibleSelection &selection) noexcept;

  // LineOperation should define member function "opeartor (size_t lineOffset, size_t lineLength, bool
  // isLastLine)". lienLength doesn't include the newline character. So the value of lineLength could be 0.
  template <typename TLineOperation>
  void ForEachLineInString(utf8_stringview string, const TLineOperation &operation) noexcept
  {
    size_t offset = 0;
    size_t newline;
    while ((newline = string.find(u8'\n', offset)) != utf8_stringview::npos)
    {
      operation(offset, newline - offset, false);
      offset = newline + 1;
    }
    if (!offset)
    {
      operation(0, string.size(), true);
    }
    else
    {
      size_t length = string.size();
      if (length != offset)
      {
        operation(offset, length - offset, true);
      }
    }
  }
}
