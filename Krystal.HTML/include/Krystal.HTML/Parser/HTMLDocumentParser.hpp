#pragma once

#include "Krystal.HTML/DOM/HTMLDocument.hpp"
#include "Krystal.HTML/Loader/TextResourceDecoder.hpp"
#include "Krystal.HTML/Parser/HTMLInputStream.hpp"
#include "Krystal.HTML/Parser/HTMLToken.hpp"
#include "Krystal.HTML/Parser/HTMLTokenAtom.hpp"
#include "Krystal.HTML/Parser/HTMLTokenizer.hpp"
#include "Krystal.HTML/Parser/HTMLTreeBuilder.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/Pointers/ReferenceWrapper.hpp"

namespace Krys::HTML
{
  class HTMLDocumentParser : NonCopyMovable<HTMLDocumentParser>
  {
    ReferenceWrapper<HTMLDocument> _document;
    HTMLInputStream _input;
    HTMLTokenizer _tokenizer;
    HTMLTreeBuilder _treeBuilder;
    bool _paused {false};

  public:
    HTMLDocumentParser(HTMLDocument &document) noexcept
        : _document(document), _input(), _tokenizer(_input), _treeBuilder()
    {
    }

    HTMLInputStream &InputStream() noexcept
    {
      return _input;
    }

    bool PumpTokenizer() noexcept
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
      // We don't clear the token immediately for Character tokens because the AtomHTMLToken avoids copying
      // the characters by keeping a pointer to the underlying buffer in the HTMLToken. Fortunately, Character
      // tokens can't cause us to re-enter the parser.
      if (rawToken->GetType() != HTMLToken::Type::Character)
      {
        rawToken.Clear();
      }

      _treeBuilder.ProcessToken(std::move(token));
    }

    KRYS_NODISCARD bool IsStopped() const noexcept
    {
      return _paused;
    }
  };
}