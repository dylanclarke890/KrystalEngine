#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Layout/LayoutUnits.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class LayoutShape;
  class RenderObject;
}

namespace krys::boo::layout
{
  class BoxGeometry;
  class ElementBox;
  class LayoutState;

  /// @brief A box of the layout tree, which the formatting contexts lay out. It's built from a renderer and
  /// holds a copy of its style.
  class Box : public CanMakeCheckedPtr<Box>
  {
    friend class ElementBox;
    friend class LayoutState;
    KRYS_NON_COPY_MOVABLE(Box);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Box);

  public:
    enum class NodeType : uint8
    {
      Text,
      GenericElement,
      ReplacedElement,
      DocumentElement,
      Body,
      // The table generates a principal block container box called the table wrapper box that contains the
      // table box and any caption boxes.
      TableWrapperBox,
      // The table box is a block-level box that contains the table's internal table boxes.
      TableBox,
      Image,
      IFrame,
      LineBreak,
      WordBreakOpportunity,
      ListMarker,
      // These boxes are implicit flex boxes with no flex display type and they should probably turned into
      // proper flex boxes.
      ImplicitFlexBox
    };

    enum class AnonymousBox : bool
    {
      No,
      Yes
    };

    struct ElementAttributes
    {
      Box::NodeType NodeType;
      AnonymousBox IsAnonymous;
    };

    enum BaseTypeFlag : uint8
    {
      InlineTextBoxFlag,
      ElementBoxFlag,
      InitialContainingBlockFlag,
    };

  private:
    class BoxRareData
    {
    public:
      layout::CellSpan TableCellSpan;
      Maybe<LayoutUnit> ColumnWidth;
      UniquePtr<RenderStyle> FirstLineStyle;
      RefPtr<const LayoutShape> Shape;
    };

    using RareDataMap = HashMap<const Box *, UniquePtr<BoxRareData>>;

    NodeType _nodeType : 4;
    bool _isAnonymous : 1;

    uint8 _baseTypeFlags : 4; // EnumSet<BaseTypeFlag>
    bool _hasRareData : 1 {false};
    bool _isInlineIntegrationRoot : 1 {false};
    // Either first anonymous block box child or simple anonymous block container (e.g flex item).
    bool _isAnonymousTextIndentCandidateForIntegration : 1 {false};

    RenderStyle _style;

    CheckedPtr<ElementBox> _parent;

    UniquePtr<Box> _nextSibling;
    CheckedPtr<Box> _previousSibling;

    // Primary LayoutState gets a direct cache.
#if KRYS_DEBUG
    mutable WeakPtr<LayoutState> _primaryLayoutState;
#endif
    mutable UniquePtr<BoxGeometry> _cachedGeometryForPrimaryLayoutState;

    CheckedPtr<RenderObject> _renderer;

  public:
    virtual ~Box() noexcept;

    KRYS_NODISCARD bool EstablishesFormattingContext() const noexcept;
    KRYS_NODISCARD bool EstablishesBlockFormattingContext() const noexcept;
    KRYS_NODISCARD bool EstablishesInlineFormattingContext() const noexcept;
    KRYS_NODISCARD bool EstablishesTableFormattingContext() const noexcept;
    KRYS_NODISCARD bool EstablishesFlexFormattingContext() const noexcept;
    KRYS_NODISCARD bool EstablishesGridFormattingContext() const noexcept;
    KRYS_NODISCARD bool EstablishesIndependentFormattingContext() const noexcept;

    KRYS_NODISCARD bool IsInFlow() const noexcept
    {
      return !IsFloatingOrOutOfFlowPositioned();
    }
    KRYS_NODISCARD bool IsPositioned() const noexcept
    {
      return IsInFlowPositioned() || IsOutOfFlowPositioned();
    }
    KRYS_NODISCARD bool IsInFlowPositioned() const noexcept
    {
      return IsRelativelyPositioned() || IsStickyPositioned();
    }
    KRYS_NODISCARD bool IsOutOfFlowPositioned() const noexcept
    {
      return IsAbsolutelyPositioned();
    }
    KRYS_NODISCARD bool IsRelativelyPositioned() const noexcept;
    KRYS_NODISCARD bool IsStickyPositioned() const noexcept;
    KRYS_NODISCARD bool IsAbsolutelyPositioned() const noexcept;
    KRYS_NODISCARD bool IsFixedPositioned() const noexcept;
    KRYS_NODISCARD bool IsFloatingPositioned() const noexcept;
    KRYS_NODISCARD bool HasFloatClear() const noexcept;
    KRYS_NODISCARD bool IsFloatAvoider() const noexcept;

    KRYS_NODISCARD bool IsFloatingOrOutOfFlowPositioned() const noexcept
    {
      return IsFloatingPositioned() || IsOutOfFlowPositioned();
    }

