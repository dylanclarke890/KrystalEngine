#include "Krystal.Booey/Layout/LayoutTree/LayoutBox.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInitialContainingBlock.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/Shapes/LayoutShape.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include <utility>

namespace krys::boo::layout
{
  Box::Box(ElementAttributes &&elementAttributes, RenderStyle &&style,
           UniquePtr<RenderStyle> &&firstLineStyle, EnumSet<BaseTypeFlag> baseTypeFlags) noexcept
      : _nodeType(elementAttributes.NodeType), _isAnonymous(static_cast<bool>(elementAttributes.IsAnonymous)),
        _baseTypeFlags(static_cast<uint8>(baseTypeFlags.ToRaw())), _style(krys::move(style))
  {
    if (firstLineStyle)
    {
      EnsureRareData().FirstLineStyle = krys::move(firstLineStyle);
    }
  }

  Box::~Box() noexcept
  {
    if (_hasRareData) KRYS_UNLIKELY
    {
      RemoveRareData();
    }
    if (_renderer)
    {
      _renderer->ClearLayoutBox();
    }
  }

  const ElementBox &Box::Parent() const noexcept
  {
    return *_parent;
  }

  RenderObject *Box::RendererForIntegration() const noexcept
  {
    return _renderer.get();
  }

  void Box::SetRendererForIntegration(RenderObject *renderer) noexcept
  {
    _renderer = ShareCheckedPtr(renderer);
  }

  UniquePtr<Box> Box::RemoveFromParent() noexcept
  {
    auto &nextOrFirst = _previousSibling ? _previousSibling->_nextSibling : _parent->_firstChild;
    auto &previousOrLast = _nextSibling ? _nextSibling->_previousSibling : _parent->_lastChild;

    krys_debug_assert(nextOrFirst.get() == this);
    krys_debug_assert(previousOrLast.get() == this);

    auto ownedSelf = std::exchange(nextOrFirst, std::exchange(_nextSibling, nullptr));

    previousOrLast = std::exchange(_previousSibling, nullptr);
    _parent = nullptr;

    krys_debug_assert(ownedSelf);
    return ownedSelf;
  }

  void Box::UpdateStyle(RenderStyle &&newStyle, UniquePtr<RenderStyle> &&newFirstLineStyle) noexcept
  {
    _style = krys::move(newStyle);
    if (newFirstLineStyle)
    {
      EnsureRareData().FirstLineStyle = krys::move(newFirstLineStyle);
    }
    else if (HasRareData())
    {
      RareData().FirstLineStyle = {};
    }
  }

  bool Box::EstablishesFormattingContext() const noexcept
  {
    return EstablishesInlineFormattingContext() || EstablishesBlockFormattingContext()
           || EstablishesTableFormattingContext() || EstablishesFlexFormattingContext()
           || EstablishesGridFormattingContext() || EstablishesIndependentFormattingContext();
  }

  bool Box::EstablishesBlockFormattingContext() const noexcept
  {
    if (IsInlineIntegrationRoot())
    {
      return true;
    }

    // ICB always creates a new (inital) block formatting context.
    if (Is<InitialContainingBlock>(*this))
    {
      return true;
    }

    if (IsTableWrapperBox())
    {
      return true;
    }

    // A block box that establishes an independent formatting context establishes a new block formatting
    // context for its contents.
    if (IsBlockBox() && EstablishesIndependentFormattingContext())
    {
      return true;
    }

    // 9.4.1 Block formatting contexts
    // Floats, absolutely positioned elements, block containers (such as inline-blocks, table-cells, and
    // table-captions) that are not block boxes, and block boxes with 'overflow' other than 'visible' (except
    // when that value has been propagated to the viewport) establish new block formatting contexts for their
    // contents.
    if (IsFloatingPositioned())
    {
      // Not all floating or out-of-positioned block level boxes establish BFC.
      // See [9.7 Relationships between 'display', 'position', and 'float'] for details.
      return IsBlockContainer();
    }

    if (IsBlockContainer() && !IsBlockBox())
    {
      return true;
    }

    if (IsBlockBox() && !IsOverflowVisible())
    {
      return true;
    }

    return false;
  }

