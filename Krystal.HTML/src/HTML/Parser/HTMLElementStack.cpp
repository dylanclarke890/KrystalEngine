#include "Krystal.HTML/HTML/Parser/HTMLElementStack.hpp"

namespace Krys::HTML
{
#pragma region Accessors

  KRYS_NODISCARD HTMLStackItem &HTMLElementStack::Top() noexcept
  {
    assert(!_items.empty());
    return _items.front();
  }

  RawPtr<HTMLStackItem> HTMLElementStack::OneBelowTop() noexcept
  {
    if (_items.size() < 2uz)
    {
      return nullptr;
    }

    return &_items[1uz];
  }

  KRYS_NODISCARD HTMLStackItem &HTMLElementStack::Bottom() noexcept
  {
    assert(!_items.empty());
    return _items.back();
  }

  KRYS_NODISCARD const HTMLStackItem &HTMLElementStack::Bottom() const noexcept
  {
    assert(!_items.empty());
    return _items.back();
  }

  KRYS_NODISCARD HTMLStackItem &HTMLElementStack::operator[](size_t index) noexcept
  {
    assert(index < _items.size());
    return _items[index];
  }

#pragma endregion

#pragma region Pop

  void HTMLElementStack::Pop() noexcept
  {
    assert(!_items.empty());

    // TODO(HTMLTREEBUILDER, HTML): When the current node is removed from the stack of open elements,
    // process internal resource links given the current node's node document.

    _items.pop_back();
  }

  void HTMLElementStack::PopUntil(const ContainerNode &node) noexcept
  {
    while (&Bottom().Node() != &node)
    {
      Pop();
    }
  }

  void HTMLElementStack::PopUntil(TagName tagName) noexcept
  {
    while (Bottom().TagName() != tagName)
    {
      Pop();
    }
  }

  void HTMLElementStack::PopUntilPopped(const ContainerNode &node) noexcept
  {
    while (true)
    {
      bool isTarget = (&Bottom().Node() == &node);
      Pop();

      if (isTarget)
      {
        break;
      }
    }
  }

  void HTMLElementStack::PopUntilPopped(TagName tagName) noexcept
  {
    while (true)
    {
      bool isTarget = (Bottom().TagName() == tagName);
      Pop();

      if (isTarget)
      {
        break;
      }
    }
  }

  void HTMLElementStack::PopUntilTableContext() noexcept
  {
    while (true)
    {
      switch (Bottom().TagName())
      {
        case TagName::table:
        case TagName::template_:
        case TagName::html:
        {
          return;
        }
        default:
        {
          Pop();
          continue;
        }
      }
    }
  }

  void HTMLElementStack::PopUntilTableBodyContext() noexcept
  {
    while (true)
    {
      switch (Bottom().TagName())
      {
        case TagName::tbody:
        case TagName::tfoot:
        case TagName::thead:
        case TagName::template_:
        case TagName::html:
        {
          return;
        }
        default:
        {
          Pop();
          continue;
        }
      }
    }
  }

  void HTMLElementStack::PopUntilTableRowContext() noexcept
  {
    while (true)
    {
      switch (Bottom().TagName())
      {
        case TagName::tr:
        case TagName::template_:
        case TagName::html:
        {
          return;
        }
        default:
        {
          Pop();
          continue;
        }
      }
    }
  }

  void HTMLElementStack::PopAll() noexcept
  {
    while (!_items.empty())
    {
      Pop();
    }
  }

#pragma endregion

#pragma region InScope

  bool HTMLElementStack::HasElementInScope(TagName targetNode) const noexcept
  {
    auto *node = &Bottom();

    while (true)
    {
      if (node->TagName() == targetNode)
      {
        return true;
      }

      switch (node->TagName())
      {
        case TagName::applet:
        case TagName::caption:
        case TagName::html:
        case TagName::table:
        case TagName::td:
        case TagName::th:
        case TagName::marquee:
        case TagName::object:
        case TagName::select:
        case TagName::template_:
        {
          // TODO(HTMLTREEBUILDER, HTML) also: MathML mi MathML mo MathML mn MathML ms MathML mtext MathML
          // annotation-xml SVG foreignObject SVG desc SVG title
          return false;
        }
        default:
        {
          break;
        }
      }

      node = EntryBefore(node->Node());
    }

    return false;
  }

  bool HTMLElementStack::HasElementInListItemScope(TagName targetNode) const noexcept
  {
    auto *node = &Bottom();

    while (true)
    {
      if (node->TagName() == targetNode)
      {
        return true;
      }

      switch (node->TagName())
      {
        case TagName::applet:
        case TagName::caption:
        case TagName::html:
        case TagName::table:
        case TagName::td:
        case TagName::th:
        case TagName::li:
        case TagName::marquee:
        case TagName::object:
        case TagName::select:
        case TagName::template_:
        case TagName::ul:
        {
          // TODO(HTMLTREEBUILDER, HTML) also: MathML mi MathML mo MathML mn MathML ms MathML mtext MathML
          // annotation-xml SVG foreignObject SVG desc SVG title
          return false;
        }
        default:
        {
          break;
        }
      }

      node = EntryBefore(node->Node());
    }

    return false;
  }

  bool HTMLElementStack::HasElementInButtonScope(TagName targetNode) const noexcept
  {
    auto *node = &Bottom();

    while (true)
    {
      if (node->TagName() == targetNode)
      {
        return true;
      }

      switch (node->TagName())
      {
        case TagName::applet:
        case TagName::button:
        case TagName::caption:
        case TagName::html:
        case TagName::table:
        case TagName::td:
        case TagName::th:
        case TagName::marquee:
        case TagName::object:
        case TagName::select:
        case TagName::template_:
        {
          // TODO(HTMLTREEBUILDER, HTML) also: MathML mi MathML mo MathML mn MathML ms MathML mtext MathML
          // annotation-xml SVG foreignObject SVG desc SVG title
          return false;
        }
        default:
        {
          break;
        }
      }

      node = EntryBefore(node->Node());
    }

    return false;
  }

  bool HTMLElementStack::HasElementInTableScope(TagName targetNode) const noexcept
  {
    auto *node = &Bottom();

    while (true)
    {
      if (node->TagName() == targetNode)
      {
        return true;
      }

      switch (node->TagName())
      {
        case TagName::html:
        case TagName::table:
        case TagName::template_:
        {
          return false;
        }
        default:
        {
          break;
        }
      }

      node = EntryBefore(node->Node());
    }

    return false;
  }

#pragma endregion

  bool HTMLElementStack::Contains(const Element &node) const noexcept
  {
    return std::ranges::any_of(_items,
                               [&](const auto &item) { return item.IsElement() && &item.Node() == &node; });
  }
}