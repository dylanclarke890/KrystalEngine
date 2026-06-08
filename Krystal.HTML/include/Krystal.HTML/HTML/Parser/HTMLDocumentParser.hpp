#pragma once

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
  class HTMLDocumentParser : NonCopyMovable<HTMLDocumentParser>
  {
    HTMLDocument &_document;
    HTMLInputStream _input;
    HTMLTokenizer _tokenizer;
    HTMLTreeBuilder _treeBuilder;

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#script-nesting-level
    uint32 _scriptNestingLevel {0u};

    /// @see https://html.spec.whatwg.org/multipage/parsing.html#parser-pause-flag
    bool _paused {false};

  public:
    HTMLDocumentParser(HTMLDocument &document) noexcept
        : _document(document), _input(), _tokenizer(_input), _treeBuilder(document)
    {
    }

    KRYS_NODISCARD HTMLInputStream &InputStream() noexcept
    {
      return _input;
    }

    KRYS_NODISCARD bool IsStopped() const noexcept
    {
      return _paused;
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
      // TODO(HTML): We can optimise here to avoid copying character data by having a pointer to the
      // underlying buffer in the HTMLToken. This is because Character tokens can't cause us to re-enter
      // the parser. This is not straightforward as we currently need to convert from utf32 to utf8 at some
      // point, revisit this later.
      // if (rawToken->GetType() != HTMLTokenType::Character)
      // {
      //   rawToken.Clear();
      // }

      HTMLTokenAtom token(*rawToken);

      // Clear the rawToken in case _treeBuilder.ProcessToken synchronously re-enters the parser.
      rawToken.Clear();

      _treeBuilder.ProcessToken(std::move(token));
    }
  };
}