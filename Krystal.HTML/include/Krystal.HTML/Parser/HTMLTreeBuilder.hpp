#pragma once

#include "Krystal.HTML/Parser/AtomHTMLToken.hpp"
#include "Krystal.HTML/Parser/HTMLConstructionSite.hpp"
#include "Krystal.HTML/Parser/InsertionMode.hpp"
#include "Krystal.HTML/StronglyTypedValues.hpp"
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
    }

    void ProcessToken(AtomHTMLToken &&token) noexcept
    {
    }
  };
}