    KRYS_NODISCARD bool IsContainingBlockForInFlow() const noexcept;
    KRYS_NODISCARD inline bool IsContainingBlockForFixedPosition() const noexcept;
    KRYS_NODISCARD inline bool IsContainingBlockForOutOfFlowPosition() const noexcept;

    KRYS_NODISCARD bool IsAnonymous() const noexcept
    {
      return _isAnonymous;
    }

    // Block level elements generate block level boxes.
    KRYS_NODISCARD bool IsBlockLevelBox() const noexcept;
    // A block-level box that is also a block container.
    KRYS_NODISCARD bool IsBlockBox() const noexcept;
    // A block-level box is also a block container box unless it is a table box or the principal box of a
    // replaced element.
    KRYS_NODISCARD bool IsBlockContainer() const noexcept;
    KRYS_NODISCARD bool IsInlineLevelBox() const noexcept;
    KRYS_NODISCARD bool IsInlineBox() const noexcept;
    KRYS_NODISCARD bool IsAtomicInlineBox() const noexcept;
    KRYS_NODISCARD bool IsInlineBlockBox() const noexcept;
    KRYS_NODISCARD bool IsInlineTableBox() const noexcept;
    KRYS_NODISCARD bool IsInitialContainingBlock() const noexcept
    {
      return BaseTypeFlags().Contains(InitialContainingBlockFlag);
    }
    KRYS_NODISCARD bool IsLayoutContainmentBox() const noexcept;
    KRYS_NODISCARD bool IsSizeContainmentBox() const noexcept;
    KRYS_NODISCARD bool IsInternalRubyBox() const noexcept;
    KRYS_NODISCARD bool IsRubyAnnotationBox() const noexcept;
    KRYS_NODISCARD bool IsInterlinearRubyAnnotationBox() const noexcept;

    KRYS_NODISCARD bool IsDocumentBox() const noexcept
    {
      return _nodeType == NodeType::DocumentElement;
    }
    KRYS_NODISCARD bool IsTableWrapperBox() const noexcept
    {
      return _nodeType == NodeType::TableWrapperBox;
    }
    KRYS_NODISCARD bool IsTableBox() const noexcept
    {
      return _nodeType == NodeType::TableBox;
    }
    KRYS_NODISCARD bool IsBodyBox() const noexcept
    {
      return _nodeType == NodeType::Body;
    }
    KRYS_NODISCARD bool IsLineBreakBox() const noexcept
    {
      return _nodeType == NodeType::LineBreak || _nodeType == NodeType::WordBreakOpportunity;
    }
    KRYS_NODISCARD bool IsIFrame() const noexcept
    {
      return _nodeType == NodeType::IFrame;
    }
    KRYS_NODISCARD bool IsImage() const noexcept
    {
      return _nodeType == NodeType::Image;
    }
    KRYS_NODISCARD bool IsListMarkerBox() const noexcept
    {
      return _nodeType == NodeType::ListMarker;
    }
    KRYS_NODISCARD bool IsReplacedBox() const noexcept
    {
      return _nodeType == NodeType::ReplacedElement || _nodeType == NodeType::Image
             || _nodeType == NodeType::ListMarker;
    }
    KRYS_NODISCARD bool IsWordBreakOpportunity() const noexcept
    {
      return _nodeType == NodeType::WordBreakOpportunity;
    }
    KRYS_NODISCARD bool IsInternalTableBox() const noexcept;
    KRYS_NODISCARD inline bool IsRuby() const noexcept;
    KRYS_NODISCARD inline bool IsRubyBase() const noexcept;
    KRYS_NODISCARD inline bool IsRubyInlineBox() const noexcept;
    KRYS_NODISCARD inline bool IsTableCaption() const noexcept;
    KRYS_NODISCARD inline bool IsTableHeader() const noexcept;
    KRYS_NODISCARD inline bool IsTableBody() const noexcept;
    KRYS_NODISCARD inline bool IsTableFooter() const noexcept;
    KRYS_NODISCARD inline bool IsTableRow() const noexcept;
    KRYS_NODISCARD inline bool IsTableColumnGroup() const noexcept;
    KRYS_NODISCARD inline bool IsTableColumn() const noexcept;
    KRYS_NODISCARD inline bool IsTableCell() const noexcept;
    KRYS_NODISCARD inline bool IsFlexBox() const noexcept;
    KRYS_NODISCARD bool IsFlexItem() const noexcept;
    KRYS_NODISCARD inline bool IsGridFormattingContext() const noexcept;
    KRYS_NODISCARD inline bool IsGridBox() const noexcept;
    KRYS_NODISCARD inline bool IsGridLanesBox() const noexcept;
    KRYS_NODISCARD bool IsGridItem() const noexcept;
    KRYS_NODISCARD inline bool IsListItem() const noexcept;

    KRYS_NODISCARD bool IsInlineIntegrationRoot() const noexcept
    {
      return _isInlineIntegrationRoot;
    }
    KRYS_NODISCARD bool IsAnonymousTextIndentCandidateForIntegration() const noexcept
    {
      return _isAnonymousTextIndentCandidateForIntegration;
    }

