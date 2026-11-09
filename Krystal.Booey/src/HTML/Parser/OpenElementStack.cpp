/*
 * Copyright (C) 2010 Google, Inc. All rights reserved.
 * Copyright (C) 2011-2024 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY GOOGLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL GOOGLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/HTML/Parser/OpenElementStack.hpp"
#include "Krystal.Booey/HTML/Algorithms/FormControlAlgorithms.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"

namespace krys::boo::html
{
#pragma region Accessors

  KRYS_NODISCARD StackItem &OpenElementStack::Top() noexcept
  {
    krys_debug_assert(!_items.empty());
    return _items.front();
  }

  StackItem *OpenElementStack::OneBelowTop() noexcept
  {
    if (_items.size() < 2uz)
    {
      return nullptr;
    }

    return &_items[1uz];
  }

  KRYS_NODISCARD StackItem &OpenElementStack::Bottom() noexcept
  {
    krys_debug_assert(!_items.empty());
    return _items.back();
  }

  KRYS_NODISCARD const StackItem &OpenElementStack::Bottom() const noexcept
  {
    krys_debug_assert(!_items.empty());
    return _items.back();
  }

  KRYS_NODISCARD StackItem &OpenElementStack::operator[](size_t index) noexcept
  {
    krys_debug_assert(index < _items.size());
    return _items[index];
  }

#pragma endregion

#pragma region Pop

  void OpenElementStack::Pop() noexcept
  {
    krys_debug_assert(!_items.empty());

    // FIXME: When the current node is removed from the stack of open elements,
    // process internal resource links given the current node's node document.

    Ref<dom::Element> back = ShareRef(Bottom().Element());
    _items.pop_back();

    back->FinishParsingChildren();

    // An option's children come after its post-connection steps, so a popped option updates its select's
    // selectedcontent elements again.
    if (auto *option = DynamicDowncast<HTMLOptionElement>(back.get()))
    {
      FormControlAlgorithms::UpdateDescendantSelectedContentsForOption(*option);
    }
  }

  void OpenElementStack::PopUntil(const dom::Element &node) noexcept
  {
    while (&Bottom().Element() != &node)
    {
      Pop();
    }
  }

  void OpenElementStack::PopUntil(TagName tagName, Namespace tagNamespace) noexcept
  {
    while (!(Bottom().Namespace() == tagNamespace && Bottom().TagName() == tagName))
    {
      Pop();
    }
  }

  void OpenElementStack::PopUntilPopped(const dom::Element &node) noexcept
  {
    while (true)
    {
      bool isTarget = (&Bottom().Element() == &node);
      Pop();

      if (isTarget)
      {
        break;
      }
    }
  }

  void OpenElementStack::PopUntilPopped(TagName tagName, Namespace tagNamespace) noexcept
  {
    while (true)
    {
      bool isTarget = (Bottom().Namespace() == tagNamespace && Bottom().TagName() == tagName);
      Pop();

      if (isTarget)
      {
        break;
      }
    }
  }

  void OpenElementStack::PopUntilTableContext() noexcept
  {
    while (true)
    {
      switch (Bottom().TagName())
      {
        case TagName::table:
        case TagName::template_:
        case TagName::html:
        {
          if (Bottom().Namespace() == Namespace::HTML)
          {
            return;
          }
          break;
        }
      }

      Pop();
    }
  }

  void OpenElementStack::PopUntilTableBodyContext() noexcept
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
          if (Bottom().Namespace() == Namespace::HTML)
          {
            return;
          }

          break;
        }
      }

      Pop();
    }
  }

  void OpenElementStack::PopUntilTableRowContext() noexcept
  {
    while (true)
    {
      switch (Bottom().TagName())
      {
        case TagName::tr:
        case TagName::template_:
        case TagName::html:
        {
          if (Bottom().Namespace() == Namespace::HTML)
          {
            return;
          }

          break;
        }
      }

      Pop();
    }
  }

  void OpenElementStack::PopAll() noexcept
  {
    while (!_items.empty())
    {
      Pop();
    }
  }

  void OpenElementStack::GenerateImpliedEndTags(Maybe<TagName> except) noexcept
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

  void OpenElementStack::GenerateImpliedEndTagsThoroughly() noexcept
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

  bool OpenElementStack::IsScopeMarker(const StackItem &item) const noexcept
  {
    switch (item.Namespace())
    {
      case Namespace::HTML:
      {
        switch (item.TagName())
        {
          case TagName::applet:
          case TagName::caption:
          case TagName::html:
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

        return false;
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

        return false;
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

        return false;
      }
    }

    return false;
  }

  bool OpenElementStack::IsListItemScopeMarker(const StackItem &item) const noexcept
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

  bool OpenElementStack::IsButtonScopeMarker(const StackItem &item) const noexcept
  {
    if (IsScopeMarker(item))
    {
      return true;
    }

    return item.Namespace() == Namespace::HTML && item.TagName() == TagName::button;
  }

  bool OpenElementStack::IsTableScopeMarker(const StackItem &item) const noexcept
  {
    if (item.Namespace() != Namespace::HTML)
    {
      return false;
    }

    return item.TagName() == TagName::table || item.TagName() == TagName::template_
           || item.TagName() == TagName::html;
  }

  bool OpenElementStack::IsTableBodyScopeMarker(const StackItem &item) const noexcept
  {
    if (item.Namespace() != Namespace::HTML)
    {
      return false;
    }

    return item.TagName() == TagName::tbody || item.TagName() == TagName::tfoot
           || item.TagName() == TagName::thead || item.TagName() == TagName::template_
           || item.TagName() == TagName::html;
  }

  bool OpenElementStack::IsTableRowScopeMarker(const StackItem &item) const noexcept
  {
    if (item.Namespace() != Namespace::HTML)
    {
      return false;
    }

    return item.TagName() == TagName::tr || item.TagName() == TagName::template_
           || item.TagName() == TagName::html;
  }

  bool OpenElementStack::HasElementInScope(TagName targetNode) const noexcept
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

  bool OpenElementStack::HasElementInScope(const dom::Element &element) const noexcept
  {
    auto it = _items.rbegin();
    while (it != _items.rend())
    {
      auto &item = *it;

      if (&item.Element() == &element)
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

  bool OpenElementStack::HasElementInListItemScope(TagName targetNode) const noexcept
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

  bool OpenElementStack::HasElementInButtonScope(TagName targetNode) const noexcept
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

  bool OpenElementStack::HasElementInTableScope(TagName targetNode) const noexcept
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

  bool OpenElementStack::Contains(const dom::Element &node) const noexcept
  {
    return std::ranges::any_of(_items, [&](const auto &item) { return &item.Element() == &node; });
  }
}