  bool Box::EstablishesInlineFormattingContext() const noexcept
  {
    if (IsInlineIntegrationRoot())
    {
      return true;
    }

    // 9.4.2 Inline formatting contexts
    // An inline formatting context is established by a block container box that contains no block-level
    // boxes.
    if (!IsBlockContainer())
    {
      return false;
    }

    auto *elementBox = DynamicDowncast<ElementBox>(*this);
    if (!elementBox)
    {
      return false;
    }

    // FIXME ???
    if (!elementBox->FirstInFlowChild())
    {
      return false;
    }

    // It's enough to check the first in-flow child since we can't have both block and inline level sibling
    // boxes.
    return elementBox->FirstInFlowChild()->IsInlineLevelBox();
  }

  bool Box::EstablishesTableFormattingContext() const noexcept
  {
    return IsTableBox();
  }

  bool Box::EstablishesFlexFormattingContext() const noexcept
  {
    return IsFlexBox();
  }

  bool Box::EstablishesGridFormattingContext() const noexcept
  {
    return IsGridFormattingContext();
  }

  bool Box::EstablishesIndependentFormattingContext() const noexcept
  {
    return IsLayoutContainmentBox() || IsAbsolutelyPositioned() || IsFlexItem() || IsGridItem();
  }

  bool Box::IsRelativelyPositioned() const noexcept
  {
    return _style.GetPosition() == PositionType::Relative;
  }

  bool Box::IsStickyPositioned() const noexcept
  {
    return _style.GetPosition() == PositionType::Sticky;
  }

  bool Box::IsAbsolutelyPositioned() const noexcept
  {
    return _style.GetPosition() == PositionType::Absolute || IsFixedPositioned();
  }

  bool Box::IsFixedPositioned() const noexcept
  {
    return _style.GetPosition() == PositionType::Fixed;
  }

  bool Box::IsFloatingPositioned() const noexcept
  {
    // FIXME: Rendering code caches values like this. (style="position: absolute; float: left")
    if (IsOutOfFlowPositioned())
    {
      return false;
    }
    return _style.Floating() != Float::None;
  }

  bool Box::HasFloatClear() const noexcept
  {
    return _style.GetClear() != Clear::None && (IsBlockLevelBox() || IsLineBreakBox());
  }

  bool Box::IsFloatAvoider() const noexcept
  {
    if (IsFloatingPositioned() || HasFloatClear())
    {
      return true;
    }

    return EstablishesTableFormattingContext() || EstablishesIndependentFormattingContext()
           || EstablishesBlockFormattingContext();
  }

  bool Box::IsInlineBlockBox() const noexcept
  {
    return _style.Display() == DisplayType::InlineBlock;
  }

  bool Box::IsInlineTableBox() const noexcept
  {
    return _style.Display() == DisplayType::InlineTable;
  }

  bool Box::IsBlockLevelBox() const noexcept
  {
    // Block level elements generate block level boxes.
    auto display = _style.Display();
    return display == DisplayType::Block || display == DisplayType::ListItem || display == DisplayType::Table
           || display == DisplayType::Flex || display == DisplayType::Box || display == DisplayType::Grid
           || display == DisplayType::GridLanes || display == DisplayType::FlowRoot;
  }

  bool Box::IsBlockBox() const noexcept
  {
    // A block-level box that is also a block container.
    return IsBlockLevelBox() && IsBlockContainer();
  }

  bool Box::IsInlineLevelBox() const noexcept
  {
    // Inline level elements generate inline level boxes.
    auto display = _style.Display();
    return display == DisplayType::Inline || display == DisplayType::InlineBox
           || display == DisplayType::InlineFlex || display == DisplayType::InlineGrid
           || display == DisplayType::InlineGridLanes || display == DisplayType::Ruby
           || display == DisplayType::RubyBase || display == DisplayType::RubyAnnotation || IsInlineBlockBox()
           || IsInlineTableBox();
  }

