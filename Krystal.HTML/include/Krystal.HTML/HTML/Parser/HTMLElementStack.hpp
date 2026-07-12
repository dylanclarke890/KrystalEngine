#pragma once

#include "Krystal.HTML/HTML/HTMLTableElement.hpp"
#include "Krystal.HTML/HTML/HTMLTemplateElement.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLStackItem.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include <cassert>

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/multipage/parsing.html#stack-of-open-elements
  class HTMLElementStack
  {
  private:
    List<HTMLStackItem> _items;

  public:
#pragma region Accessors

    /// @brief Get the first stack item added to the stack.
    KRYS_NODISCARD HTMLStackItem &Top() noexcept;

    /// @brief Get the stack item immediately below the top of the stack, or null if there is no such item.
    KRYS_NODISCARD RawPtr<HTMLStackItem> OneBelowTop() noexcept;

    /// @brief Get the most recently added element (the current element).
    KRYS_NODISCARD HTMLStackItem &Bottom() noexcept;

    /// @brief Get the most recently added element (the current element).
    KRYS_NODISCARD const HTMLStackItem &Bottom() const noexcept;

    /// @brief Get the stack item at the given index.
    KRYS_NODISCARD HTMLStackItem &operator[](size_t index) noexcept;

#pragma endregion

#pragma region Push

    /// @brief Add an element to the top of the stack.
    void Push(HTMLStackItem &&item) noexcept
    {
      _items.push_back(Krys::Move(item));
    }

#pragma endregion

#pragma region Pop

    /// @brief Remove the most recently added element from the stack.
    void Pop() noexcept;

    /// @brief Pops elements off the stack until the given node has been encountered. The given node is not
    /// popped.
    void PopUntil(const Element &node) noexcept;

    /// @brief Pops elements off the stack until the given tagname has been encountered. The given tagname is
    /// not popped.
    void PopUntil(TagName tagName, Namespace tagNamespace) noexcept;

    /// @brief Pops elements off the stack until the given node itself has been popped.
    void PopUntilPopped(const Element &node) noexcept;

    /// @brief Pops elements off the stack until the given tagname itself has been popped.
    void PopUntilPopped(TagName tagName, Namespace tagNamespace) noexcept;

    void PopUntilTableContext() noexcept;

    void PopUntilTableBodyContext() noexcept;

    void PopUntilTableRowContext() noexcept;

    void PopAll() noexcept;

    /// @see https://html.spec.whatwg.org/#generate-implied-end-tags
    void GenerateImpliedEndTags(Maybe<TagName> exception = Null) noexcept;

    /// @see https://html.spec.whatwg.org/#generate-implied-end-tags-thoroughly
    void GenerateImpliedEndTagsThoroughly() noexcept;

#pragma endregion

#pragma region InScope
  private:
    KRYS_NODISCARD bool IsScopeMarker(const HTMLStackItem &item) const noexcept;

    KRYS_NODISCARD bool IsListItemScopeMarker(const HTMLStackItem &item) const noexcept;

    KRYS_NODISCARD bool IsButtonScopeMarker(const HTMLStackItem &item) const noexcept;

    KRYS_NODISCARD bool IsTableScopeMarker(const HTMLStackItem &item) const noexcept;

    KRYS_NODISCARD bool IsTableBodyScopeMarker(const HTMLStackItem &item) const noexcept;

    KRYS_NODISCARD bool IsTableRowScopeMarker(const HTMLStackItem &item) const noexcept;

  public:
    KRYS_NODISCARD bool HasElementInScope(TagName targetNode) const noexcept;

    KRYS_NODISCARD bool HasElementInScope(const Element &element) const noexcept;

    KRYS_NODISCARD bool HasElementInListItemScope(TagName targetNode) const noexcept;

    KRYS_NODISCARD bool HasElementInButtonScope(TagName targetNode) const noexcept;

    KRYS_NODISCARD bool HasElementInTableScope(TagName targetNode) const noexcept;

#pragma endregion

    KRYS_NODISCARD auto begin() noexcept
    {
      return _items.begin();
    }

    KRYS_NODISCARD auto end() const noexcept
    {
      return _items.end();
    }

    KRYS_NODISCARD auto cbegin() const noexcept
    {
      return _items.cbegin();
    }

    KRYS_NODISCARD auto cend() const noexcept
    {
      return _items.cend();
    }

    KRYS_NODISCARD auto rbegin() const noexcept
    {
      return _items.rbegin();
    }

    KRYS_NODISCARD auto rend() const noexcept
    {
      return _items.rend();
    }

    /// @brief Returns true if the stack is empty.
    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _items.empty();
    }

    /// @brief Get the number of elements in the stack.
    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _items.size();
    }

    void Remove(const Element &node) noexcept
    {
      auto it = std::ranges::find_if(_items, [&](const auto &item) { return &item.Element() == &node; });
      if (it != _items.end())
      {
        _items.erase(it);
      }
    }

    KRYS_NODISCARD bool Contains(const Element &node) const noexcept;

    /// @brief Returns a pointer to the stack entry whose node matches the given node, or null.
    KRYS_NODISCARD RawPtr<HTMLStackItem> Find(const ContainerNode &node) noexcept
    {
      auto it = std::ranges::find_if(_items, [&](const auto &item) { return &item.Element() == &node; });
      return it != _items.end() ? &*it : nullptr;
    }

    /// @brief Inserts the new element into the stack of open elements immediately below the position of
    /// reference.
    void InsertBelow(HTMLStackItem &&newItem, const Element &reference) noexcept
    {
      auto it = std::ranges::find_if(_items, [&](const auto &item) { return &item.Element() == &reference; });
      assert(it != _items.end());
      _items.insert(std::next(it), Krys::Move(newItem));
    }

    /// @brief Returns the entry for the element immediately before the given node in the stack, if any such
    /// entry exists and is an element entry. Otherwise, returns null.
    /// @note This should always return a non-null entry for the situations in which it's used in the HTML
    /// tree builder algorithms.
    KRYS_NODISCARD RawPtr<HTMLStackItem> EntryBefore(Element &node) noexcept
    {
      for (auto it = _items.rbegin(); it != _items.rend(); ++it)
      {
        if (&it->Element() == &node)
        {
          auto nextIt = std::next(it);

          if (nextIt != _items.rend())
          {
            return &*nextIt;
          }

          return nullptr;
        }
      }

      return nullptr;
    }

    /// @brief Returns the entry for the element immediately before the given node in the stack, if any such
    /// entry exists and is an element entry. Otherwise, returns null.
    /// @note This should always return a non-null entry for the situations in which it's used in the HTML
    /// tree builder algorithms.
    KRYS_NODISCARD RawPtr<const HTMLStackItem> EntryBefore(Element &node) const noexcept
    {
      for (auto it = _items.rbegin(); it != _items.rend(); ++it)
      {
        if (&it->Element() == &node)
        {
          auto nextIt = std::next(it);
          if (nextIt != _items.rend())
          {
            return &*nextIt;
          }

          return nullptr;
        }
      }

      return nullptr;
    }

    KRYS_NODISCARD bool ContainsTemplateElement() const noexcept
    {
      return std::ranges::any_of(_items,
                                 [](const auto &item) { return Is<HTMLTemplateElement>(item.Element()); });
    }

    KRYS_NODISCARD bool ContainsInvalidUnclosedElements() const noexcept
    {
      return std::ranges::any_of(_items,
                                 [](const auto &item)
                                 {
                                   switch (item.TagName())
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
                                     case TagName::tbody:
                                     case TagName::td:
                                     case TagName::tfoot:
                                     case TagName::th:
                                     case TagName::thead:
                                     case TagName::tr:
                                     case TagName::body:
                                     case TagName::html:
                                     {
                                       return item.Namespace() != Namespace::HTML;
                                     }
                                   }

                                   return true;
                                 });
    }
  };
}