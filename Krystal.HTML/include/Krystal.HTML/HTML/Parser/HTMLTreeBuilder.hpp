#pragma once

#include "Krystal.HTML/HTML/Enums/InsertionMode.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLActiveFormattingElementList.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLElementStackItem.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLOpenElementStack.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTokenAtom.hpp"
#include "Krystal.Lib/Types/Stack.hpp"

namespace Krys::HTML
{
  class HTMLTreeBuilder
  {
  private:
    /// @see https://html.spec.whatwg.org/multipage/syntax.html#insertion-mode
    InsertionMode _insertionMode : BitCount<InsertionMode>() {InsertionMode::Initial};

    /// @see https://html.spec.whatwg.org/multipage/syntax.html#original-insertion-mode
    InsertionMode _originalInsertionMode : BitCount<InsertionMode>() {InsertionMode::Initial};
    
    /// @see https://html.spec.whatwg.org/multipage/parsing.html#stack-of-template-insertion-modes
    Stack<InsertionMode> _templateInsertionModeStack;

    RefPtr<Document> _document;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#stack-of-open-elements
    HTMLOpenElementStack _openElementStack;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#the-list-of-active-formatting-elements
    HTMLActiveFormattingElementList _activeFormattingElementList;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#the-element-pointers
    HTMLStackItem _head;
    HTMLStackItem _form;


  public:
    void ProcessToken(HTMLTokenAtom &&token) noexcept
    {
    }

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#reset-the-insertion-mode-appropriately
    void ResetInsertionModeAppropriately() noexcept
    {
      bool last = false;
    }

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#current-template-insertion-mode
    InsertionMode CurrentTemplateInsertionMode() const noexcept
    {
      return _templateInsertionModeStack.empty() ? InsertionMode::Initial : _templateInsertionModeStack.top();
    }
  };
}