#pragma once

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
    void Push(HTMLStackItem &&item) noexcept
    {
      _items.push_back(Krys::Move(item));
    }

    /// @brief Get the first element added to the stack.
    HTMLStackItem &First() noexcept
    {
      assert(!_items.empty());
      return _items.back();
    }

    /// @brief Get the the element most recently added to the stack.
    HTMLStackItem &Last() noexcept
    {
      assert(!_items.empty());
      return _items.front();
    }

    /// @brief Remove the most recently added element from the stack.
    void Pop() noexcept
    {
      assert(!_items.empty());
      _items.pop_back();
    }
  };
}