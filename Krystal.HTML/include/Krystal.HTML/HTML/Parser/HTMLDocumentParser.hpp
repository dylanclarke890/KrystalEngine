#pragma once

#include "Krystal.HTML/DOM/DocumentFragment.hpp"
#include "Krystal.HTML/DOM/HTMLDocument.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLElementStack.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLInputStream.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLToken.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTokenAtom.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTokenizer.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTreeBuilder.hpp"
#include "Krystal.HTML/HTML/Parser/TextResourceDecoder.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/Pointers/ReferenceWrapper.hpp"

namespace Krys::HTML
{
  class HTMLDocumentParser : public RefCounted<HTMLDocumentParser>
  {
    HTMLDocument &_document;
    HTMLInputStream _input;
    HTMLTokenizer _tokenizer;
    HTMLTreeBuilder _treeBuilder;
    Maybe<HTMLStackItem> _fragmentParsingContext;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#script-nesting-level
    uint32 _scriptNestingLevel {0u};

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#parser-pause-flag
    bool _paused {false};

  public:
    HTMLDocumentParser(HTMLDocument &document) noexcept;

    /// @see https://html.spec.whatwg.org/#html-fragment-parsing-algorithm
    KRYS_NODISCARD static SmallNodeList
      ParseFragment(Element &contextElement, utf8_string &&input, bool allowDeclarativeShadowRoots = false,
                    ParserScriptingMode scriptingMode = ParserScriptingMode::Inert) noexcept;

    void PumpTokenizer() noexcept;

    KRYS_NODISCARD HTMLInputStream &InputStream() noexcept
    {
      return _input;
    }

    KRYS_NODISCARD bool IsStopped() const noexcept
    {
      return _paused;
    }

    void ScriptingMode(ParserScriptingMode scriptingMode) noexcept
    {
      _treeBuilder.ScriptingMode(scriptingMode);
    }

    ParserScriptingMode ScriptingMode() const noexcept
    {
      return _treeBuilder.ScriptingMode();
    }

    void TokenizerState(TokenizerState state) noexcept
    {
      _tokenizer.State(state);
    }

    void FormElement(HTMLFormElement &form) noexcept
    {
      _treeBuilder.FormElement(form);
    }

    KRYS_NODISCARD HTMLElementStack &OpenElementStack() noexcept
    {
      return _treeBuilder.OpenElementStack();
    }

    void ResetInsertionModeAppropriately() noexcept
    {
      _treeBuilder.ResetInsertionModeAppropriately();
    }
  };
}