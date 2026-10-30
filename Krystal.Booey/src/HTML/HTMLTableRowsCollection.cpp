#include "Krystal.Booey/HTML/HTMLTableRowsCollection.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/HTMLTableRowElement.hpp"
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

  HTMLTableRowsCollection::HTMLTableRowsCollection(HTMLTableElement &table) noexcept : _table(ShareRef(table))
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

  RefPtr<dom::Element> HTMLTableRowsCollection::Item(size_t index) noexcept
  {
    auto *row = RowAfter(*_table, nullptr);
    for (size_t i = 0; row && i < index; ++i)
    {
      row = RowAfter(*_table, row);
    }
    return ShareRefPtr<dom::Element>(row);
  }

  RefPtr<const dom::Element> HTMLTableRowsCollection::Item(size_t index) const noexcept
  {
    return const_cast<HTMLTableRowsCollection *>(this)->Item(index);
  }

  RefPtr<dom::Element> HTMLTableRowsCollection::operator[](size_t index) noexcept
  {
    return Item(index);
  }

  RefPtr<const dom::Element> HTMLTableRowsCollection::operator[](size_t index) const noexcept
  {
    return Item(index);
  }

  RefPtr<dom::Element> HTMLTableRowsCollection::NamedItem(dom::DOMStringView name) noexcept
  {
    if (name.empty())
    {
      return nullptr;
    }

    for (auto *row = RowAfter(*_table, nullptr); row; row = RowAfter(*_table, row))
    {
      if (row->Id() == name || dom::ElementAlgorithms::GetAttributeValue(*row, u8"name") == name)
      {
        return ShareRefPtr<dom::Element>(row);
      }
    }
    return nullptr;
  }

  RefPtr<const dom::Element> HTMLTableRowsCollection::NamedItem(dom::DOMStringView name) const noexcept
  {
    return const_cast<HTMLTableRowsCollection *>(this)->NamedItem(name);
  }

  RefPtr<dom::Element> HTMLTableRowsCollection::operator[](dom::DOMStringView name) noexcept
  {
    return NamedItem(name);
  }

  RefPtr<const dom::Element> HTMLTableRowsCollection::operator[](dom::DOMStringView name) const noexcept
  {
    return NamedItem(name);
  }

  size_t HTMLTableRowsCollection::Length() const noexcept
  {
    size_t length = 0uz;
    for (auto *row = RowAfter(*_table, nullptr); row; row = RowAfter(*_table, row))
    {
      ++length;
    }
    return length;
  }

  List<dom::DOMString> HTMLTableRowsCollection::SupportedPropertyNames() const noexcept
  {
    List<dom::DOMString> supportedNames;
    for (auto *row = RowAfter(*_table, nullptr); row; row = RowAfter(*_table, row))
    {
      auto id = row->Id();
      if (!id.empty() && !std::ranges::contains(supportedNames, id))
      {
        supportedNames.push_back(id);
      }

      auto name = dom::ElementAlgorithms::GetAttributeValue(*row, u8"name");
      if (!name.empty() && !std::ranges::contains(supportedNames, name))
      {
        supportedNames.push_back(name);
      }
    }
    return supportedNames;
  }
}
