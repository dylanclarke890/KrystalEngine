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
    RawPtr<Element> ElementBeforeLastTable {nullptr};
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

    /// @brief Pops elements off the stack until the given node itself has been popped.
    void PopUntilPopped(const ContainerNode &node) noexcept
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

    /// @brief Pops elements off the stack until the given tagname itself has been popped.
    void PopUntilPopped(TagName tagName) noexcept
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

    /// @brief Get the first element added to the stack (typically the html element).
    KRYS_NODISCARD HTMLStackItem &Top() noexcept
    {
      assert(!_items.empty());
      return _items.front();
    }

    KRYS_NODISCARD const HTMLStackItem &Top() const noexcept
    {
      assert(!_items.empty());
      return _items.front();
    }

    /// @brief Get the most recently added element (the current element).
    KRYS_NODISCARD HTMLStackItem &Bottom() noexcept
    {
      assert(!_items.empty());
      return _items.back();
    }

    KRYS_NODISCARD const HTMLStackItem &Bottom() const noexcept
    {
      assert(!_items.empty());
      return _items.back();
    }

    void Remove(const ContainerNode &node) noexcept
    {
      auto it = std::ranges::find_if(_items, [&](const auto &item)
                                     { return item.IsElement() && &item.Node() == &node; });
      if (it != _items.end())
      {
        _items.erase(it);
      }
    }

    /// @brief Returns a pointer to the stack entry whose node matches the given node, or null.
    KRYS_NODISCARD RawPtr<HTMLStackItem> Find(const ContainerNode &node) noexcept
    {
      auto it = std::ranges::find_if(_items, [&](const auto &item)
                                     { return item.IsElement() && &item.Node() == &node; });
      return it != _items.end() ? &*it : nullptr;
    }

    /// @brief Inserts newItem immediately above (toward the current element) the reference entry.
    /// @see https://html.spec.whatwg.org/multipage/parsing.html#adoption-agency-algorithm
    void InsertAbove(HTMLStackItem newItem, const HTMLStackItem &reference) noexcept
    {
      auto it = std::ranges::find_if(_items, [&](const auto &item) { return &item == &reference; });
      assert(it != _items.end());
      _items.insert(std::next(it), Krys::Move(newItem));
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

    KRYS_NODISCARD HTMLStackItem &At(size_t index) noexcept
    {
      assert(index < _items.size());
      return _items[index];
    }

    KRYS_NODISCARD const HTMLStackItem &At(size_t index) const noexcept
    {
      assert(index < _items.size());
      return _items[index];
    }

    KRYS_NODISCARD HTMLStackItem &operator[](size_t index) noexcept
    {
      assert(index < _items.size());
      return _items[index];
    }

    KRYS_NODISCARD const HTMLStackItem &operator[](size_t index) const noexcept
    {
      assert(index < _items.size());
      return _items[index];
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

      auto lastTableIt = _items.rend();
      auto lastTemplateIt = _items.rend();

      for (auto it = _items.rbegin(); it != _items.rend(); ++it)
      {
        if (!Is<HTMLElement>(it->Node()))
        {
          continue;
        }

        auto &element = Downcast<HTMLElement>(it->Node());
        if (result.LastTemplateElement == nullptr && Is<HTMLTemplateElement>(element))
        {
          result.LastTemplateElement = &Downcast<HTMLTemplateElement>(element);
          lastTemplateIt = it;
        }
        else if (result.LastTableElement == nullptr && Is<HTMLTableElement>(element))
        {
          result.LastTableElement = &Downcast<HTMLTableElement>(element);
          lastTableIt = it;

          auto next = std::next(it);
          if (next != _items.rend() && next->IsElement())
          {
            result.ElementBeforeLastTable = &Downcast<Element>(next->Node());
          }
        }

        if (result.LastTemplateElement != nullptr && result.LastTableElement != nullptr)
        {
          break;
        }
      }

      result.TemplateIsMostRecent = lastTemplateIt < lastTableIt;

      return result;
    }

    KRYS_NODISCARD bool ContainsTemplateElement() const noexcept
    {
      return std::ranges::any_of(_items, [](const auto &item)
                                 { return item.IsElement() && Is<HTMLTemplateElement>(item.Node()); });
    }

    KRYS_NODISCARD bool ContainsInvalidUnclosedElements() const noexcept
    {
      return std::ranges::any_of(_items,
                                 [](const auto &item)
                                 {
                                   if (!item.IsElement())
                                   {
                                     return false;
                                   }

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
                                     case TagName::html:     return false;
                                     default:                return true;
                                   }
                                 });
    }
  };
}