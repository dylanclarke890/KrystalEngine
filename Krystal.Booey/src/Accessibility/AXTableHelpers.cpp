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

#include "Krystal.Booey/Accessibility/AXTableHelpers.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityObject.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableCaptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableCellElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableRowElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableSectionElement.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTable.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTableRow.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include <array>

namespace krys::boo::ax::table_helpers
{
  namespace
  {
    using html::TagName;
    using infra::StringAlgorithms;

    KRYS_NODISCARD TagName TagNameOf(const dom::Element &element) noexcept
    {
      auto *htmlElement = DynamicDowncast<html::HTMLElement>(element);
      return htmlElement ? htmlElement->TagQualifiedName().TagName() : TagName::Unknown;
    }

    KRYS_NODISCARD bool HasAnyRole(const dom::Element &element, Span<const dom::DOMStringView> roles) noexcept
    {
      auto value = AccessibilityObject::AttributeWithDefaultARIA(element, u8"role");
      for (const auto &name : StringAlgorithms::SplitOnWhitespace(value))
      {
        for (auto role : roles)
        {
          if (StringAlgorithms::ASCIICaseInsensitiveMatch(name, role))
          {
            return true;
          }
        }
      }
      return false;
    }

    /// @brief @p element's @p name attribute as an integer, 0 if it isn't one.
    KRYS_NODISCARD int64 IntegralAttribute(const dom::Element &element, dom::DOMStringView name) noexcept
    {
      auto value = element.GetAttribute(dom::DOMStringAtom(name));
      if (!value)
      {
        return 0;
      }
      auto result = html::Numbers::ParseInteger(*value);
      return result.Success() ? result.Value : 0;
    }

    KRYS_NODISCARD bool HasNonEmptyAttribute(const dom::Element &element, dom::DOMStringView name) noexcept
    {
      auto value = element.GetAttribute(dom::DOMStringAtom(name));
      return value && !value->empty();
    }

    KRYS_NODISCARD const RenderStyle *StyleFrom(const dom::Element &element) noexcept
    {
      if (auto *renderer = element.Renderer())
      {
        return &renderer->Style();
      }
      return element.ExistingComputedStyle();
    }

    KRYS_NODISCARD Color BackgroundColorOf(const RenderStyle &style) noexcept
    {
      return style.ColorResolvingCurrentColor(css::PropertyId::BackgroundColor);
    }
  }

  bool HasRole(const dom::Element &element, dom::DOMStringView role) noexcept
  {
    const dom::DOMStringView roles[] = {role};
    return HasAnyRole(element, roles);
  }

  bool HasNoRole(const dom::Element &element) noexcept
  {
    return AccessibilityObject::AttributeWithDefaultARIA(element, u8"role").empty();
  }

  bool HasTableRole(const dom::Element &element) noexcept
  {
    const dom::DOMStringView roles[] = {u8"grid", u8"table", u8"treegrid"};
    return HasAnyRole(element, roles);
  }

  bool HasCellARIARole(const dom::Element &element) noexcept
  {
    const dom::DOMStringView roles[] = {u8"gridcell", u8"cell", u8"columnheader", u8"rowheader"};
    return HasAnyRole(element, roles);
  }

  bool HasRowRole(const dom::Element &element) noexcept
  {
    return HasRole(element, u8"row");
  }

  bool IsRowGroup(const dom::Element &element) noexcept
  {
    auto tagName = TagNameOf(element);
    return tagName == TagName::thead || tagName == TagName::tbody || tagName == TagName::tfoot
           || HasRole(element, u8"rowgroup");
  }

  bool IsTableRole(AccessibilityRole role) noexcept
  {
    switch (role)
    {
      case AccessibilityRole::Table:
      case AccessibilityRole::Grid:
      case AccessibilityRole::TreeGrid: return true;
      default:                          return false;
    }
  }

  bool IsTableRowElement(const dom::Element &element) noexcept
  {
    if (HasRowRole(element))
    {
      return true;
    }
    if (!HasNoRole(element))
    {
      // This has a non-row role, so it shouldn't be considered a row.
      return false;
    }
    return Is<RenderTableRow>(element.Renderer()) || Is<html::HTMLTableRowElement>(element);
  }

  bool IsTableCellElement(const dom::Element &element) noexcept
  {
    if (HasCellARIARole(element))
    {
      return true;
    }
    if (Is<html::HTMLTableCellElement>(element) && HasNoRole(element))
    {
      return true;
    }
    return Is<RenderTableCell>(element.Renderer());
  }

