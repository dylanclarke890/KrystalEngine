#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/Editing/VisibleSelection.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Page/ScrollBehavior.hpp"
#include "Krystal.Booey/Platform/CaretAnimator.hpp"
#include "Krystal.Booey/Platform/ScrollAlignment.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  class CharacterData;
  class Document;
  class Element;
  class Node;
  class Range;
}

namespace krys::boo
{
  enum class UserTriggered : bool
  {
    No,
    Yes
  };

  enum class CaretVisibility : bool
  {
    Visible,
    Hidden
  };

  enum class CaretVisibilitySuppressionReason : uint8
  {
    IsNotFocusedOrActive,
    TextPlaceholderIsShowing,
  };

  enum class RevealExtentOption : bool
  {
    RevealExtent,
    DoNotRevealExtent
  };

  enum class ForceCenterScroll : bool
  {
    No,
    Yes
  };

  // FIXME: Only scrolling forward (onlyAllowForwardScrolling), once scrolling a rect into view can.
  struct RevealSelectionOptions
  {
    SelectionRevealMode RevealMode {SelectionRevealMode::Reveal};
    ScrollAlignment Alignment {ScrollAlignment::AlignCenterIfNeeded};
    RevealExtentOption RevealExtent {RevealExtentOption::DoNotRevealExtent};
    ScrollBehavior Behavior {ScrollBehavior::Instant};
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
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::CaretVisibilitySuppressionReason, 2uz);

namespace krys::boo
{
  class Color;
  class GraphicsContext;
  class RenderBlock;
  class RenderStyle;
  class RenderView;

  /// @brief A caret's rect, local to the block that paints it, and whether it's shown.
  class CaretBase
  {
    KRYS_NON_COPY_MOVABLE(CaretBase);

  private:
    LayoutRect
      _caretLocalRect; // caret rect in coords local to the renderer responsible for painting the caret
    bool _caretRectNeedsUpdate {
      true}; // true if _caretLocalRect (and _absCaretBounds in FrameSelection) need to be calculated
    CaretVisibility _caretVisibility;

  public:
    KRYS_NODISCARD static Color ComputeCaretColor(const RenderStyle &elementStyle,
                                                  const dom::Node *node) noexcept;

  protected:
    explicit CaretBase(CaretVisibility visibility = CaretVisibility::Hidden) noexcept;

    void InvalidateCaretRect(dom::Node *node, bool caretRectChanged = false,
                             CaretAnimator *caretAnimator = nullptr) noexcept;
    void ClearCaretRect() noexcept;
    bool UpdateCaretRect(dom::Document &document, const VisiblePosition &caretPosition) noexcept;
    KRYS_NODISCARD bool ShouldRepaintCaret(const RenderView *view, bool isContentEditable) const noexcept;
    void PaintCaret(const dom::Node &node, GraphicsContext &context, const LayoutPoint &paintOffset,
                    CaretAnimator *caretAnimator) const noexcept;

    KRYS_NODISCARD const LayoutRect &LocalCaretRectWithoutUpdate() const noexcept
    {
      return _caretLocalRect;
    }

    KRYS_NODISCARD bool ShouldUpdateCaretRect() const noexcept
    {
      return _caretRectNeedsUpdate;
    }
    void SetCaretRectNeedsUpdate() noexcept
    {
      _caretRectNeedsUpdate = true;
    }

    void SetCaretVisibility(CaretVisibility visibility) noexcept
    {
      _caretVisibility = visibility;
    }
    KRYS_NODISCARD bool CaretIsVisible() const noexcept
    {
      return _caretVisibility == CaretVisibility::Visible;
    }
    KRYS_NODISCARD CaretVisibility GetCaretVisibility() const noexcept
    {
      return _caretVisibility;
    }
  };

  /// @brief A document's selection: what's selected, whether the document is focused and active (which picks
  /// the selection's colours), and keeping the render tree's selection (`RenderSelection`) in step with it.
  // FIXME: The block cursor of overtype mode (paintBlockCursor), with editing commands; the caret colour the
  // host sets (setCaretColor), which only iOS uses.
  // FIXME: The typing style, with editing commands.
  // FIXME: selectionchange at text form controls, with form controls.
  // FIXME: Frames: the drag caret (DragCaretController), selecting the frame element in the parent
  // (selectFrameElementInParentIfFullySelected), selections in another frame's document.
  // FIXME: The editor's notifications (selectionWillChange, respondToChangedSelection, shouldChangeSelection
  // asking its client), text form controls and closing typing, with editing commands and form controls.
  class FrameSelection final : private CaretBase, public CaretAnimationClient
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

