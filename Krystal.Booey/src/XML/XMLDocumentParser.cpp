#include "Krystal.Booey/XML/XMLDocumentParser.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"

namespace krys::boo::xml
{
  namespace
  {
    /// @brief How deep elements may nest before parsing stops with a fatal error.
    constexpr size_t MaxXMLTreeDepth = 5'000;

    /// @brief The namespaces in scope on a fragment's context element.
    struct XMLParsingNamespaces
    {
      DOMStringAtom DefaultNamespace {null};
      HashMap<DOMStringAtom, DOMStringAtom> PrefixNamespaces;
    };

    KRYS_NODISCARD XMLParsingNamespaces FindXMLParsingNamespaces(Element *contextElement) noexcept
    {
      if (!contextElement)
      {
        return {};
      }

      XMLParsingNamespaces result;
      result.DefaultNamespace = contextElement->LookupNamespaceURI(null);
      for (auto *element = contextElement; element; element = element->ParentElement())
      {
        if (!element->HasAttributes())
        {
          continue;
        }
        for (auto &attribute : element->AttributeList())
        {
          // A declaration nearer the context element hides an ancestor's for the same prefix.
          if (attribute->Prefix() == infra::namespace_prefixes::XMLNS)
          {
            result.PrefixNamespaces.try_emplace(attribute->LocalName(), DOMStringAtom(attribute->Value()));
          }
        }
      }
      return result;
    }
  }

#pragma region Parsing

  void XMLDocumentParser::Append(DOMString source) noexcept
  {
    if (IsStopped())
    {
      return;
    }
    DoWrite(source);
  }

  void XMLDocumentParser::Finish() noexcept
  {
    krys_debug_assert(!IsStopped());
    Ref<XMLDocumentParser> protectedThis = ShareRef(*this);
    End();
  }

  void XMLDocumentParser::End() noexcept
  {
    // Ending touches the document and its loader, which a fragment's parser has no business with.
    krys_debug_assert(!_parsingFragment);

    DoEnd();
    // Ending can detach the parser, taking its document away.
    if (IsDetached())
    {
      return;
    }

    if (_sawError && !IsStopped())
    {
      InsertErrorMessageBlock();
      if (IsDetached())
      {
        return;
      }
    }
    else
    {
      UpdateLeafTextNode();
      Document()->StyleScope().DidChangeStyleSheetEnvironment();
    }

    if (IsParsing())
    {
      PrepareToStopParsing();
    }
    ClearCurrentNodeStack();
    Ref<boo::Document> document = ShareRef(*Document());
    document->FinishedParsing();
  }

  void XMLDocumentParser::Detach() noexcept
  {
    ClearCurrentNodeStack();
    DocumentParser::Detach();
  }

  bool XMLDocumentParser::ParseDocumentFragment(DOMStringView chunk, DocumentFragment &fragment,
                                                Element *contextElement) noexcept
  {
    if (chunk.empty())
    {
      return true;
    }

    // https://html.spec.whatwg.org/#xml-fragment-parsing-algorithm
    // (Rather than feeding the parser the context element's start and end tags, the parser starts with the
    // namespaces in scope on it declared, and parses the input straight into the fragment, checking it's
    // well formed as it goes.)
    auto namespaces = FindXMLParsingNamespaces(contextElement);
    auto parser = XMLDocumentParser::Create(fragment, krys::move(namespaces.PrefixNamespaces),
                                            namespaces.DefaultNamespace);
    bool wellFormed = parser->AppendFragmentSource(chunk);
    // Not Finish(): ending touches the document and its loader. Detaching is what a parser must do before
    // it's destroyed.
    parser->Detach();
    return wellFormed;
  }

#pragma endregion

#pragma region Errors

  void XMLDocumentParser::HandleError(XMLErrors::Type type, const char *message,
                                      SourceLocation position) noexcept
  {
    if (!_xmlErrors)
    {
      _xmlErrors = CreateUnique<XMLErrors>(*Document());
    }
    _xmlErrors->HandleError(type, message, position);
    if (type != XMLErrors::Type::Warning)
    {
      _sawError = true;
    }
    if (type == XMLErrors::Type::Fatal)
    {
      StopParsing();
    }
  }

  void XMLDocumentParser::InsertErrorMessageBlock() noexcept
  {
    krys_debug_assert(_xmlErrors);
    _xmlErrors->InsertErrorMessageBlock();
  }

#pragma endregion

#pragma region The tree being built

  void XMLDocumentParser::PushCurrentNode(ContainerNode *node) noexcept
  {
    krys_debug_assert(node);
    krys_debug_assert(_currentNode);
    _currentNodeStack.push_back(std::exchange(_currentNode, ShareRefPtr(node)));
    if (_currentNodeStack.size() > MaxXMLTreeDepth)
    {
      HandleError(XMLErrors::Type::Fatal, "Excessive node nesting.", TextPosition());
    }
  }

  void XMLDocumentParser::PopCurrentNode() noexcept
  {
    if (!_currentNode)
    {
      return;
    }
    krys_debug_assert(!_currentNodeStack.empty());
    _currentNode = krys::move(_currentNodeStack.back());
    _currentNodeStack.pop_back();
  }

  void XMLDocumentParser::ClearCurrentNodeStack() noexcept
  {
    _currentNode = nullptr;
    _leafTextNode = nullptr;
    _currentNodeStack.clear();
  }

  void XMLDocumentParser::CreateLeafTextNode() noexcept
  {
    if (_leafTextNode)
    {
      return;
    }
    krys_debug_assert(_bufferedText.empty());
    _leafTextNode = CreateRef<Text>(_currentNode->NodeDocument(), DOMString());
    if (RefPtr<ContainerNode> currentNode = _currentNode)
    {
      (void)dom::MutationAlgorithms::Append(*_leafTextNode, *currentNode);
    }
  }

  bool XMLDocumentParser::UpdateLeafTextNode() noexcept
  {
    if (IsStopped())
    {
      return false;
    }
    if (!_leafTextNode)
    {
      return true;
    }

    RefPtr<Text> leafTextNode = _leafTextNode;
    (void)leafTextNode->AppendData(krys::move(_bufferedText));
    _bufferedText.clear();
    _leafTextNode = nullptr;
    // What appending the text ran may have stopped the parser.
    return !IsStopped();
  }

#pragma endregion
}
