/**
 * Copyright (C) 2018-2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

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
