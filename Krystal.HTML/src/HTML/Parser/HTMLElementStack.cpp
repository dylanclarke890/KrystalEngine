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

  void HTMLElementStack::GenerateImpliedEndTags(Maybe<TagName> except) noexcept
  {
    while (true)
    {
      auto &currentNode = Bottom();
      if (currentNode.Namespace() != Namespace::HTML)
      {
        return;
      }

      switch (currentNode.TagName())
      {
        case TagName::dd:
        case TagName::dt:
        case TagName::li:
        case TagName::optgroup:
        case TagName::option:
        case TagName::p:
        case TagName::rb:
        case TagName::rp:
        case TagName::rt:
        case TagName::rtc:
        {
          if (except.has_value() && currentNode.TagName() == except.value())
          {
            return;
          }

          Pop();
          continue;
        }
      }

      return;
    }
  }

  void HTMLElementStack::GenerateImpliedEndTagsThoroughly() noexcept
  {
    while (true)
    {
      auto &currentNode = Bottom();
      if (currentNode.Namespace() != Namespace::HTML)
      {
        return;
      }

      switch (currentNode.TagName())
      {
        case TagName::caption:
        case TagName::colgroup:
        case TagName::dd:
        case TagName::dt:
        case TagName::li:
        case TagName::optgroup:
        case TagName::option:
        case TagName::p:
        case TagName::rb:
        case TagName::rp:
        case TagName::rt:
        case TagName::rtc:
        case TagName::tbody:
        case TagName::td:
        case TagName::tfoot:
        case TagName::th:
        case TagName::thead:
        case TagName::tr:
        {
          Pop();
          break;
        }
      }

      return;
    }
  }

#pragma endregion

#pragma region InScope

  bool HTMLElementStack::IsScopeMarker(const HTMLStackItem &item) const noexcept
  {
    switch (item.Namespace())
    {
      case Namespace::HTML:
      {
        switch (item.TagName())
        {
          case TagName::applet:
          case TagName::caption:
          case TagName::marquee:
          case TagName::object:
          case TagName::select:
          case TagName::table:
          case TagName::td:
          case TagName::th:
          case TagName::template_:
          {
            return true;
          }
        }
        break;
      }
      case Namespace::MathML:
      {
        switch (item.TagName())
        {
          case TagName::mi:
          case TagName::mo:
          case TagName::mn:
          case TagName::ms:
          case TagName::mtext:
          case TagName::annotation_xml:
          {
            return true;
          }
        }
        break;
      }
      case Namespace::SVG:
      {
        switch (item.TagName())
        {
          case TagName::foreignObject:
          case TagName::desc:
          case TagName::title:
          {
            return true;
          }
        }
        break;
      }
    }

    return item.IsRootNode();
  }

  bool HTMLElementStack::IsListItemScopeMarker(const HTMLStackItem &item) const noexcept
  {
    if (IsScopeMarker(item))
    {
      return true;
    }

    if (item.Namespace() != Namespace::HTML)
    {
      return false;
    }

    return item.TagName() == TagName::ol || item.TagName() == TagName::ul;
  }

  bool HTMLElementStack::IsTableScopeMarker(const HTMLStackItem &item) const noexcept
  {
    if (item.Namespace() != Namespace::HTML)
    {
      return item.IsRootNode();
    }

    return item.TagName() == TagName::table || item.TagName() == TagName::template_ || item.IsRootNode();
  }

  bool HTMLElementStack::IsTableBodyScopeMarker(const HTMLStackItem &item) const noexcept
  {
    if (item.Namespace() != Namespace::HTML)
    {
      return item.IsRootNode();
    }

    return item.TagName() == TagName::tbody || item.TagName() == TagName::tfoot
           || item.TagName() == TagName::thead || item.TagName() == TagName::template_ || item.IsRootNode();
  }

  bool HTMLElementStack::IsTableRowScopeMarker(const HTMLStackItem &item) const noexcept
  {
    if (item.Namespace() != Namespace::HTML)
    {
      return item.IsRootNode();
    }

    return item.TagName() == TagName::tr || item.TagName() == TagName::template_ || item.IsRootNode();
  }

  bool HTMLElementStack::IsButtonScopeMarker(const HTMLStackItem &item) const noexcept
  {
    if (IsScopeMarker(item))
    {
      return true;
    }

    if (item.Namespace() != Namespace::HTML)
    {
      return false;
    }

    return item.TagName() == TagName::button;
  }

  bool HTMLElementStack::IsSelectScopeMarker(const HTMLStackItem &item) const noexcept
  {
    if (item.Namespace() != Namespace::HTML)
    {
      return false;
    }

    return item.TagName() != TagName::optgroup && item.TagName() != TagName::option;
  }

  bool HTMLElementStack::HasElementInScope(TagName targetNode) const noexcept
  {
    auto it = _items.rbegin();
    while (it != _items.rend())
    {
      auto &item = *it;

      if (item.Namespace() == Namespace::HTML && item.TagName() == targetNode)
      {
        return true;
      }

      if (IsScopeMarker(item))
      {
        return false;
      }

      ++it;
    }

    std::unreachable();
    return false;
  }

  bool HTMLElementStack::HasElementInScope(const Element &element) const noexcept
  {
    auto it = _items.rbegin();
    while (it != _items.rend())
    {
      auto &item = *it;
      if (&item.Node() == &element)
      {
        return true;
      }

      if (IsScopeMarker(item))
      {
        return false;
      }

      ++it;
    }

    std::unreachable();
    return false;
  }

  bool HTMLElementStack::HasElementInListItemScope(TagName targetNode) const noexcept
  {
    auto it = _items.rbegin();
    while (it != _items.rend())
    {
      auto &item = *it;
      if (item.Namespace() == Namespace::HTML && item.TagName() == targetNode)
      {
        return true;
      }
      if (IsListItemScopeMarker(item))
      {
        return false;
      }
      ++it;
    }

    std::unreachable();
    return false;
  }

  bool HTMLElementStack::HasElementInButtonScope(TagName targetNode) const noexcept
  {
    auto it = _items.rbegin();
    while (it != _items.rend())
    {
      auto &item = *it;
      if (item.Namespace() == Namespace::HTML && item.TagName() == targetNode)
      {
        return true;
      }

      if (IsButtonScopeMarker(item))
      {
        return false;
      }

      ++it;
    }

    std::unreachable();
    return false;
  }

  bool HTMLElementStack::HasElementInTableScope(TagName targetNode) const noexcept
  {
    auto it = _items.rbegin();
    while (it != _items.rend())
    {
      auto &item = *it;

      if (item.Namespace() == Namespace::HTML && item.TagName() == targetNode)
      {
        return true;
      }

      if (IsTableScopeMarker(item))
      {
        return false;
      }

      ++it;
    }

    std::unreachable();
    return false;
  }

#pragma endregion

  bool HTMLElementStack::Contains(const Element &node) const noexcept
  {
    return std::ranges::any_of(_items,
                               [&](const auto &item) { return item.IsElement() && &item.Node() == &node; });
  }
}