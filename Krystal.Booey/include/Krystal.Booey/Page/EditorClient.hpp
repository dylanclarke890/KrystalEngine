#pragma once

#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/Editing/TextAffinity.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class KeyboardEvent;
}

namespace krys::boo
{
  class LocalFrame;

  /// @brief What a page's editor asks of its host: the commands keys run, and whether selections and
  /// clipboard access are allowed.
  // FIXME: The rest of the editor client: editing delegates (shouldBeginEditing, shouldInsertText,
  // shouldApplyStyle and their notifications), undo and redo, spelling and grammar, text checking and input
  // methods, with editing commands.
  class EditorClient
  {
    KRYS_NON_COPY_MOVABLE(EditorClient);

  public:
    EditorClient() noexcept = default;
    virtual ~EditorClient() noexcept = default;

    KRYS_NODISCARD virtual bool ShouldChangeSelectedRange(const Maybe<dom::SimpleRange> &fromRange,
                                                          const Maybe<dom::SimpleRange> &toRange,
                                                          Affinity affinity,
                                                          bool stillSelecting) noexcept = 0;

    KRYS_NODISCARD virtual bool CanCopyCut(LocalFrame *frame, bool defaultValue) const noexcept = 0;

    /// @brief Runs the editor command a `keydown` or `keypress` is bound to, if any.
    /// @return Whether the event was handled, so its default action is done.
    virtual bool HandleKeyboardEvent(dom::KeyboardEvent &event) noexcept = 0;
  };

  /// @brief The editor client of a page that has no host.
  class EmptyEditorClient final : public EditorClient
  {
  public:
    KRYS_NODISCARD bool ShouldChangeSelectedRange(const Maybe<dom::SimpleRange> &,
                                                  const Maybe<dom::SimpleRange> &, Affinity,
                                                  bool) noexcept final
    {
      return false;
    }

    KRYS_NODISCARD bool CanCopyCut(LocalFrame *, bool defaultValue) const noexcept final
    {
      return defaultValue;
    }

    bool HandleKeyboardEvent(dom::KeyboardEvent &) noexcept final
    {
      return false;
    }
  };
}