  bool Box::IsInlineBox() const noexcept
  {
    // An inline box is one that is both inline-level and whose contents participate in its containing inline
    // formatting context. A non-replaced element with a 'display' value of 'inline' generates an inline box.
    auto display = _style.Display();
    return (display == DisplayType::Inline || display == DisplayType::Ruby
            || display == DisplayType::RubyBase)
           && !IsReplacedBox();
  }

  bool Box::IsAtomicInlineBox() const noexcept
  {
    // Inline-level boxes that are not inline boxes (such as replaced inline-level elements, inline-block
    // elements, and inline-table elements) are called atomic inline-level boxes because they participate in
    // their inline formatting context as a single opaque box.
    return IsInlineLevelBox() && !IsInlineBox();
  }

  bool Box::IsFlexItem() const noexcept
  {
    // Each in-flow child of a flex container becomes a flex item
    // (https://www.w3.org/TR/css-flexbox-1/#flex-items).
    return IsInFlow() && Parent().IsFlexBox();
  }

  bool Box::IsGridItem() const noexcept
  {
    return IsInFlow() && Parent().IsGridFormattingContext();
  }

  bool Box::IsBlockContainer() const noexcept
  {
    auto display = _style.Display();
    return display == DisplayType::Block || display == DisplayType::FlowRoot
           || display == DisplayType::ListItem || display == DisplayType::RubyBlock || IsInlineBlockBox()
           || IsTableCell() || IsTableCaption(); // TODO && !replaced element
  }

  bool Box::IsLayoutContainmentBox() const noexcept
  {
    auto supportsLayoutContainment = [&]
    {
      // If the element does not generate a principal box (as is the case with display values of contents or
      // none), or its principal box is an internal table box other than table-cell, or an internal ruby box,
      // or a non-atomic inline-level box, layout containment has no effect.
      if (IsInternalTableBox())
      {
        return IsTableCell();
      }
      if (IsInternalRubyBox())
      {
        return false;
      }
      if (IsInlineLevelBox())
      {
        return IsAtomicInlineBox();
      }
      return true;
    };
    return _style.UsedContain().Contains(style::ContainValue::Layout) && supportsLayoutContainment();
  }

  bool Box::IsRubyAnnotationBox() const noexcept
  {
    return _style.Display() == DisplayType::RubyAnnotation;
  }

  bool Box::IsInterlinearRubyAnnotationBox() const noexcept
  {
    return IsRubyAnnotationBox() && !_style.IsInterCharacterRubyPosition();
  }

  bool Box::IsInternalRubyBox() const noexcept
  {
    return _style.Display() == DisplayType::RubyBase || _style.Display() == DisplayType::RubyAnnotation;
  }

  bool Box::IsSizeContainmentBox() const noexcept
  {
    auto supportsSizeContainment = [&]
    {
      // If the element does not generate a principal box (as is the case with display: contents or display:
      // none), or its inner display type is table, or its principal box is an internal table box, or an
      // internal ruby box, or a non-atomic inline-level box, size containment has no effect.
      if (IsInternalTableBox() || IsTableBox())
      {
        return false;
      }
      if (IsInternalRubyBox())
      {
        return false;
      }
      if (IsInlineLevelBox())
      {
        return IsAtomicInlineBox();
      }
      return true;
    };
    return _style.UsedContain().Contains(style::ContainValue::Size) && supportsSizeContainment();
  }

  bool Box::IsInternalTableBox() const noexcept
  {
    // table-row-group, table-header-group, table-footer-group, table-row, table-cell, table-column-group,
    // table-column generates the appropriate internal table box which participates in a table formatting
    // context.
    return IsTableBody() || IsTableHeader() || IsTableFooter() || IsTableRow() || IsTableCell()
           || IsTableColumnGroup() || IsTableColumn();
  }

