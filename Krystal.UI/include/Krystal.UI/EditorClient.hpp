#pragma once

#include "Krystal.Booey/Page/EditorClient.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::ui
{
  /// @brief A view's editor client: runs the editor commands Windows binds keys to (Shift with the arrows to
  /// extend the selection, Ctrl+A to select all, Ctrl+C to copy, and the rest), and allows selection changes
  /// and copying.
  // FIXME: Inserting the text a keypress types (insertText), with editing commands and text input.
  class EditorClient final : public boo::EditorClient
  {
  public:
    KRYS_NODISCARD bool ShouldChangeSelectedRange(const Maybe<boo::dom::SimpleRange> &fromRange,
                                                  const Maybe<boo::dom::SimpleRange> &toRange,
                                                  boo::Affinity affinity,
                                                  bool stillSelecting) noexcept override;

    KRYS_NODISCARD bool CanCopyCut(boo::LocalFrame *frame, bool defaultValue) const noexcept override;

    bool HandleKeyboardEvent(boo::dom::KeyboardEvent &event) noexcept override;

    /// @brief The name of the editor command @p event is bound to, or empty if none is.
    KRYS_NODISCARD static utf8_stringview InterpretKeyEvent(const boo::dom::KeyboardEvent &event) noexcept;
  };
}
