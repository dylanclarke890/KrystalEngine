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
    /// @brief Tracks a position in the list for use in the adoption agency algorithm.
    struct Bookmark
    {
      size_t index {0};
    };

    /// @see https://html.spec.whatwg.org/push-onto-the-list-of-active-formatting-elements
    void PushElement(HTMLStackItem &&item) noexcept;

    /// @brief Pushes a marker onto the list of active formatting elements.
    /// @see https://html.spec.whatwg.org/multipage/parsing.html#concept-parser-marker
    void PushMarker() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#reconstruct-the-active-formatting-elements
    void Reconstruct(HTMLElementStack &openElementStack) noexcept;

    /// @brief Pops elements up to and including the last marker.
    /// @see https://html.spec.whatwg.org/#clear-the-list-of-active-formatting-elements-up-to-the-last-marker
    void ClearUpToLastMarker() noexcept;

    /// @brief Searches from the end of the list back to the last marker for an element with the given tag
    /// name. Returns a pointer to that stack item, or null if none is found.
    KRYS_NODISCARD RawPtr<HTMLStackItem> FindFormattingElementFromLastMarker(TagName name) noexcept;

    /// @brief Returns a pointer to the formatting entry whose node matches the given node, or null.
    KRYS_NODISCARD RawPtr<FormattingListEntry> Find(const ContainerNode &node) noexcept;

    /// @brief Returns a bookmark positioned at the entry for the given node.
    KRYS_NODISCARD Bookmark BookmarkFor(const ContainerNode &node) noexcept;

    /// @brief Moves the bookmark to the position immediately after the given entry.
    void MoveBookmarkAfter(Bookmark &bookmark, const FormattingListEntry &entry) noexcept;

    /// @brief Removes the entry for node and adjusts bookmark if it falls at or after the removed entry.
    void RemoveAndUpdateBookmark(const ContainerNode &node, Bookmark &bookmark) noexcept;

    /// @brief Removes oldElement from the list and inserts newItem at the bookmark position.
    /// @see https://html.spec.whatwg.org/multipage/parsing.html#adoption-agency-algorithm step 18
    void SwapTo(const ContainerNode &oldElement, HTMLStackItem newItem,
                const Bookmark &bookmark) noexcept;

    /// @brief Removes the formatting list entry whose node matches the given node, if present.
    void RemoveFormattingElement(const ContainerNode &node) noexcept;

    bool ContainsFormattingElement(const ContainerNode &node) const noexcept
    {
      return std::ranges::any_of(_formattingElements, [&](const FormattingListEntry &entry)
                                 { return entry.IsFormattingElement() && &entry.Item().Node() == &node; });
    }
  };
}