    enum class Alteration : bool
    {
      Move,
      Extend
    };

    enum class VerticalDirection : bool
    {
      Up,
      Down
    };

    enum class ShouldUpdateAppearance : bool
    {
      No,
      Yes
    };

  private:
    enum class PositionType : uint8
    {
      Start,
      End,
      Extent
    };

  private:
    dom::Document *_document;
    VisibleSelection _selection;
    VisiblePosition _originalBase; // Used to store base before the adjustment at bidi boundary.
    TextGranularity _granularity {TextGranularity::CharacterGranularity};
    RefPtr<dom::Node> _previousCaretNode; // The last node which painted the caret. Retained for clearing the
                                          // old caret when it moves.
    RefPtr<dom::Range> _associatedLiveRange;
    // The painted bounds of the caret in absolute coordinates
    IntRect _absCaretBounds;
    UniquePtr<CaretAnimator> _caretAnimator;
    EnumSet<CaretVisibilitySuppressionReason> _caretVisibilitySuppressionReasons;
    Maybe<LayoutUnit> _xPosForVerticalArrowNavigation;
    SelectionRevealMode _selectionRevealMode {SelectionRevealMode::DoNotReveal};

    bool _caretInsidePositionFixed {false};
    bool _absCaretBoundsDirty {true};
    bool _focused {false};
    bool _alwaysAlignCursorOnScrollWhenRevealingSelection {false};
    bool _hasScheduledSelectionChangeEventOnDocument {false};
    bool _shouldShowBlockCursor {false};
    bool _pendingSelectionUpdate {false};

  public:
    explicit FrameSelection(dom::Document *document = nullptr) noexcept;
    ~FrameSelection() noexcept override;

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

    bool Modify(Alteration alter, SelectionDirection direction, TextGranularity granularity,
                UserTriggered userTriggered = UserTriggered::No) noexcept;
    bool Modify(Alteration alter, uint32 verticalDistance, VerticalDirection direction,
                UserTriggered userTriggered = UserTriggered::No,
                CursorAlignOnScroll align = CursorAlignOnScroll::IfNeeded) noexcept;

    void WillBeModified(Alteration alter, SelectionDirection direction) noexcept;
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

    /// @brief Scrolls the selection (its caret, extent or bounds) into view.
    void RevealSelection(const RevealSelectionOptions &revealSelectionOptions = {}) noexcept;

    /// @brief The renderer that is responsible for painting the caret (in the selection start node).
    KRYS_NODISCARD RenderBlock *CaretRendererWithoutUpdatingLayout() const noexcept;

    /// @brief Bounds of possibly-transformed caret in absolute coordinates.
    KRYS_NODISCARD IntRect AbsoluteCaretBounds(bool *insideFixed = nullptr) noexcept;
    void SetCaretRectNeedsUpdate() noexcept
    {
      CaretBase::SetCaretRectNeedsUpdate();
    }

    void AddCaretVisibilitySuppressionReason(
      CaretVisibilitySuppressionReason reason,
      ShouldUpdateAppearance doAppearanceUpdate = ShouldUpdateAppearance::Yes) noexcept;
    void RemoveCaretVisibilitySuppressionReason(
      CaretVisibilitySuppressionReason reason,
      ShouldUpdateAppearance doAppearanceUpdate = ShouldUpdateAppearance::Yes) noexcept;

    KRYS_NODISCARD bool IsCaretVisible() const noexcept
    {
      return CaretBase::CaretIsVisible();
    }
    void PaintCaret(GraphicsContext &context, const LayoutPoint &paintOffset) noexcept;

    /// @brief Used to suspend caret blinking while the mouse is down.
    void SetCaretBlinkingSuspended(bool suspended) noexcept;
    KRYS_NODISCARD bool IsCaretBlinkingSuspended() const noexcept;

    KRYS_NODISCARD CaretAnimator &GetCaretAnimator() noexcept
    {
      return *_caretAnimator;
    }
    KRYS_NODISCARD const CaretAnimator &GetCaretAnimator() const noexcept
    {
      return *_caretAnimator;
    }

    KRYS_NODISCARD dom::Document *GetDocument() noexcept override;

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

