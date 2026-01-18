#pragma once

#include "Krystal.HTML/HTMLDocument.hpp"
#include "Krystal.HTML/Loader/TextResourceDecoder.hpp"
#include "Krystal.HTML/Parser/HTMLInputStream.hpp"
#include "Krystal.HTML/Parser/HTMLTokenizer.hpp"
#include "Krystal.HTML/Parser/HTMLTreeBuilder.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/Utils/ReferenceWrapper.hpp"

namespace Krys::HTML
{
  class HTMLDocumentParser : NonCopyMovable<HTMLDocumentParser>
  {
    ReferenceWrapper<HTMLDocument> _document;
    HTMLInputStream _input;
    HTMLTokenizer _tokenizer;
    HTMLTreeBuilder _treeBuilder;

  public:
    HTMLDocumentParser(HTMLDocument &document) noexcept : _document(document)
    {
    }
  };
}