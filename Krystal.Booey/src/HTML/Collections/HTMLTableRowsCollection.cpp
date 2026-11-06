#include "Krystal.Booey/HTML/Collections/HTMLTableRowsCollection.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableRowElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include <algorithm>

namespace krys::boo::html
{
  namespace
  {
    KRYS_NODISCARD bool TableRowsIsTableSection(const dom::Node &node, TagName sectionTag) noexcept
    {
      auto *element = DynamicDowncast<HTMLElement>(node);
      return element && element->HasTagName(sectionTag);
    }

    // Because the parent is a table or a table section, it's an HTML element.
    KRYS_NODISCARD bool TableRowsIsInSection(HTMLTableRowElement &row, TagName sectionTag) noexcept
    {
      return TableRowsIsTableSection(*row.ParentNode(), sectionTag);
    }

    KRYS_NODISCARD dom::Element *TableRowsFirstElementChild(dom::ContainerNode &node) noexcept
    {
      for (auto *child = node.FirstChild(); child; child = child->NextSibling())
      {
        if (auto *element = DynamicDowncast<dom::Element>(*child))
        {
          return element;
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD dom::Element *TableRowsLastElementChild(dom::ContainerNode &node) noexcept
    {
      for (auto *child = node.LastChild(); child; child = child->PreviousSibling())
      {
        if (auto *element = DynamicDowncast<dom::Element>(*child))
        {
          return element;
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD dom::Element *TableRowsNextElementSibling(dom::Node &node) noexcept
    {
      for (auto *sibling = node.NextSibling(); sibling; sibling = sibling->NextSibling())
      {
        if (auto *element = DynamicDowncast<dom::Element>(*sibling))
        {
          return element;
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD dom::Element *TableRowsPreviousElementSibling(dom::Node &node) noexcept
    {
      for (auto *sibling = node.PreviousSibling(); sibling; sibling = sibling->PreviousSibling())
      {
        if (auto *element = DynamicDowncast<dom::Element>(*sibling))
        {
          return element;
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD HTMLTableRowElement *TableRowsFirstRowChild(dom::ContainerNode &node) noexcept
    {
      for (auto *child = node.FirstChild(); child; child = child->NextSibling())
      {
        if (auto *row = AsTableElement<HTMLTableRowElement>(child))
        {
          return row;
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD HTMLTableRowElement *TableRowsLastRowChild(dom::ContainerNode &node) noexcept
    {
      for (auto *child = node.LastChild(); child; child = child->PreviousSibling())
      {
        if (auto *row = AsTableElement<HTMLTableRowElement>(child))
        {
          return row;
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD HTMLTableRowElement *TableRowsNextRowSibling(HTMLTableRowElement &row) noexcept
    {
      for (auto *sibling = row.NextSibling(); sibling; sibling = sibling->NextSibling())
      {
        if (auto *nextRow = AsTableElement<HTMLTableRowElement>(sibling))
        {
          return nextRow;
        }
      }
      return nullptr;
    }
  }

  HTMLTableRowsCollection::HTMLTableRowsCollection(HTMLTableElement &table) noexcept
      : LiveHTMLCollection(table, [](const dom::Element &) { return true; }), _table(ShareRef(table))
  {
  }

  HTMLTableRowElement *HTMLTableRowsCollection::RowAfter(HTMLTableElement &table,
                                                         HTMLTableRowElement *previous) noexcept
  {
    // Start by looking for the next row in this section. Continue only if there is none.
    if (previous && previous->ParentNode() != &table)
    {
      if (auto *row = TableRowsNextRowSibling(*previous))
      {
        return row;
      }
    }

    dom::Element *child = nullptr;

    // If still looking at head sections, find the first row in the next head section.
    if (!previous)
    {
      child = TableRowsFirstElementChild(table);
    }
    else if (TableRowsIsInSection(*previous, TagName::thead))
    {
      child = TableRowsNextElementSibling(*previous->ParentNode());
    }
    for (; child; child = TableRowsNextElementSibling(*child))
    {
      if (TableRowsIsTableSection(*child, TagName::thead))
      {
        if (auto *row = TableRowsFirstRowChild(*child))
        {
          return row;
        }
      }
    }

    // If still looking at top level and bodies, find the next row in top level or the first in the next body
    // section.
    if (!previous || TableRowsIsInSection(*previous, TagName::thead))
    {
      child = TableRowsFirstElementChild(table);
    }
    else if (previous->ParentNode() == &table)
    {
      child = TableRowsNextElementSibling(*previous);
    }
    else if (TableRowsIsInSection(*previous, TagName::tbody))
    {
      child = TableRowsNextElementSibling(*previous->ParentNode());
    }
    for (; child; child = TableRowsNextElementSibling(*child))
    {
      if (auto *row = AsTableElement<HTMLTableRowElement>(child))
      {
        return row;
      }
      if (TableRowsIsTableSection(*child, TagName::tbody))
      {
        if (auto *row = TableRowsFirstRowChild(*child))
        {
          return row;
        }
      }
    }

    // Find the first row in the next foot section.
    if (!previous || !TableRowsIsInSection(*previous, TagName::tfoot))
    {
      child = TableRowsFirstElementChild(table);
    }
    else
    {
      child = TableRowsNextElementSibling(*previous->ParentNode());
    }
    for (; child; child = TableRowsNextElementSibling(*child))
    {
      if (TableRowsIsTableSection(*child, TagName::tfoot))
      {
        if (auto *row = TableRowsFirstRowChild(*child))
        {
          return row;
        }
      }
    }

    return nullptr;
  }

  HTMLTableRowElement *HTMLTableRowsCollection::LastRow(HTMLTableElement &table) noexcept
  {
    for (auto *child = TableRowsLastElementChild(table); child;
         child = TableRowsPreviousElementSibling(*child))
    {
      if (TableRowsIsTableSection(*child, TagName::tfoot))
      {
        if (auto *row = TableRowsLastRowChild(*child))
        {
          return row;
        }
      }
    }

    for (auto *child = TableRowsLastElementChild(table); child;
         child = TableRowsPreviousElementSibling(*child))
    {
      if (auto *row = AsTableElement<HTMLTableRowElement>(child))
      {
        return row;
      }
      if (TableRowsIsTableSection(*child, TagName::tbody))
      {
        if (auto *row = TableRowsLastRowChild(*child))
        {
          return row;
        }
      }
    }

    for (auto *child = TableRowsLastElementChild(table); child;
         child = TableRowsPreviousElementSibling(*child))
    {
      if (TableRowsIsTableSection(*child, TagName::thead))
      {
        if (auto *row = TableRowsLastRowChild(*child))
        {
          return row;
        }
      }
    }

    return nullptr;
  }

  dom::Element *HTMLTableRowsCollection::CollectionBegin() const noexcept
  {
    return RowAfter(*_table, nullptr);
  }

  dom::Element *HTMLTableRowsCollection::CollectionNext(dom::Element &element) const noexcept
  {
    return RowAfter(*_table, &static_cast<HTMLTableRowElement &>(element));
  }
}