    /// @brief The live range the DOM's Selection gives out for the selection, made when it's first asked for.
    KRYS_NODISCARD RefPtr<dom::Range> AssociatedLiveRange() noexcept;
    void AssociateLiveRange(dom::Range &liveRange) noexcept;
    void DisassociateLiveRange() noexcept;
    /// @brief Sets the selection from the associated live range, after its boundary points changed.
    void UpdateFromAssociatedLiveRange() noexcept;
    void UpdateOrDisassociateLiveRange(bool shouldMaintainLiveRange) noexcept;

    KRYS_NODISCARD bool IsInDocumentTree() const noexcept;
    KRYS_NODISCARD bool IsConnectedToDocument() const noexcept;

  private:
    void UpdateCaretVisibility(ShouldUpdateAppearance doAppearanceUpdate) noexcept;
    bool RecomputeCaretRect() noexcept;
    void InvalidateCaretRect() noexcept;

    void CaretAnimationDidUpdate(CaretAnimator &animator) noexcept override;
    KRYS_NODISCARD dom::Node *CaretNode() noexcept override;
    KRYS_NODISCARD LayoutRect LocalCaretRect() const noexcept override
    {
      return LocalCaretRectWithoutUpdate();
    }

    void SetFocusedElementIfNeeded(EnumSet<SetSelectionOption> options) noexcept;

    void UpdateSelectionAppearanceNow() noexcept;
    void UpdateAndRevealSelection(ScrollBehavior scrollBehavior = ScrollBehavior::Instant,
                                  RevealExtentOption revealExtent = RevealExtentOption::RevealExtent,
                                  ForceCenterScroll forceCenterScroll = ForceCenterScroll::No) noexcept;

    KRYS_NODISCARD TextDirection DirectionOfEnclosingBlock() noexcept;
    KRYS_NODISCARD TextDirection DirectionOfSelection() noexcept;

    KRYS_NODISCARD VisiblePosition PositionForPlatform(bool isGetStart) const noexcept;
    KRYS_NODISCARD VisiblePosition StartForPlatform() const noexcept;
    KRYS_NODISCARD VisiblePosition EndForPlatform() const noexcept;
    KRYS_NODISCARD VisiblePosition
      NextWordPositionForPlatform(const VisiblePosition &originalPosition) noexcept;

    KRYS_NODISCARD VisiblePosition ModifyExtendingRight(TextGranularity granularity,
                                                        UserTriggered userTriggered) noexcept;
    KRYS_NODISCARD VisiblePosition ModifyExtendingForward(TextGranularity granularity,
                                                          UserTriggered userTriggered) noexcept;
    KRYS_NODISCARD VisiblePosition ModifyMovingRight(TextGranularity granularity,
                                                     bool *reachedBoundary = nullptr) noexcept;
    KRYS_NODISCARD VisiblePosition ModifyMovingForward(TextGranularity granularity,
                                                       bool *reachedBoundary = nullptr) noexcept;
    KRYS_NODISCARD VisiblePosition ModifyExtendingLeft(TextGranularity granularity,
                                                       UserTriggered userTriggered) noexcept;
    KRYS_NODISCARD VisiblePosition ModifyExtendingBackward(TextGranularity granularity,
                                                           UserTriggered userTriggered) noexcept;
    KRYS_NODISCARD VisiblePosition ModifyMovingLeft(TextGranularity granularity,
                                                    bool *reachedBoundary = nullptr) noexcept;
    KRYS_NODISCARD VisiblePosition ModifyMovingBackward(TextGranularity granularity,
                                                        bool *reachedBoundary = nullptr) noexcept;
    void AdjustSelectionExtentIfNeeded(VisiblePosition &extent, bool isForward,
                                       UserTriggered userTriggered) noexcept;

    KRYS_NODISCARD LayoutUnit LineDirectionPointForBlockDirectionNavigation(PositionType type) noexcept;

    bool DispatchSelectStart() noexcept;

    bool SetSelectionWithoutUpdatingAppearance(const VisibleSelection &newSelectionPossiblyWithoutDirection,
                                               EnumSet<SetSelectionOption> options, CursorAlignOnScroll align,
                                               TextGranularity granularity) noexcept;
    void SetNodeFlags(VisibleSelection &selection, bool value) noexcept;

    void RespondToNodeModification(dom::Node &node, bool anchorRemoved, bool focusRemoved, bool baseRemoved,
                                   bool extentRemoved, bool startRemoved, bool endRemoved) noexcept;

    void FocusedOrActiveStateChanged() noexcept;
  };
}
