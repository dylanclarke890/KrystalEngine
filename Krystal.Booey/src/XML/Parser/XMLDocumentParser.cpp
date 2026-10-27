#include "Krystal.Booey/XML/Parser/XMLDocumentParser.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

namespace krys::boo::xml
{
  namespace
  {
    constexpr size_t MaxXMLTreeDepth = 5'000;
  }

  void XMLDocumentParser::PushCurrentNode(dom::ContainerNode *n) noexcept
  {
    krys_debug_assert(n);
    krys_debug_assert(_currentNode);
    _currentNodeStack.push_back(std::exchange(_currentNode, ShareRefPtr(n)));
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

  void XMLDocumentParser::Append(dom::DOMString &&source) noexcept
  {
    if (IsStopped())
    {
      return;
    }

    DoWrite(source);
  }

  void XMLDocumentParser::HandleError(XMLErrors::Type type, const char *m, SourceLocation position) noexcept
  {
    if (!_xmlErrors)
    {
      _xmlErrors = CreateUnique<XMLErrors>(*Document());
    }
    _xmlErrors->HandleError(type, m, position);
    if (type != XMLErrors::Type::Warning)
    {
      _sawError = true;
    }
    if (type == XMLErrors::Type::Fatal)
    {
      StopParsing();
    }
  }

  void XMLDocumentParser::CreateLeafTextNode() noexcept
  {
    if (_leafTextNode)
    {
      return;
    }

    krys_debug_assert(_bufferedText.empty());
    _leafTextNode = CreateRef<dom::Text>(_currentNode->NodeDocument(), dom::DOMString());
    if (RefPtr<dom::ContainerNode> currentNode = _currentNode)
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

    RefPtr<dom::Text> leafTextNode = _leafTextNode;
    (void)leafTextNode->AppendData(krys::move(_bufferedText));
    _bufferedText.clear();

    _leafTextNode = nullptr;

    // Check again whether the parser is stopped, since what appending the data ran might have detached this
    // parser.
    return !IsStopped();
  }

  void XMLDocumentParser::Detach() noexcept
  {
    ClearCurrentNodeStack();
    DocumentParser::Detach();
  }

  void XMLDocumentParser::End() noexcept
  {
    // XMLDocumentParserLibxml2 will do bad things to the document if DoEnd() is called.
    krys_debug_assert(!_parsingFragment);

    DoEnd();

    // DoEnd() call above can detach the parser and null out its document.
    // In that case, we just bail out.
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
    Ref<dom::Document> document = ShareRef(*Document());
    document->FinishedParsing();
  }

  void XMLDocumentParser::Finish() noexcept
  {
    // FIXME: We should assert(!IsStopped()) here, since it does not make sense to call any methods on
    // DocumentParser once it's been stopped.

    Ref<XMLDocumentParser> protectedThis = ShareRef(*this);
    End();
  }

  void XMLDocumentParser::InsertErrorMessageBlock() noexcept
  {
    krys_debug_assert(_xmlErrors);
    _xmlErrors->InsertErrorMessageBlock();
  }

  namespace
  {
    struct XMLParsingNamespaces
    {
      dom::DOMStringAtom DefaultNamespace {dom::DOMStringAtom::Null()};
      HashMap<dom::DOMStringAtom, dom::DOMStringAtom> PrefixNamespaces;
    };

    KRYS_NODISCARD XMLParsingNamespaces FindXMLParsingNamespaces(dom::Element *contextElement) noexcept
    {
      if (!contextElement)
      {
        return {};
      }

      XMLParsingNamespaces result;

      result.DefaultNamespace = contextElement->LookupNamespaceURI(dom::DOMStringAtom::Null());

      for (auto *element = contextElement; element; element = element->ParentElement())
      {
        if (!element->HasAttributes())
        {
          continue;
        }
        for (auto &attribute : element->AttributeList())
        {
          if (attribute->Prefix() == infra::NamespacePrefixes::XMLNS)
          {
            result.PrefixNamespaces.insert_or_assign(attribute->LocalName(),
                                                     dom::DOMStringAtom(attribute->Value()));
          }
        }
      }

      return result;
    }
  }

  bool XMLDocumentParser::ParseDocumentFragment(dom::DOMStringView chunk, dom::DocumentFragment &fragment,
                                                dom::Element *contextElement) noexcept
  {
    if (chunk.empty())
    {
      return true;
    }

    // FIXME: We need to implement the HTML5 XML Fragment parsing algorithm:
    // http://www.whatwg.org/specs/web-apps/current-work/multipage/the-xhtml-syntax.html#xml-fragment-parsing-algorithm
    // For now we have a hack for script/style innerHTML support:
    if (contextElement
        && (contextElement->LocalName() == u8"script" || contextElement->LocalName() == u8"style"))
    {
      auto text = CreateRef<dom::Text>(fragment.NodeDocument(), dom::DOMString(chunk));
      (void)dom::MutationAlgorithms::Append(*text, fragment);
      return true;
    }

    auto namespaces = FindXMLParsingNamespaces(contextElement);
    auto parser = XMLDocumentParser::Create(fragment, krys::move(namespaces.PrefixNamespaces),
                                            namespaces.DefaultNamespace);
    bool wellFormed = parser->AppendFragmentSource(chunk);
    // Do not call Finish(). The Finish() and DoEnd() implementations touch the main document and loader and
    // can cause crashes in the fragment case.
    parser->Detach();  // Allows ~DocumentParser to assert it was detached before destruction.
    return wellFormed; // AppendFragmentSource()'s wellFormed is more permissive than Document's.
  }
}
