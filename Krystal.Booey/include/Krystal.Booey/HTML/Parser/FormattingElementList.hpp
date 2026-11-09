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

#pragma once

#include "Krystal.Booey/HTML/Parser/OpenElementStack.hpp"
#include "Krystal.Booey/HTML/Parser/StackItem.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::html
{
  class FormattingListEntry
  {
  private:
    Maybe<StackItem> _item;

  public:
    FormattingListEntry() noexcept = default;

    FormattingListEntry(StackItem &&item) noexcept : _item(krys::move(item))
    {
    }

    KRYS_NODISCARD StackItem &Item() noexcept
    {
      krys_debug_assert(_item.has_value());
      return _item.value();
    }

    KRYS_NODISCARD const StackItem &Item() const noexcept
    {
      krys_debug_assert(_item.has_value());
      return _item.value();
    }

    KRYS_NODISCARD bool IsMarker() const noexcept
    {
      return !_item.has_value();
    }

    KRYS_NODISCARD bool IsFormattingElement() const noexcept
    {
      return _item.has_value();
    }

    void ReplaceItem(StackItem newItem) noexcept
    {
      _item = krys::move(newItem);
    }
  };

  /// @see https://html.spec.whatwg.org/multipage/parsing.html#the-list-of-active-formatting-elements
  class FormattingElementList
  {
  private:
    List<FormattingListEntry> _formattingElements;

  public:
    /// @brief Pushes a marker onto the list of active formatting elements.
    /// @see https://html.spec.whatwg.org/multipage/parsing.html#concept-parser-marker
    void PushMarker() noexcept;

    /// @brief Pushes an element onto the list of active formatting elements.
    /// @see https://html.spec.whatwg.org/push-onto-the-list-of-active-formatting-elements
    void PushElement(StackItem &&item) noexcept;

    /// @brief Searches from the end of the list back to the last marker for an element with the given tag
    /// name. Returns a pointer to that stack item, or null if none is found.
    KRYS_NODISCARD StackItem *FindFromLastMarker(TagName tagName, Namespace tagNamespace) noexcept;

    /// @brief Returns a pointer to the formatting entry whose node matches the given node, or nullptr if it
    /// wasn't found.
    KRYS_NODISCARD FormattingListEntry *Find(const dom::Element &node) noexcept;

    /// @brief Returns true if the list contains a formatting entry whose node matches the given node.
    KRYS_NODISCARD bool Contains(const dom::Element &node) const noexcept;

    /// @brief Removes the formatting list entry whose node matches the given node, if present.
    void Remove(const dom::Element &node) noexcept;

    /// @brief Pops elements up to and including the last marker.
    /// @see https://html.spec.whatwg.org/#clear-the-list-of-active-formatting-elements-up-to-the-last-marker
    void ClearUpToLastMarker() noexcept;

    /// @brief Returns true if the list is empty.
    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _formattingElements.empty();
    }

    /// @brief Returns an iterator to the last entry in the list, or end() if the list is empty.
    KRYS_NODISCARD auto Last() noexcept
    {
      if (_formattingElements.empty())
      {
        return _formattingElements.end();
      }

      return std::prev(_formattingElements.end());
    }

    /// @brief Returns an iterator to the first entry in the list.
    KRYS_NODISCARD auto begin() noexcept
    {
      return _formattingElements.begin();
    }

    /// @brief Returns an iterator to the end of the list.
    KRYS_NODISCARD auto end() noexcept
    {
      return _formattingElements.end();
    }

#pragma region Bookmark

    /// @brief Tracks a position in the list for use in the adoption agency algorithm.
    struct Bookmark
    {
      size_t index {0uz};
    };

    /// @brief Returns a bookmark positioned at the entry for the given node.
    KRYS_NODISCARD Bookmark BookmarkFor(const dom::Element &node) noexcept;

    /// @brief Moves the bookmark to the position immediately after the given entry.
    void MoveBookmarkAfter(Bookmark &bookmark, const FormattingListEntry &entry) noexcept;

    /// @brief Removes the entry for node and adjusts bookmark if it falls at or after the removed entry.
    void RemoveAndUpdateBookmark(const dom::Element &node, Bookmark &bookmark) noexcept;

    /// @brief Inserts newItem at the bookmark position.
    void InsertAtBookmark(StackItem &&newItem, const Bookmark &bookmark) noexcept;

#pragma endregion
  };
}