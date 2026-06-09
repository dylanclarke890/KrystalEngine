#pragma once

#include "Krystal.HTML/HTML/HTMLTableElement.hpp"
#include "Krystal.HTML/HTML/HTMLTemplateElement.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLStackItem.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include <cassert>

namespace Krys::HTML
{
  struct LastTableAndTemplateResult
  {
    RawPtr<HTMLTableElement> LastTableElement {nullptr};
    RawPtr<HTMLTemplateElement> LastTemplateElement {nullptr};
    RawPtr<ContainerNode> ElementBeforeLastTable {nullptr};
    bool TemplateIsMostRecent {false};
  };

  /// @see https://html.spec.whatwg.org/multipage/parsing.html#stack-of-open-elements
  class HTMLElementStack
  {
  private:
    List<HTMLStackItem> _items;

  public:
    /// @brief Add an element to the top of the stack.
    void Push(HTMLStackItem &&item) noexcept
    {
      _items.push_back(Krys::Move(item));
    }

    /// @brief Remove the most recently added element from the stack.
    void Pop() noexcept
    {
      assert(!_items.empty());

      // TODO(HTMLTREEBUILDER, HTML): When the current node is removed from the stack of open elements,
      // process internal resource links given the current node's node document.

      _items.pop_back();
    }

    /// @brief Get the first element added to the stack.
    KRYS_NODISCARD HTMLStackItem &Top() noexcept
    {
      assert(!_items.empty());
      return _items.back();
    }

    KRYS_NODISCARD const HTMLStackItem &Top() const noexcept
    {
      assert(!_items.empty());
      return _items.back();
    }

    /// @brief Get the the element most recently added to the stack.
    KRYS_NODISCARD HTMLStackItem &Bottom() noexcept
    {
      assert(!_items.empty());
      return _items.front();
    }

    KRYS_NODISCARD const HTMLStackItem &Bottom() const noexcept
    {
      assert(!_items.empty());
      return _items.front();
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

    KRYS_NODISCARD bool ContainsElement(const ContainerNode &node) const noexcept
    {
      return std::ranges::any_of(_items,
                                 [&](const auto &item) { return item.IsElement() && &item.Node() == &node; });
    }

    /// @brief Returns the entry for the element immediately before the given node in the stack, if any such
    /// entry exists and is an element entry. Otherwise, returns null.
    /// @note This should always return a non-null entry for the situations in which it's used in the HTML
    /// tree builder algorithms.
    KRYS_NODISCARD RawPtr<HTMLStackItem> EntryBefore(ContainerNode &node) noexcept
    {
      for (auto it = _items.rbegin(); it != _items.rend(); ++it)
      {
        if (it->IsElement() && &it->Node() == &node)
        {
          auto nextIt = std::next(it);
          if (nextIt != _items.rend() && nextIt->IsElement())
          {
            return &*nextIt;
          }
        }
      }

      return nullptr;
    }

    /// @brief Returns the entry for the element immediately before the given node in the stack, if any such
    /// entry exists and is an element entry. Otherwise, returns null.
    /// @note This should always return a non-null entry for the situations in which it's used in the HTML
    /// tree builder algorithms.
    KRYS_NODISCARD RawPtr<const HTMLStackItem> EntryBefore(ContainerNode &node) const noexcept
    {
      for (auto it = _items.rbegin(); it != _items.rend(); ++it)
      {
        if (it->IsElement() && &it->Node() == &node)
        {
          auto nextIt = std::next(it);
          if (nextIt != _items.rend() && nextIt->IsElement())
          {
            return &*nextIt;
          }
        }
      }

      return nullptr;
    }

    KRYS_NODISCARD LastTableAndTemplateResult LastTableAndTemplate() const noexcept
    {
      LastTableAndTemplateResult result;

      // TODO: compute last table, last template, the node entry before last table and whether last template
      // was added more recently than last table.

      return result;
    }
  };
}