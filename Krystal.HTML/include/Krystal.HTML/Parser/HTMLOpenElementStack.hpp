#pragma once

#include "Krystal.HTML/Parser/HTMLElementStackItem.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include <cassert>

namespace Krys::HTML
{
  /// @brief This stack grows downwards; the topmost node on the stack is the first one added to the stack,
  /// and the bottommost node of the stack is the most recently added node in the stack.
  /// @see https://html.spec.whatwg.org/multipage/parsing.html#stack-of-open-elements
  class HTMLOpenElementStack
  {
  private:
    List<HTMLElementStackItem> _items;

  public:
    void Push(const HTMLElementStackItem &item) noexcept
    {
      _items.push_back(item);
    }

    void Pop() noexcept
    {
      assert(!_items.empty());
      _items.pop_back();
    }

    /// @brief Get the topmost element (the first element added to the stack).
    HTMLElementStackItem &Top() noexcept
    {
      assert(!_items.empty());
      return _items.back();
    }

    HTMLElementStackItem &At(size_t index) noexcept
    {
      assert(index < _items.size());
      return _items[index];
    }

    /// @brief Get the bottommost element (the element most recently added to the stack).
    HTMLElementStackItem &Bottom() noexcept
    {
      assert(!_items.empty());
      return _items.front();
    }
  };
}