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
    /// @brief Pushes a marker onto the list of active formatting elements.
    /// @see https://html.spec.whatwg.org/multipage/parsing.html#concept-parser-marker
    void PushMarker() noexcept;

    /// @brief Pushes an element onto the list of active formatting elements.
    /// @see https://html.spec.whatwg.org/push-onto-the-list-of-active-formatting-elements
    void PushElement(HTMLStackItem &&item) noexcept;

    /// @brief Searches from the end of the list back to the last marker for an element with the given tag
    /// name. Returns a pointer to that stack item, or null if none is found.
    KRYS_NODISCARD RawPtr<HTMLStackItem> FindFromLastMarker(TagName tagName, Namespace tagNamespace) noexcept;

    /// @brief Returns a pointer to the formatting entry whose node matches the given node, or nullptr if it
    /// wasn't found.
    KRYS_NODISCARD RawPtr<FormattingListEntry> Find(const ContainerNode &node) noexcept;

    /// @brief Returns true if the list contains a formatting entry whose node matches the given node.
    KRYS_NODISCARD bool Contains(const ContainerNode &node) const noexcept;

    /// @brief Removes the formatting list entry whose node matches the given node, if present.
    void Remove(const ContainerNode &node) noexcept;

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
    KRYS_NODISCARD Bookmark BookmarkFor(const ContainerNode &node) noexcept;

    /// @brief Moves the bookmark to the position immediately after the given entry.
    void MoveBookmarkAfter(Bookmark &bookmark, const FormattingListEntry &entry) noexcept;

    /// @brief Removes the entry for node and adjusts bookmark if it falls at or after the removed entry.
    void RemoveAndUpdateBookmark(const ContainerNode &node, Bookmark &bookmark) noexcept;

    /// @brief Inserts newItem at the bookmark position.
    void InsertAtBookmark(HTMLStackItem &&newItem, const Bookmark &bookmark) noexcept;

#pragma endregion
  };
}