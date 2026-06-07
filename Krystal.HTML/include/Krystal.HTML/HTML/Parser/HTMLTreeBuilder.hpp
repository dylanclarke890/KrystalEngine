#pragma once

#include "Krystal.HTML/DOM/HTMLDocument.hpp"
#include "Krystal.HTML/HTML/Enums/InsertionMode.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLActiveFormattingElementList.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLElementStack.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLStackItem.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTokenAtom.hpp"
#include "Krystal.Lib/Types/Stack.hpp"

namespace Krys::HTML
{
  class HTMLTreeBuilder
  {
  private:
    HTMLDocument &_document;

    /// @see https://html.spec.whatwg.org/multipage/syntax.html#insertion-mode
    InsertionMode _insertionMode : BitCount<InsertionMode>() {InsertionMode::Initial};

    /// @see https://html.spec.whatwg.org/multipage/syntax.html#original-insertion-mode
    InsertionMode _originalInsertionMode : BitCount<InsertionMode>() {InsertionMode::Initial};

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#stack-of-template-insertion-modes
    Stack<InsertionMode> _templateInsertionModeStack;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#stack-of-open-elements
    HTMLElementStack _openElementStack;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#the-list-of-active-formatting-elements
    HTMLActiveFormattingElementList _activeFormattingElementList;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#the-element-pointers
    RawPtr<HTMLStackItem> _head;
    RawPtr<HTMLStackItem> _form;

  public:
    HTMLTreeBuilder(HTMLDocument &document) noexcept : _document(document)
    {
    }

    void ProcessToken(HTMLTokenAtom &&token) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#reset-the-insertion-mode-appropriately
    void ResetInsertionModeAppropriately() noexcept
    {
    }

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#current-template-insertion-mode
    InsertionMode CurrentTemplateInsertionMode() const noexcept
    {
      return _templateInsertionModeStack.empty() ? InsertionMode::Initial : _templateInsertionModeStack.top();
    }

    /// @brief Get the current insertion mode.
    InsertionMode GetInsertionMode() const noexcept
    {
      return _insertionMode;
    }

    /// @brief Get the current element stack (for debugging/testing).
    HTMLElementStack &GetElementStack() noexcept
    {
      return _openElementStack;
    }

  private:
    void ProcessTokenInInitialMode(const HTMLTokenAtom &token) noexcept;
    void ProcessTokenInBeforeHTMLMode(const HTMLTokenAtom &token) noexcept;
    void ProcessTokenInBeforeHeadMode(const HTMLTokenAtom &token) noexcept;
    void ProcessTokenInHeadMode(const HTMLTokenAtom &token) noexcept;
    void ProcessTokenInInHeadNoscriptMode(const HTMLTokenAtom &token) noexcept;
    void ProcessTokenInAfterHeadMode(const HTMLTokenAtom &token) noexcept;
    void ProcessTokenInBodyMode(const HTMLTokenAtom &token) noexcept;
    void ProcessTokenInTextMode(const HTMLTokenAtom &token) noexcept;
    void ProcessTokenInTableMode(const HTMLTokenAtom &token) noexcept;

    void ChangeInsertionMode(InsertionMode newMode) noexcept
    {
      _insertionMode = newMode;
    }
  };
}