  html::HTMLTableElement *TableElementIncludingAncestors(dom::Node *node, RenderObject *renderer) noexcept
  {
    if (auto *table = DynamicDowncast<html::HTMLTableElement>(node))
    {
      return table;
    }
    auto *renderTable = DynamicDowncast<RenderTable>(renderer);
    if (!renderTable)
    {
      return nullptr;
    }
    if (auto *table = DynamicDowncast<html::HTMLTableElement>(renderTable->GetElement()))
    {
      return table;
    }
    // Try to find the table element when the object is mapped to an anonymous table renderer.
    auto *firstChild = renderTable->FirstChild();
    auto *firstChildNode = firstChild ? firstChild->GetNode() : nullptr;
    if (!firstChildNode)
    {
      return nullptr;
    }
    if (auto *table = DynamicDowncast<html::HTMLTableElement>(*firstChildNode))
    {
      return table;
    }
    // The table element is between the child and the element the anonymous table is in (a table element with
    // another display type, say); one further out is another table's.
    dom::Node *container = nullptr;
    for (auto *parent = renderTable->Parent(); parent && !container; parent = parent->Parent())
    {
      container = parent->GetNode();
    }
    for (auto *ancestor = firstChildNode->ParentElement(); ancestor; ancestor = ancestor->ParentElement())
    {
      if (auto *table = DynamicDowncast<html::HTMLTableElement>(*ancestor))
      {
        return table;
      }
      if (ancestor == container)
      {
        break;
      }
    }
    return nullptr;
  }

  bool TableElementIndicatesAccessibleTable(const html::HTMLTableElement &table) noexcept
  {
    // If there is a caption element, summary, THEAD, or TFOOT section, it's most certainly a data table.
    auto head = table.THead();
    auto foot = table.TFoot();
    if (!table.Summary().empty() || (head && head->Renderer()) || (foot && foot->Renderer())
        || table.Caption())
    {
      return true;
    }

    // If someone used the "rules" attribute, the table should appear.
    if (!table.Rules().empty())
    {
      return true;
    }

    // If there's a colgroup or col element, it's probably a data table.
    for (auto *child = table.FirstElementChild().get(); child; child = child->NextElementSibling().get())
    {
      auto tagName = TagNameOf(*child);
      if (tagName == TagName::col || tagName == TagName::colgroup)
      {
        return true;
      }
    }
    return false;
  }

  bool TableSectionIndicatesAccessibleTable(html::HTMLTableSectionElement &section,
                                            AXObjectCache &cache) noexcept
  {
    // Use the presence of any non-group role as a sign that the author wants this to be an accessibility
    // table (rather than a layout table).
    if (auto *object = cache.GetOrCreate(section))
    {
      auto role = object->Role();
      if (!object->IsGroup() && role != AccessibilityRole::Unknown && role != AccessibilityRole::Ignored)
      {
        return true;
      }
    }
    return false;
  }

