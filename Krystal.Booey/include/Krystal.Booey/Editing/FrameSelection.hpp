#pragma once

#include "Krystal.Booey/Editing/VisibleSelection.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class CharacterData;
  class Document;
  class Element;
  class Node;
}

namespace krys::boo
{
  enum class UserTriggered : bool
  {
    No,
    Yes
  };

  enum class FrameSelectionSetSelectionOption : uint8
  {
    FireSelectEvent,
    CloseTyping,
    ClearTypingStyle,
    SpellCorrectionTriggered,
    DoNotSetFocus,
    DictationTriggered,
    IsUserTriggered,
    RevealSelection,
    RevealSelectionUpToMainFrame,
    SmoothScroll,
    DelegateMainFrameScroll,
    RevealSelectionBounds,
    ForceCenterScroll,
    ForBindings,
    DoNotNotifyEditorClients,
    MaintainLiveRange,
    OnlyAllowForwardScrolling
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::FrameSelectionSetSelectionOption, 17uz);

namespace krys::boo
{
  /// @brief A document's selection: what's selected, whether the document is focused and active (which picks
  /// the selection's colours), and keeping the render tree's selection (`RenderSelection`) in step with it.
  // FIXME: The caret (CaretBase, the caret animator, its rect and painting, caret visibility), with carets in
  // editable content.
  // FIXME: Modifying the selection by granularity and vertical distance (modify, willBeModified, the
  // modifyExtending and modifyMoving functions), revealing it (revealSelection), the typing style, the live
  // range of the DOM's Selection and selectionchange events, with the keyboard and the DOM's Selection.
  // FIXME: Focusing the editable root a selection is placed in (setFocusedElementIfNeeded), with carets in
  // editable content.
  // FIXME: Frames: the drag caret (DragCaretController), selecting the frame element in the parent
  // (selectFrameElementInParentIfFullySelected), selections in another frame's document.
  // FIXME: The editor's notifications (selectionWillChange, respondToChangedSelection, shouldChangeSelection
  // asking its client), text form controls and closing typing, with editing commands and form controls.
  class FrameSelection
  {
    KRYS_NON_COPY_MOVABLE(FrameSelection);

  public:
    using SetSelectionOption = FrameSelectionSetSelectionOption;

    enum class CursorAlignOnScroll : bool
    {
      IfNeeded,
      Always
    };

    enum class EndPointsAdjustmentMode : bool
    {
      DoNotAdjust,
      AdjustAtBidiBoundary
    };

    enum class ShouldCloseTyping : bool
    {
      No,
      Yes
    };

    enum class ClipToVisibleContent : bool
    {
      No,
      Yes
    };

  private:
    dom::Document *_document;
    VisibleSelection _selection;
    VisiblePosition _originalBase; // Used to store base before the adjustment at bidi boundary.
    TextGranularity _granularity {TextGranularity::CharacterGranularity};

    bool _focused {false};
    bool _shouldShowBlockCursor {false};
    bool _pendingSelectionUpdate {false};

  public:
    explicit FrameSelection(dom::Document *document = nullptr) noexcept;
    ~FrameSelection() noexcept;

    KRYS_NODISCARD static constexpr EnumSet<SetSelectionOption>
      DefaultSetSelectionOptions(UserTriggered userTriggered = UserTriggered::No) noexcept
    {
      EnumSet<SetSelectionOption> options {SetSelectionOption::CloseTyping,
                                           SetSelectionOption::ClearTypingStyle};
      if (userTriggered == UserTriggered::Yes)
      {
        options.Add({SetSelectionOption::RevealSelection, SetSelectionOption::FireSelectEvent,
                     SetSelectionOption::IsUserTriggered});
      }
      return options;
    }

    KRYS_NODISCARD dom::Element *RootEditableElementOrDocumentElement() const noexcept;

    void MoveTo(const VisiblePosition &position, UserTriggered userTriggered = UserTriggered::No,
                CursorAlignOnScroll align = CursorAlignOnScroll::IfNeeded) noexcept;
    void MoveTo(const VisiblePosition &base, const VisiblePosition &extent,
                UserTriggered userTriggered = UserTriggered::No) noexcept;
    void MoveTo(const dom::Position &position, Affinity affinity,
                UserTriggered userTriggered = UserTriggered::No) noexcept;
    void MoveTo(const dom::Position &base, const dom::Position &extent, Affinity affinity,
                UserTriggered userTriggered = UserTriggered::No) noexcept;
    void MoveWithoutValidationTo(const dom::Position &base, const dom::Position &extent,
                                 bool selectionHasDirection,
                                 EnumSet<SetSelectionOption> options = DefaultSetSelectionOptions()) noexcept;

