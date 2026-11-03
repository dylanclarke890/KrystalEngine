#pragma once

#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/Parser/InputStream.hpp"
#include "Krystal.Booey/HTML/Parser/OpenElementStack.hpp"
#include "Krystal.Booey/HTML/Parser/Token.hpp"
#include "Krystal.Booey/HTML/Parser/TokenAtom.hpp"
#include "Krystal.Booey/HTML/Parser/Tokenizer.hpp"
#include "Krystal.Booey/HTML/Parser/TreeBuilder.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo::html
{
  class DocumentParser : public RefCounted<DocumentParser>
  {
    dom::HTMLDocument &_document;
    InputStream _input;
    Tokenizer _tokenizer;
    TreeBuilder _treeBuilder;
    Maybe<StackItem> _fragmentParsingContext;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#script-nesting-level
    uint32 _scriptNestingLevel {0u};

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#parser-pause-flag
    bool _paused {false};

  public:
    DocumentParser(dom::HTMLDocument &document) noexcept;

    /// @brief Parses @p input as the children of @p target, an element or a shadow root (whose host is then
    /// the context).
    /// @see https://html.spec.whatwg.org/#html-fragment-parsing-algorithm
    KRYS_NODISCARD static Ref<dom::DocumentFragment>
      ParseFragment(dom::ContainerNode &target, utf8_string input, bool allowDeclarativeShadowRoots = false,
                    ParserScriptingMode scriptingMode = ParserScriptingMode::Inert,
                    Maybe<SanitizerConfig> &&configuration = null) noexcept;

    /// @brief Sets the parser sanitizer configuration; null for none.
    /// @see https://html.spec.whatwg.org/multipage/parsing.html#parser-sanitizer-configuration
    void SanitizerConfiguration(Maybe<SanitizerConfig> &&configuration) noexcept
    {
      _treeBuilder.SanitizerConfiguration(krys::move(configuration));
    }

    void PumpTokenizer() noexcept;

    /// @brief Makes the parser take all its input as text, in a `pre` element: a text document's parser.
    /// @see https://html.spec.whatwg.org/#navigate-text
    void StartPlainText() noexcept;

    KRYS_NODISCARD InputStream &InputStream() noexcept
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

    KRYS_NODISCARD OpenElementStack &GetOpenElementStack() noexcept
    {
      return _treeBuilder.GetOpenElementStack();
    }

    void ResetInsertionModeAppropriately() noexcept
    {
      _treeBuilder.ResetInsertionModeAppropriately();
    }
  };
}