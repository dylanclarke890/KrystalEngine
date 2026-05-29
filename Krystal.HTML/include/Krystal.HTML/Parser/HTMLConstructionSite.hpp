#pragma once

#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/Parser/HTMLActiveFormattingElementList.hpp"
#include "Krystal.HTML/Parser/HTMLElementStackItem.hpp"
#include "Krystal.HTML/Parser/HTMLOpenElementStack.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"

namespace Krys::HTML
{
  class HTMLConstructionSite
  {
  private:
    WeakPtr<Document> _document;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#stack-of-open-elements
    HTMLOpenElementStack _openElementStack;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#the-list-of-active-formatting-elements
    HTMLActiveFormattingElementList _activeFormattingElementList;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#the-element-pointers
    HTMLElementStackItem _head;
    HTMLElementStackItem _form;
  };
}