    KRYS_NODISCARD const VisibleSelection &Selection() const noexcept
    {
      return _selection;
    }
    void SetSelection(const VisibleSelection &selection,
                      EnumSet<SetSelectionOption> options = DefaultSetSelectionOptions(),
                      CursorAlignOnScroll align = CursorAlignOnScroll::IfNeeded,
                      TextGranularity granularity = TextGranularity::CharacterGranularity) noexcept;

    bool SetSelectedRange(const Maybe<dom::SimpleRange> &range, Affinity affinity,
                          ShouldCloseTyping closeTyping,
                          UserTriggered userTriggered = UserTriggered::No) noexcept;
    void SelectAll() noexcept;
    void Clear() noexcept;
    void WillBeRemovedFromFrame() noexcept;

    /// @brief Brings the render tree's selection up to date, once the rendering is.
    void UpdateAppearanceAfterUpdatingRendering() noexcept;

    /// @brief Clears the render tree's selection, to be set again at the next rendering update.
    void SetNeedsSelectionUpdate() noexcept;

    /// @brief Whether @p point (in the document's coordinates) is over the selected range.
    KRYS_NODISCARD bool Contains(const LayoutPoint &point) const noexcept;

    KRYS_NODISCARD TextGranularity Granularity() const noexcept
    {
      return _granularity;
    }

    void SetStart(const VisiblePosition &position, UserTriggered trigger = UserTriggered::No) noexcept;
    void SetEnd(const VisiblePosition &position, UserTriggered trigger = UserTriggered::No) noexcept;

    void SetBase(const VisiblePosition &position, UserTriggered userTriggered = UserTriggered::No) noexcept;
    void SetBase(const dom::Position &position, Affinity affinity,
                 UserTriggered userTriggered = UserTriggered::No) noexcept;
    void SetExtent(const VisiblePosition &position, UserTriggered userTriggered = UserTriggered::No) noexcept;
    void SetExtent(const dom::Position &position, Affinity affinity,
                   UserTriggered userTriggered = UserTriggered::No) noexcept;

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return _selection.IsNone();
    }
    KRYS_NODISCARD bool IsCaret() const noexcept
    {
      return _selection.IsCaret();
    }
    KRYS_NODISCARD bool IsRange() const noexcept
    {
      return _selection.IsRange();
    }
    KRYS_NODISCARD bool IsCaretOrRange() const noexcept
    {
      return _selection.IsCaretOrRange();
    }
    KRYS_NODISCARD bool IsAll(EditingBoundaryCrossingRule rule =
                                EditingBoundaryCrossingRule::CannotCrossEditingBoundary) const noexcept
    {
      return _selection.IsAll(rule);
    }

    /// @brief Moves the selection's end points out of @p node before it's removed.
    void NodeWillBeRemoved(dom::Node &node) noexcept;
    /// @brief Adjusts the selection's end points in @p node for its data replaced at @p offset.
    void TextWasReplaced(dom::CharacterData &node, uint32 offset, uint32 oldLength,
                         uint32 newLength) noexcept;

    void SetFocused(bool isFocused) noexcept;
    KRYS_NODISCARD bool IsFocused() const noexcept
    {
      return _focused;
    }
    KRYS_NODISCARD bool IsFocusedAndActive() const noexcept;

    /// @brief Sets the render tree's selection from the selection.
    void UpdateAppearance() noexcept;

    KRYS_NODISCARD bool ShouldChangeSelection(const VisibleSelection &newSelection) const noexcept;

    void SetSelectionByMouseIfDifferent(
      const VisibleSelection &passedNewSelection, TextGranularity granularity,
      EndPointsAdjustmentMode endpointsAdjustmentMode = EndPointsAdjustmentMode::DoNotAdjust) noexcept;

    KRYS_NODISCARD FloatRect
      SelectionBounds(ClipToVisibleContent clipToVisibleContent = ClipToVisibleContent::Yes) noexcept;

    void SetSelectionFromNone() noexcept;

    KRYS_NODISCARD bool ShouldShowBlockCursor() const noexcept
    {
      return _shouldShowBlockCursor;
    }

    KRYS_NODISCARD bool IsInDocumentTree() const noexcept;
    KRYS_NODISCARD bool IsConnectedToDocument() const noexcept;

  private:
    void UpdateSelectionAppearanceNow() noexcept;
    void UpdateAndRevealSelection() noexcept;

    bool SetSelectionWithoutUpdatingAppearance(const VisibleSelection &newSelectionPossiblyWithoutDirection,
                                               EnumSet<SetSelectionOption> options, CursorAlignOnScroll align,
                                               TextGranularity granularity) noexcept;
    void SetNodeFlags(VisibleSelection &selection, bool value) noexcept;

    void RespondToNodeModification(dom::Node &node, bool anchorRemoved, bool focusRemoved, bool baseRemoved,
                                   bool extentRemoved, bool startRemoved, bool endRemoved) noexcept;

    void FocusedOrActiveStateChanged() noexcept;
  };
}
