/*
 * Copyright (C) 2010 Google, Inc. All rights reserved.
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

#include "Krystal.Booey/HTML/Parser/FormattingElementList.hpp"

namespace krys::boo::html
{
  void FormattingElementList::PushMarker() noexcept
  {
    _formattingElements.emplace_back(FormattingListEntry {});
  }

  void FormattingElementList::PushElement(StackItem &&item) noexcept
  {
    // 1. If there are already three elements in the list of active formatting elements after the last marker,
    //    if any, or anywhere in the list if there are no markers, that have the same tag name, namespace, and
    //    attributes as element, then remove the earliest such element from the list of active formatting
    //    elements. For these purposes, the attributes must be compared as they were on the tokens for which
    //    the elements were created; two elements have the same attributes if all their parsed attributes can
    //    be paired such that the two attributes in each pair have identical names, namespaces, and values
    //    (the order of the attributes does not matter).
    // (The stack item keeps the token's attributes, so attributes the sanitizer removed still count.)

    auto lastMarkerIt = std::find_if(_formattingElements.rbegin(), _formattingElements.rend(),
                                     [](const auto &entry) { return entry.IsMarker(); });
    auto lastMarkerIndex =
      (lastMarkerIt != _formattingElements.rend())
        ? static_cast<size_t>(std::distance(lastMarkerIt, _formattingElements.rend())) - 1uz
        : 0uz;

    SmallList<decltype(_formattingElements.begin())> identicalFormattingElements;
    for (auto it = _formattingElements.begin() + lastMarkerIndex; it != _formattingElements.end(); ++it)
    {
      if (!it->IsFormattingElement())
      {
        continue;
      }

      auto &entryItem = it->Item();
      if (entryItem.TagName() != item.TagName() || entryItem.Namespace() != item.Namespace())
      {
        continue;
      }

      auto &entryAttrs = entryItem.Attributes();
      if (item.Attributes().size() != entryAttrs.size())
      {
        continue;
      }

      if (entryAttrs.empty() && item.Attributes().empty())
      {
        identicalFormattingElements.push_back(it);
        continue;
      }

      if (std::ranges::all_of(entryAttrs,
                              [&](const auto &attr)
                              {
                                return std::ranges::any_of(
                                  item.Attributes(), [&](const auto &a)
                                  { return a.Name == attr.Name && a.Value == attr.Value; });
                              }))
      {
        identicalFormattingElements.push_back(it);
      }
    }

    if (identicalFormattingElements.size() == 3)
    {
      _formattingElements.erase(identicalFormattingElements.front());
    }

    // 2. Add element to the list of active formatting elements.
    _formattingElements.emplace_back(FormattingListEntry(krys::move(item)));
  }

  StackItem *FormattingElementList::FindFromLastMarker(TagName tagName, Namespace tagNamespace) noexcept
  {
    for (auto it = _formattingElements.rbegin(); it != _formattingElements.rend(); ++it)
    {
      if (it->IsMarker())
      {
        return nullptr;
      }

      auto &item = it->Item();
      if (item.TagName() == tagName && item.Namespace() == tagNamespace)
      {
        return &it->Item();
      }
    }

    return nullptr;
  }

  bool FormattingElementList::Contains(const dom::Element &node) const noexcept
  {
    return std::ranges::any_of(_formattingElements, [&](const FormattingListEntry &entry)
                               { return entry.IsFormattingElement() && &entry.Item().Element() == &node; });
  }

  FormattingListEntry *FormattingElementList::Find(const dom::Element &node) noexcept
  {
    auto it =
      std::ranges::find_if(_formattingElements, [&](const FormattingListEntry &entry)
                           { return entry.IsFormattingElement() && &entry.Item().Element() == &node; });

    return it != _formattingElements.end() ? &*it : nullptr;
  }

  void FormattingElementList::Remove(const dom::Element &node) noexcept
  {
    auto it =
      std::ranges::find_if(_formattingElements, [&](const FormattingListEntry &entry)
                           { return entry.IsFormattingElement() && &entry.Item().Element() == &node; });

    if (it != _formattingElements.end())
    {
      _formattingElements.erase(it);
    }
  }

  void FormattingElementList::ClearUpToLastMarker() noexcept
  {
    while (true)
    {
      if (_formattingElements.empty())
      {
        return;
      }

      auto wasMarker = _formattingElements.back().IsMarker();
      _formattingElements.pop_back();

      if (wasMarker)
      {
        return;
      }
    }
  }

#pragma region Bookmark

  FormattingElementList::Bookmark FormattingElementList::BookmarkFor(const dom::Element &node) noexcept
  {
    for (size_t i = 0uz; i < _formattingElements.size(); ++i)
    {
      const auto &entry = _formattingElements[i];
      if (entry.IsFormattingElement() && &entry.Item().Element() == &node)
      {
        return Bookmark {i};
      }
    }

    krys_debug_assert_msg(false, "node not found in formatting list");
    return Bookmark {0uz};
  }

  void FormattingElementList::MoveBookmarkAfter(Bookmark &bookmark, const FormattingListEntry &entry) noexcept
  {
    for (size_t i = 0uz; i < _formattingElements.size(); ++i)
    {
      if (&_formattingElements[i] == &entry)
      {
        bookmark.index = i + 1uz;
        return;
      }
    }
  }

  void FormattingElementList::RemoveAndUpdateBookmark(const dom::Element &node, Bookmark &bookmark) noexcept
  {
    auto it =
      std::ranges::find_if(_formattingElements, [&](const FormattingListEntry &entry)
                           { return entry.IsFormattingElement() && &entry.Item().Element() == &node; });

    if (it == _formattingElements.end())
    {
      return;
    }

    auto removeIndex = static_cast<size_t>(std::distance(_formattingElements.begin(), it));

    _formattingElements.erase(it);

    if (removeIndex < bookmark.index)
    {
      --bookmark.index;
    }
  }

  void FormattingElementList::InsertAtBookmark(StackItem &&newItem, const Bookmark &bookmark) noexcept
  {
    auto insertIt = _formattingElements.begin() + static_cast<ptrdiff_t>(bookmark.index);
    _formattingElements.insert(insertIt, FormattingListEntry(krys::move(newItem)));
  }

#pragma endregion
}
