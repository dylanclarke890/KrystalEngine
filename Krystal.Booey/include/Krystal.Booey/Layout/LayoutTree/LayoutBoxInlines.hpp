#pragma once

#include "Krystal.Booey/Layout/LayoutTree/LayoutBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::layout
{
  inline bool Box::IsRuby() const noexcept
  {
    return Style().Display() == DisplayType::Ruby;
  }

  inline bool Box::IsRubyBase() const noexcept
  {
    return Style().Display() == DisplayType::RubyBase;
  }

  inline bool Box::IsRubyInlineBox() const noexcept
  {
    return IsRuby() || IsRubyBase();
  }

  inline bool Box::IsTableCaption() const noexcept
  {
    return Style().Display() == DisplayType::TableCaption;
  }

  inline bool Box::IsTableHeader() const noexcept
  {
    return Style().Display() == DisplayType::TableHeaderGroup;
  }

  inline bool Box::IsTableBody() const noexcept
  {
    return Style().Display() == DisplayType::TableRowGroup;
  }

  inline bool Box::IsTableFooter() const noexcept
  {
    return Style().Display() == DisplayType::TableFooterGroup;
  }

  inline bool Box::IsTableRow() const noexcept
  {
    return Style().Display() == DisplayType::TableRow;
  }

  inline bool Box::IsTableColumnGroup() const noexcept
  {
    return Style().Display() == DisplayType::TableColumnGroup;
  }

  inline bool Box::IsTableColumn() const noexcept
  {
    return Style().Display() == DisplayType::TableColumn;
  }

  inline bool Box::IsTableCell() const noexcept
  {
    return Style().Display() == DisplayType::TableCell;
  }

  inline bool Box::IsFlexBox() const noexcept
  {
    return Style().Display() == DisplayType::Flex || Style().Display() == DisplayType::InlineFlex
           || _nodeType == NodeType::ImplicitFlexBox;
  }

  inline bool Box::IsGridFormattingContext() const noexcept
  {
    return IsGridBox() || IsGridLanesBox();
  }

  inline bool Box::IsGridBox() const noexcept
  {
    return Style().Display() == DisplayType::Grid || Style().Display() == DisplayType::InlineGrid;
  }

  inline bool Box::IsGridLanesBox() const noexcept
  {
    return Style().Display() == DisplayType::GridLanes || Style().Display() == DisplayType::InlineGridLanes;
  }

  inline bool Box::IsListItem() const noexcept
  {
    return Style().Display() == DisplayType::ListItem;
  }

  inline bool Box::IsContainingBlockForFixedPosition() const noexcept
  {
    return IsInitialContainingBlock() || IsLayoutContainmentBox() || Style().HasTransform();
  }

  inline bool Box::IsContainingBlockForOutOfFlowPosition() const noexcept
  {
    return IsInitialContainingBlock() || IsPositioned() || IsLayoutContainmentBox() || Style().HasTransform();
  }
}