  bool IsDataTableWithTraversal(html::HTMLTableElement &table, AXObjectCache &cache) noexcept
  {
    bool didTopSectionCheck = false;
    auto topSectionIndicatesLayoutTable = [&](html::HTMLTableSectionElement *section)
    {
      if (didTopSectionCheck || !section)
      {
        return false;
      }
      didTopSectionCheck = true;
      return TableSectionIndicatesAccessibleTable(*section, cache);
    };

    // Store the background color of the table to check against cell's background colors.
    Color tableBackgroundColor = Color::white;
    bool tableHasBorderSpacing = false;
    if (const auto *tableStyle = StyleFrom(table))
    {
      tableBackgroundColor = BackgroundColorOf(*tableStyle);
      auto usedZoom = tableStyle->UsedZoomForLength();
      tableHasBorderSpacing =
        style::Evaluate<LayoutUnit>(tableStyle->GetBorderSpacing().Width(), usedZoom) > 0
        && style::Evaluate<LayoutUnit>(tableStyle->GetBorderSpacing().Height(), usedZoom) > 0;
    }

    uint32 cellCount = 0;
    uint32 borderedCellCount = 0;
    uint32 backgroundDifferenceCellCount = 0;
    uint32 cellsWithTopBorder = 0;
    uint32 cellsWithBottomBorder = 0;
    uint32 cellsWithLeftBorder = 0;
    uint32 cellsWithRightBorder = 0;

    HashMap<const dom::Node *, uint32> cellCountForEachRow;
    std::array<Color, 5> alternatingRowColors;
    uint32 alternatingRowColorCount = 0;
    uint32 rowCount = 0;
    uint32 maxColumnCount = 0;

    auto isDataTableBasedOnRowColumnCount = [&]
    {
      // If there are at least 20 rows, we'll call it a data table.
      return (rowCount >= 20 && maxColumnCount >= 2) || (rowCount >= 2 && maxColumnCount >= 20);
    };

    bool firstColumnHasAllHeaderCells = true;
    html::HTMLTableRowElement *firstRow = nullptr;
    html::HTMLTableSectionElement *firstBody = nullptr;
    html::HTMLTableSectionElement *firstFoot = nullptr;

    // Do a breadth-first search to determine if this is a data table.
    List<dom::Element *> elementsToVisit {&table};
    for (size_t next = 0; next < elementsToVisit.size(); ++next)
    {
      auto *currentParent = elementsToVisit[next];
      bool rowIsAllTableHeaderCells = true;
      for (auto *currentElement = currentParent->FirstElementChild().get(); currentElement;
           currentElement = currentElement->NextElementSibling().get())
      {
        if (auto *section = DynamicDowncast<html::HTMLTableSectionElement>(*currentElement))
        {
          auto tagName = TagNameOf(*section);
          if (tagName == TagName::thead)
          {
            if (topSectionIndicatesLayoutTable(section))
            {
              return false;
            }
          }
          else if (tagName == TagName::tbody)
          {
            firstBody = firstBody ? firstBody : section;
          }
          else
          {
            krys_debug_assert(tagName == TagName::tfoot);
            firstFoot = firstFoot ? firstFoot : section;
          }
        }
        else if (auto *row = DynamicDowncast<html::HTMLTableRowElement>(*currentElement))
        {
          firstRow = firstRow ? firstRow : row;

          rowCount += 1;
          if (isDataTableBasedOnRowColumnCount())
          {
            return true;
          }

          if (IntegralAttribute(*row, u8"aria-rowindex") >= 1 || IntegralAttribute(*row, u8"aria-colindex")
              || HasNonEmptyAttribute(*row, u8"aria-rowindextext") || HasRole(*row, u8"row"))
          {
            return true;
          }

          // For the first 5 rows, cache the background color so we can check if this table has zebra-striped
          // rows.
          if (alternatingRowColorCount < alternatingRowColors.size())
          {
            if (const auto *rowStyle = StyleFrom(*row))
            {
              alternatingRowColors[alternatingRowColorCount] = BackgroundColorOf(*rowStyle);
              ++alternatingRowColorCount;
            }
          }
        }
        else if (auto *cell = DynamicDowncast<html::HTMLTableCellElement>(*currentElement))
        {
          ++cellCount;

          bool isTHCell = TagNameOf(*cell) == TagName::th;
          if (!isTHCell && rowIsAllTableHeaderCells)
          {
            rowIsAllTableHeaderCells = false;
          }
          if (const auto *parentNode = cell->ParentNode())
          {
            auto iterator = cellCountForEachRow.find(parentNode);
            if (iterator == cellCountForEachRow.end())
            {
              // If we don't have an entry for this parent yet, it must be the first column.
              if (!isTHCell && firstColumnHasAllHeaderCells)
              {
                firstColumnHasAllHeaderCells = false;
              }
              iterator = cellCountForEachRow.emplace(parentNode, 0).first;
            }
            iterator->second += 1;
            maxColumnCount = std::max(iterator->second, maxColumnCount);
            if (isDataTableBasedOnRowColumnCount())
            {
              return true;
            }
          }

          // In this case, the developer explicitly assigned a "data" table attribute.
          if (!cell->Headers().empty() || !cell->Abbr().empty() || !cell->Axis().empty()
              || !cell->Scope().empty() || HasCellARIARole(*cell))
          {
            return true;
          }

          // If the author has used ARIA to specify a valid column or row index or index text, assume they
          // want us to treat the table as a data table.
          if (IntegralAttribute(*cell, u8"aria-colindex") >= 1
              || IntegralAttribute(*cell, u8"aria-rowindex") >= 1
              || HasNonEmptyAttribute(*cell, u8"aria-colindextext")
              || HasNonEmptyAttribute(*cell, u8"aria-rowindextext"))
          {
            return true;
          }

          // If the author has used ARIA to specify a column or row span, we're supposed to ignore the value
          // for the purposes of exposing the span. But assume they want us to treat the table as a data
          // table.
          if (IntegralAttribute(*cell, u8"aria-colspan") >= 1
              || IntegralAttribute(*cell, u8"aria-rowspan") >= 1)
          {
            return true;
          }

          Color cellColor = Color::white;
          if (const auto *cellStyle = StyleFrom(*cell))
          {
            if (cellStyle->EmptyCells() == EmptyCell::Hide)
            {
              // If the empty-cells style is set, we'll call it a data table.
              return true;
            }
            cellColor = BackgroundColorOf(*cellStyle);
          }

          if (auto *cellRenderer = DynamicDowncast<RenderBlock>(cell->Renderer()))
          {
            bool hasBorderTop = cellRenderer->BorderTop() > 0;
            bool hasBorderBottom = cellRenderer->BorderBottom() > 0;
            bool hasBorderLeft = cellRenderer->BorderLeft() > 0;
            bool hasBorderRight = cellRenderer->BorderRight() > 0;
            // If a cell has matching bordered sides, call it a (fully) bordered cell.
            if ((hasBorderTop && hasBorderBottom) || (hasBorderLeft && hasBorderRight))
            {
              ++borderedCellCount;
            }

            // Also keep track of each individual border, so we can catch tables where most cells have a
            // bottom border, for example.
            cellsWithTopBorder += hasBorderTop;
            cellsWithBottomBorder += hasBorderBottom;
            cellsWithLeftBorder += hasBorderLeft;
            cellsWithRightBorder += hasBorderRight;
          }

          // If the cell has a different color from the table and there is cell spacing, then it is probably a
          // data table cell (spacing and colors take the place of borders).
          if (tableHasBorderSpacing && tableBackgroundColor != cellColor && !cellColor.IsOpaque())
          {
            ++backgroundDifferenceCellCount;
          }

          // If we've found 10 "good" cells, we don't need to keep searching.
          if (borderedCellCount >= 10 || backgroundDifferenceCellCount >= 10)
          {
            return true;
          }
        }
        else if (Is<html::HTMLTableElement>(*currentElement))
        {
          // Don't descend into nested tables.
          continue;
        }
        elementsToVisit.push_back(currentElement);
      }

      // If the first row of a multi-row table is comprised of all <th> tags, assume it is a data table.
      if (firstRow && currentParent == firstRow && rowIsAllTableHeaderCells && rowCount >= 2)
      {
        auto iterator = cellCountForEachRow.find(currentParent);
        if (iterator != cellCountForEachRow.end() && iterator->second >= 1)
        {
          return true;
        }
      }
    }

    // If there is less than two valid cells, it's not a data table.
    if (cellCount <= 1)
    {
      return false;
    }

    if (topSectionIndicatesLayoutTable(firstBody) || topSectionIndicatesLayoutTable(firstFoot))
    {
      return false;
    }

    if (firstColumnHasAllHeaderCells && rowCount >= 2)
    {
      return true;
    }

    // At least half of the cells had borders, it's a data table.
    auto neededCellCount = cellCount / 2;
    if (borderedCellCount >= neededCellCount || cellsWithTopBorder >= neededCellCount
        || cellsWithBottomBorder >= neededCellCount || cellsWithLeftBorder >= neededCellCount
        || cellsWithRightBorder >= neededCellCount)
    {
      return true;
    }

    // At least half of the cells had different background colors, it's a data table.
    if (backgroundDifferenceCellCount >= neededCellCount)
    {
      return true;
    }

    if (isDataTableBasedOnRowColumnCount())
    {
      return true;
    }

    // Check if there is an alternating row background color indicating a zebra striped style pattern.
    if (alternatingRowColorCount > 2)
    {
      const auto &firstColor = alternatingRowColors[0];
      for (uint32 k = 1; k < alternatingRowColorCount; ++k)
      {
        // If an odd row was the same color as the first row, it's not alternating.
        if (k % 2 == 1 && alternatingRowColors[k] == firstColor)
        {
          return false;
        }
        // If an even row is not the same as the first row, it's not alternating.
        if (!(k % 2) && alternatingRowColors[k] != firstColor)
        {
          return false;
        }
      }
      return true;
    }
    return false;
  }
}