  const Box *Box::NextInFlowSibling() const noexcept
  {
    auto *nextSibling = NextSibling();
    while (nextSibling && !nextSibling->IsInFlow())
    {
      nextSibling = nextSibling->NextSibling();
    }
    return nextSibling;
  }

  const Box *Box::NextInFlowOrFloatingSibling() const noexcept
  {
    auto *nextSibling = NextSibling();
    while (nextSibling && !(nextSibling->IsInFlow() || nextSibling->IsFloatingPositioned()))
    {
      nextSibling = nextSibling->NextSibling();
    }
    return nextSibling;
  }

  const Box *Box::NextOutOfFlowSibling() const noexcept
  {
    auto *nextSibling = NextSibling();
    while (nextSibling && !nextSibling->IsOutOfFlowPositioned())
    {
      nextSibling = nextSibling->NextSibling();
    }
    return nextSibling;
  }

  const Box *Box::PreviousInFlowSibling() const noexcept
  {
    auto *previousSibling = PreviousSibling();
    while (previousSibling && !previousSibling->IsInFlow())
    {
      previousSibling = previousSibling->PreviousSibling();
    }
    return previousSibling;
  }

  const Box *Box::PreviousInFlowOrFloatingSibling() const noexcept
  {
    auto *previousSibling = PreviousSibling();
    while (previousSibling && !(previousSibling->IsInFlow() || previousSibling->IsFloatingPositioned()))
    {
      previousSibling = previousSibling->PreviousSibling();
    }
    return previousSibling;
  }

  const Box *Box::PreviousOutOfFlowSibling() const noexcept
  {
    auto *previousSibling = PreviousSibling();
    while (previousSibling && !previousSibling->IsOutOfFlowPositioned())
    {
      previousSibling = previousSibling->PreviousSibling();
    }
    return previousSibling;
  }

  bool Box::IsDescendantOf(const Box &box) const noexcept
  {
    for (auto *ancestor = &Parent(); !ancestor->IsInitialContainingBlock(); ancestor = &ancestor->Parent())
    {
      if (ancestor == &box)
      {
        return true;
      }
    }
    return false;
  }

  bool Box::IsDescendantOfWithinFormattingContext(const Box &box) const noexcept
  {
    for (auto *ancestor = &Parent(); !ancestor->EstablishesFormattingContext();
         ancestor = &ancestor->Parent())
    {
      if (ancestor == &box)
      {
        return true;
      }
    }
    return false;
  }

  bool Box::IsInFormattingContextEstablishedBy(const ElementBox &formattingContextRoot) const noexcept
  {
    krys_debug_assert(formattingContextRoot.EstablishesFormattingContext());

    auto *ancestor = &Parent();
    while (true)
    {
      if (ancestor->EstablishesFormattingContext())
      {
        break;
      }
      ancestor = &ancestor->Parent();
    }
    return ancestor == &formattingContextRoot;
  }

  bool Box::IsOverflowVisible() const noexcept
  {
    auto isOverflowVisible =
      _style.OverflowX() == Overflow::Visible || _style.OverflowY() == Overflow::Visible;
    // UAs must apply the 'overflow' property set on the root element to the viewport. When the root element
    // is an HTML "HTML" element or an XHTML "html" element, and that element has an HTML "BODY" element or an
    // XHTML "body" element as a child, user agents must instead apply the 'overflow' property from the first
    // such child element to the viewport, if the value on the root element is 'visible'. The 'visible' value
    // when used for the viewport must be interpreted as 'auto'. The element from which the value is
    // propagated must have a used value for 'overflow' of 'visible'.
    if (IsBodyBox())
    {
      auto &documentBox = Parent();
      if (!documentBox.IsDocumentBox())
      {
        return isOverflowVisible;
      }
      if (!documentBox.IsOverflowVisible())
      {
        return isOverflowVisible;
      }
      return true;
    }
    if (Is<InitialContainingBlock>(*this))
    {
      auto *documentBox = Downcast<ElementBox>(*this).FirstChild();
      if (!documentBox || !documentBox->IsDocumentBox())
      {
        return isOverflowVisible;
      }
      auto *elementBox = DynamicDowncast<ElementBox>(*documentBox);
      if (!elementBox)
      {
        return isOverflowVisible;
      }
      auto *bodyBox = elementBox->FirstChild();
      if (!bodyBox || !bodyBox->IsBodyBox())
      {
        return isOverflowVisible;
      }
      auto &bodyBoxStyle = bodyBox->Style();
      return bodyBoxStyle.OverflowX() == Overflow::Visible || bodyBoxStyle.OverflowY() == Overflow::Visible;
    }
    return isOverflowVisible;
  }

