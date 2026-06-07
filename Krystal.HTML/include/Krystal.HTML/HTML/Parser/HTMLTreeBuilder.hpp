#pragma once

#include "Krystal.HTML/HTML/Enums/InsertionMode.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLConstructionSite.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTokenAtom.hpp"
#include "Krystal.Lib/Types/Stack.hpp"

namespace Krys::HTML
{
  class HTMLTreeBuilder
  {
  private:
    /// @see https://html.spec.whatwg.org/multipage/syntax.html#the-insertion-mode
    InsertionMode _insertionMode {InsertionMode::Initial};
    InsertionMode _originalInsertionMode {InsertionMode::Initial};
    Stack<InsertionMode> _templateInsertionModeStack;
    HTMLConstructionSite _tree;

  public:
    /// @see https://html.spec.whatwg.org/multipage/parsing.html#reset-the-insertion-mode-appropriately
    void ResetInsertionModeAppropriately() noexcept
    {
      bool last = false;
    }

    void ProcessToken(HTMLTokenAtom &&token) noexcept
    {
    }
  };
}