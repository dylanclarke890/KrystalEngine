#pragma once

#include "Krystal.HTML/HTML/Parser/HTMLStackItem.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/multipage/parsing.html#the-list-of-active-formatting-elements
  class HTMLActiveFormattingElementList
  {
  private:
    List<HTMLStackItem> _items;
  };
}