  bool Box::IsPaddingApplicable() const noexcept
  {
    if (IsAnonymous())
    {
      return false;
    }

    if (IsTableBox() && Style().GetBorderCollapse() == BorderCollapse::Collapse)
    {
      // When the table collapses its borders with inner table elements, there's no room for padding.
      return false;
    }

    // 8.4 Padding properties:
    // Applies to: all elements except table-row-group, table-header-group, table-footer-group, table-row,
    // table-column-group and table-column
    return !IsTableHeader() && !IsTableBody() && !IsTableFooter() && !IsTableRow() && !IsTableColumnGroup()
           && !IsTableColumn();
  }

  void Box::SetRowSpan(size_t rowSpan) noexcept
  {
    EnsureRareData().TableCellSpan.Row = rowSpan;
  }

  void Box::SetColumnSpan(size_t columnSpan) noexcept
  {
    EnsureRareData().TableCellSpan.Column = columnSpan;
  }

  size_t Box::RowSpan() const noexcept
  {
    if (!HasRareData())
    {
      return 1;
    }
    return RareData().TableCellSpan.Row;
  }

  size_t Box::ColumnSpan() const noexcept
  {
    if (!HasRareData())
    {
      return 1;
    }
    return RareData().TableCellSpan.Column;
  }

  void Box::SetColumnWidth(LayoutUnit columnWidth) noexcept
  {
    EnsureRareData().ColumnWidth = columnWidth;
  }

  Maybe<LayoutUnit> Box::ColumnWidth() const noexcept
  {
    if (!HasRareData())
    {
      return null;
    }
    return RareData().ColumnWidth;
  }

  const LayoutShape *Box::Shape() const noexcept
  {
    if (!HasRareData())
    {
      return nullptr;
    }
    return RareData().Shape.get();
  }

  void Box::SetShape(RefPtr<const LayoutShape> shape) noexcept
  {
    EnsureRareData().Shape = krys::move(shape);
  }

  const ElementBox *Box::AssociatedRubyAnnotationBox() const noexcept
  {
    if (Style().Display() != DisplayType::RubyBase)
    {
      return nullptr;
    }

    auto *next = NextSibling();
    if (!next || next->Style().Display() != DisplayType::RubyAnnotation)
    {
      return nullptr;
    }

    return DynamicDowncast<ElementBox>(next);
  }

  Box::RareDataMap &Box::GetRareDataMap() noexcept
  {
    static NeverDestroyed<RareDataMap> map;
    return map;
  }

  const Box::BoxRareData &Box::RareData() const noexcept
  {
    krys_debug_assert(HasRareData());
    return *GetRareDataMap().at(this);
  }

  Box::BoxRareData &Box::RareData() noexcept
  {
    krys_debug_assert(HasRareData());
    return *GetRareDataMap().at(this);
  }

  Box::BoxRareData &Box::EnsureRareData() noexcept
  {
    SetHasRareData(true);
    auto &rareData = GetRareDataMap()[this];
    if (!rareData)
    {
      rareData = CreateUnique<BoxRareData>();
    }
    return *rareData;
  }

  void Box::RemoveRareData() noexcept
  {
    GetRareDataMap().erase(this);
    SetHasRareData(false);
  }
}
