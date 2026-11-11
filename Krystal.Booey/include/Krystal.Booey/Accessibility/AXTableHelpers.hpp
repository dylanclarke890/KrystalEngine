/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Accessibility/AccessibilityRole.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"

namespace krys::boo
{
  class RenderObject;

  namespace dom
  {
    class Element;
    class Node;
  }

  namespace html
  {
    class HTMLTableElement;
    class HTMLTableSectionElement;
  }
}

namespace krys::boo::ax
{
  class AXObjectCache;
}

/// @brief What the table, row and cell objects share: which elements are tables, rows and cells, and the
/// heuristic that tells a data table from a layout table.
namespace krys::boo::ax::table_helpers
{
  /// @brief The role a cell takes when its table is a layout table, not a data table.
  constexpr AccessibilityRole LayoutTableCellRole = AccessibilityRole::TextGroup;

  /// @brief Whether @p element's `role` (or its custom element's default role) lists @p role.
  KRYS_NODISCARD bool HasRole(const dom::Element &element, dom::DOMStringView role) noexcept;
  /// @brief Whether @p element has no `role` (and its custom element no default role).
  KRYS_NODISCARD bool HasNoRole(const dom::Element &element) noexcept;
  /// @brief Whether @p element's role is `grid`, `table` or `treegrid`.
  KRYS_NODISCARD bool HasTableRole(const dom::Element &element) noexcept;
  /// @brief Whether @p element's role is `gridcell`, `cell`, `columnheader` or `rowheader`.
  KRYS_NODISCARD bool HasCellARIARole(const dom::Element &element) noexcept;
  KRYS_NODISCARD bool HasRowRole(const dom::Element &element) noexcept;
  /// @brief Whether @p element is a `thead`, `tbody` or `tfoot`, or has the `rowgroup` role.
  KRYS_NODISCARD bool IsRowGroup(const dom::Element &element) noexcept;

  KRYS_NODISCARD bool IsTableRole(AccessibilityRole role) noexcept;
  /// @brief Whether @p element is a row: `role=row`, or a `tr` (or `display: table-row` element) with no
  /// role.
  KRYS_NODISCARD bool IsTableRowElement(const dom::Element &element) noexcept;
  /// @brief Whether @p element is a cell: a cell role, or a `td` or `th` (or `display: table-cell` element)
  /// with no role.
  KRYS_NODISCARD bool IsTableCellElement(const dom::Element &element) noexcept;

  /// @brief The `table` element @p node is, or that @p renderer (a table renderer) is for.
  KRYS_NODISCARD html::HTMLTableElement *TableElementIncludingAncestors(dom::Node *node,
                                                                        RenderObject *renderer) noexcept;
  /// @brief Whether the table has something only data tables have: a caption, summary, header or footer,
  /// rules, or columns.
  KRYS_NODISCARD bool TableElementIndicatesAccessibleTable(const html::HTMLTableElement &table) noexcept;
  /// @brief Whether the author gave the section a role other than a group (which says they want a data
  /// table).
  KRYS_NODISCARD bool TableSectionIndicatesAccessibleTable(html::HTMLTableSectionElement &section,
                                                           AXObjectCache &cache) noexcept;
  /// @brief WebKit's heuristic for a table with nothing that marks it out: looks at its rows' and cells'
  /// attributes, borders, colours and number.
  KRYS_NODISCARD bool IsDataTableWithTraversal(html::HTMLTableElement &table, AXObjectCache &cache) noexcept;
}