    KRYS_NODISCARD const ElementBox &Parent() const noexcept;
    KRYS_NODISCARD const Box *NextSibling() const noexcept
    {
      return _nextSibling.get();
    }
    KRYS_NODISCARD const Box *NextInFlowSibling() const noexcept;
    KRYS_NODISCARD const Box *NextInFlowOrFloatingSibling() const noexcept;
    KRYS_NODISCARD const Box *NextOutOfFlowSibling() const noexcept;
    KRYS_NODISCARD const Box *PreviousSibling() const noexcept
    {
      return _previousSibling.get();
    }
    KRYS_NODISCARD const Box *PreviousInFlowSibling() const noexcept;
    KRYS_NODISCARD const Box *PreviousInFlowOrFloatingSibling() const noexcept;
    KRYS_NODISCARD const Box *PreviousOutOfFlowSibling() const noexcept;
    KRYS_NODISCARD bool IsDescendantOf(const Box &box) const noexcept;
    KRYS_NODISCARD bool IsDescendantOfWithinFormattingContext(const Box &box) const noexcept;
    KRYS_NODISCARD bool
      IsInFormattingContextEstablishedBy(const ElementBox &formattingContextRoot) const noexcept;

    // FIXME: This is currently needed for style updates.
    KRYS_NODISCARD Box *NextSibling() noexcept
    {
      return _nextSibling.get();
    }

    KRYS_NODISCARD bool IsElementBox() const noexcept
    {
      return BaseTypeFlags().Contains(ElementBoxFlag);
    }
    KRYS_NODISCARD bool IsInlineTextBox() const noexcept
    {
      return BaseTypeFlags().Contains(InlineTextBoxFlag);
    }

    KRYS_NODISCARD bool IsPaddingApplicable() const noexcept;
    KRYS_NODISCARD bool IsOverflowVisible() const noexcept;

    void UpdateStyle(RenderStyle &&newStyle, UniquePtr<RenderStyle> &&newFirstLineStyle) noexcept;
    KRYS_NODISCARD const RenderStyle &Style() const noexcept
    {
      return _style;
    }
    KRYS_NODISCARD const RenderStyle &FirstLineStyle() const noexcept
    {
      return HasRareData() && RareData().FirstLineStyle ? *RareData().FirstLineStyle : _style;
    }
    KRYS_NODISCARD boo::WritingMode GetWritingMode() const noexcept
    {
      return Style().GetWritingMode();
    }

    // FIXME: Find a better place for random DOM things.
    void SetRowSpan(size_t rowSpan) noexcept;
    KRYS_NODISCARD size_t RowSpan() const noexcept;

    void SetColumnSpan(size_t columnSpan) noexcept;
    KRYS_NODISCARD size_t ColumnSpan() const noexcept;

    void SetColumnWidth(LayoutUnit columnWidth) noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit> ColumnWidth() const noexcept;

    void SetIsInlineIntegrationRoot() noexcept
    {
      _isInlineIntegrationRoot = true;
    }
    void SetIsAnonymousTextIndentCandidateForIntegration(bool value) noexcept
    {
      _isAnonymousTextIndentCandidateForIntegration = value;
    }

    KRYS_NODISCARD const LayoutShape *Shape() const noexcept;
    void SetShape(RefPtr<const LayoutShape> shape) noexcept;

    KRYS_NODISCARD const ElementBox *AssociatedRubyAnnotationBox() const noexcept;

    KRYS_NODISCARD RenderObject *RendererForIntegration() const noexcept;
    void SetRendererForIntegration(RenderObject *renderer) noexcept;

    KRYS_NODISCARD UniquePtr<Box> RemoveFromParent() noexcept;

  protected:
    Box(ElementAttributes &&elementAttributes, RenderStyle &&style, UniquePtr<RenderStyle> &&firstLineStyle,
        EnumSet<BaseTypeFlag> baseTypeFlags) noexcept;

  private:
    KRYS_NODISCARD bool HasRareData() const noexcept
    {
      return _hasRareData;
    }
    void SetHasRareData(bool hasRareData) noexcept
    {
      _hasRareData = hasRareData;
    }
    KRYS_NODISCARD const BoxRareData &RareData() const noexcept;
    KRYS_NODISCARD BoxRareData &RareData() noexcept;
    BoxRareData &EnsureRareData() noexcept;
    void RemoveRareData() noexcept;

    KRYS_NODISCARD EnumSet<BaseTypeFlag> BaseTypeFlags() const noexcept
    {
      return EnumSet<BaseTypeFlag>::FromRaw(_baseTypeFlags);
    }

    KRYS_NODISCARD static RareDataMap &GetRareDataMap() noexcept;
  };

  inline bool Box::IsContainingBlockForInFlow() const noexcept
  {
    return IsBlockContainer() || EstablishesFormattingContext();
  }
}
