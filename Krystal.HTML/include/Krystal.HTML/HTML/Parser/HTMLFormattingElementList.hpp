#pragma once

#include "Krystal.HTML/HTML/Parser/HTMLElementStack.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLStackItem.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class FormattingListEntry
  {
  private:
    Maybe<HTMLStackItem> _item;

  public:
    FormattingListEntry() noexcept = default;

    FormattingListEntry(HTMLStackItem &&item) noexcept : _item(Krys::Move(item))
    {
    }

    KRYS_NODISCARD HTMLStackItem &Item() noexcept
    {
      assert(_item.has_value());
      return _item.value();
    }

    KRYS_NODISCARD const HTMLStackItem &Item() const noexcept
    {
      assert(_item.has_value());
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

    void ReplaceItem(HTMLStackItem newItem) noexcept
    {
      _item = Krys::Move(newItem);
    }
  };

  /// @see https://html.spec.whatwg.org/multipage/parsing.html#the-list-of-active-formatting-elements
  class HTMLFormattingElementList
  {
  private:
    List<FormattingListEntry> _formattingElements;

  public:
    /// @see https://html.spec.whatwg.org/push-onto-the-list-of-active-formatting-elements
    void PushElement(HTMLStackItem &&item) noexcept;

    /// @brief Pushes a marker onto the list of active formatting elements.
    /// @see https://html.spec.whatwg.org/multipage/parsing.html#concept-parser-marker
    void PushMarker() noexcept;

    /// @brief Pops elements up to and including the last marker.
    /// @see https://html.spec.whatwg.org/#clear-the-list-of-active-formatting-elements-up-to-the-last-marker
    void ClearUpToLastMarker() noexcept;

    /// @brief Searches from the end of the list back to the last marker for an element with the given tag
    /// name. Returns a pointer to that stack item, or null if none is found.
    KRYS_NODISCARD RawPtr<HTMLStackItem> FindFromLastMarker(TagName tagName, Namespace tagNamespace) noexcept;

    /// @brief Returns a pointer to the formatting entry whose node matches the given node, or null.
    KRYS_NODISCARD RawPtr<FormattingListEntry> Find(const ContainerNode &node) noexcept;

    /// @brief Tracks a position in the list for use in the adoption agency algorithm.
    struct Bookmark
    {
      size_t index {0uz};
    };

    /// @brief Returns a bookmark positioned at the entry for the given node.
    KRYS_NODISCARD Bookmark BookmarkFor(const ContainerNode &node) noexcept;

    /// @brief Moves the bookmark to the position immediately after the given entry.
    void MoveBookmarkAfter(Bookmark &bookmark, const FormattingListEntry &entry) noexcept;

    /// @brief Inserts newItem at the bookmark position.
    void Insert(HTMLStackItem &&newItem, const Bookmark &bookmark) noexcept;

    /// @brief Removes the formatting list entry whose node matches the given node, if present.
    void Remove(const ContainerNode &node) noexcept;

    /// @brief Removes the entry for node and adjusts bookmark if it falls at or after the removed entry.
    void RemoveAndUpdateBookmark(const ContainerNode &node, Bookmark &bookmark) noexcept;

    bool Contains(const ContainerNode &node) const noexcept
    {
      return std::ranges::any_of(_formattingElements, [&](const FormattingListEntry &entry)
                                 { return entry.IsFormattingElement() && &entry.Item().Node() == &node; });
    }

    bool IsEmpty() const noexcept
    {
      return _formattingElements.empty();
    }

    auto begin() noexcept
    {
      return _formattingElements.begin();
    }

    auto end() noexcept
    {
      return _formattingElements.end();
    }

    auto Last() noexcept
    {
      if (_formattingElements.empty())
      {
        return _formattingElements.end();
      }

      return std::prev(_formattingElements.end());
    }
  };
}