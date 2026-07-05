#pragma once

#include "Krystal.HTML/DOM/DocumentFragment.hpp"
#include "Krystal.HTML/DOM/HTMLDocument.hpp"
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
  struct FragmentParsingContext
  {
  private:
    RefPtr<DocumentFragment> _fragment;
    RefPtr<Element> _contextElement;

  public:
    FragmentParsingContext() noexcept = default;

    FragmentParsingContext(DocumentFragment &fragment, Element &contextElement) noexcept
        : _fragment(ShareRefPtr(&fragment)), _contextElement(ShareRefPtr(&contextElement))
    {
    }

    KRYS_NODISCARD DocumentFragment &Fragment() const noexcept
    {
      assert(_fragment);
      return *_fragment;
    }

    KRYS_NODISCARD Element &ContextElement() const noexcept
    {
      assert(_contextElement);
      return *_contextElement;
    }
  };

  class HTMLDocumentParser : public RefCounted<HTMLDocumentParser>
  {
    HTMLDocument &_document;
    HTMLInputStream _input;
    HTMLTokenizer _tokenizer;
    HTMLTreeBuilder _treeBuilder;
    Maybe<FragmentParsingContext> _fragmentParsingContext;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#script-nesting-level
    uint32 _scriptNestingLevel {0u};

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#parser-pause-flag
    bool _paused {false};

  public:
    HTMLDocumentParser(HTMLDocument &document) noexcept
        : _document(document), _input(), _tokenizer(_input), _treeBuilder(document, _tokenizer)
    {
      _treeBuilder.SetScriptingMode(ParserScriptingMode::Normal);
    }

    HTMLDocumentParser(HTMLDocument &document, DocumentFragment &fragment, Element &contextElement) noexcept
        : _document(document), _input(), _tokenizer(_input), _treeBuilder(document, _tokenizer),
          _fragmentParsingContext(FragmentParsingContext(fragment, contextElement))
    {
      _treeBuilder.SetScriptingMode(ParserScriptingMode::Fragment);
    }

    KRYS_NODISCARD HTMLInputStream &InputStream() noexcept
    {
      return _input;
    }

    KRYS_NODISCARD bool IsStopped() const noexcept
    {
      return _paused;
    }

    void DisableScripting() noexcept
    {
      _treeBuilder.SetScriptingMode(ParserScriptingMode::Disabled);
    }

    void EnableScripting() noexcept
    {
      if (_fragmentParsingContext.has_value())
      {
        _treeBuilder.SetScriptingMode(ParserScriptingMode::Fragment);
      }
      else
      {
        _treeBuilder.SetScriptingMode(ParserScriptingMode::Normal);
      }
    }

    KRYS_NODISCARD bool PumpTokenizer() noexcept
    {
      while (!IsStopped())
      {
        NextTokenPtr token = _tokenizer.NextToken();
        if (!token)
        {
          return false;
        }

        ConstructTreeFromToken(token);
      }

      return false;
    }

    void ConstructTreeFromToken(NextTokenPtr &rawToken) noexcept
    {
      HTMLTokenAtom token(*rawToken);

      // Clear the rawToken in case _treeBuilder.ProcessToken synchronously re-enters the parser.
      // Character tokens can't cause us to re-enter the parser so we can optimise by keeping a pointer to the
      // data buffer in the token instead of needing to copy it.
      if (rawToken->Type() != HTMLTokenType::Character)
      {
        rawToken.Clear();
      }

      _treeBuilder.ProcessToken(Krys::Move(token));
    }
  };
}