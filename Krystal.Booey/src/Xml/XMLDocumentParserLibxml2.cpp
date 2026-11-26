#include "Krystal.Booey/Dom/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/Dom/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/Dom/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/Dom/Internals/ElementFactory.hpp"
#include "Krystal.Booey/Dom/Nodes/CDATASection.hpp"
#include "Krystal.Booey/Dom/Nodes/Comment.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"
#include "Krystal.Booey/Dom/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/Dom/Nodes/DocumentType.hpp"
#include "Krystal.Booey/Dom/Nodes/DOMImplementation.hpp"
#include "Krystal.Booey/Dom/Nodes/ProcessingInstruction.hpp"
#include "Krystal.Booey/Dom/Nodes/Text.hpp"
#include "Krystal.Booey/Html/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/Html/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/Html/Elements/HTMLTemplateElement.hpp"
#include "Krystal.Booey/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Html/Parser/CharacterReference.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Svg/Elements/SVGElement.hpp"
#include "Krystal.Booey/Xml/XMLDocumentParser.hpp"
#include "Krystal.Booey/Xml/XMLDocumentParserScope.hpp"
#include "Krystal.Core/Containers/Array.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <libxml/entities.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/SAX2.h>
#include <libxml/xmlIO.h>
#include <mutex>

namespace krys::boo::xml
{
#pragma region libxml2 set-up

  namespace
  {
    /// @brief What libxml2 is handed as an open file: every read of it is empty.
    int globalDescriptor = 0;
    bool didInitializeXMLParser = false;

    KRYS_NODISCARD DOMStringView ToStringView(Span<const xmlChar> string) noexcept
    {
      return DOMStringView(reinterpret_cast<const char8 *>(string.data()), string.size());
    }

    KRYS_NODISCARD DOMStringView ToStringView(const xmlChar *string) noexcept
    {
      if (!string)
      {
        return {};
      }
      return DOMStringView(reinterpret_cast<const char8 *>(string),
                           std::strlen(reinterpret_cast<const char *>(string)));
    }

    KRYS_NODISCARD DOMString ToString(Span<const xmlChar> string) noexcept
    {
      return DOMString(ToStringView(string));
    }

    KRYS_NODISCARD DOMString ToString(const xmlChar *string) noexcept
    {
      return DOMString(ToStringView(string));
    }

    KRYS_NODISCARD DOMStringAtom ToAtomString(Span<const xmlChar> string) noexcept
    {
      return DOMStringAtom(ToStringView(string));
    }

    KRYS_NODISCARD DOMStringAtom ToAtomString(const xmlChar *string) noexcept
    {
      if (!string)
      {
        return null;
      }
      return DOMStringAtom(ToStringView(string));
    }

    int MatchFunc(const char *) noexcept
    {
      // Only loads asked for while Booey's parser runs, so an application's own use of libxml2 is left alone.
      return XMLDocumentParserScope::CurrentCachedResourceLoader() ? 1 : 0;
    }

    void *OpenFunc(const char *) noexcept
    {
      krys_debug_assert(XMLDocumentParserScope::CurrentCachedResourceLoader());

      // Every load libxml2 asks for reads nothing: there are no network loads.
      return &globalDescriptor;
    }

    int ReadFunc(void *context, char *, int) noexcept
    {
      (void)context;
      krys_debug_assert(context == &globalDescriptor);
      return 0;
    }

    int WriteFunc(void *, const char *, int) noexcept
    {
      return 0;
    }

    int CloseFunc(void *) noexcept
    {
      return 0;
    }
  }

  xmlParserInputPtr ExternalEntityLoader(const char *, const char *, xmlParserCtxtPtr) noexcept
  {
    // External entities and DTDs aren't loaded: there are no network loads.
    return nullptr;
  }

  void InitializeXMLParser() noexcept
  {
    if (didInitializeXMLParser)
    {
      return;
    }
    didInitializeXMLParser = true;

    xmlInitParser();
    xmlRegisterInputCallbacks(MatchFunc, OpenFunc, ReadFunc, CloseFunc);
    xmlRegisterOutputCallbacks(MatchFunc, OpenFunc, WriteFunc, CloseFunc);
  }

  Ref<XMLParserContext> XMLParserContext::CreateStringParser(xmlSAXHandlerPtr handlers,
                                                             void *userData) noexcept
  {
    InitializeXMLParser();

    xmlParserCtxtPtr parser = xmlCreatePushParserCtxt(handlers, nullptr, nullptr, 0, nullptr);
    parser->_private = userData;

    // Entities are replaced by their text, and the text is UTF-8 whatever an XML declaration says (see
    // DIFFERENCES.md).
    xmlCtxtUseOptions(parser, XML_PARSE_NOENT | XML_PARSE_HUGE | XML_PARSE_IGNORE_ENC);

    return AdoptRef(*new XMLParserContext(parser));
  }

  RefPtr<XMLParserContext> XMLParserContext::CreateMemoryParser(xmlSAXHandlerPtr handlers, void *userData,
                                                                const utf8_string &chunk) noexcept
  {
    InitializeXMLParser();

    // AppendFragmentSource() checks that the length doesn't overflow an int.
    xmlParserCtxtPtr parser =
      xmlCreateMemoryParserCtxt(reinterpret_cast<const char *>(chunk.data()), static_cast<int>(chunk.size()));

    if (!parser)
    {
      return nullptr;
    }

    std::memcpy(parser->sax, handlers, sizeof(xmlSAXHandler));

    // As for a document, but without libxml2's name dictionary.
    // FIXME: Find out whether XML_PARSE_NODICT is needed: a document's parser doesn't use it.
    xmlCtxtUseOptions(parser, XML_PARSE_NODICT | XML_PARSE_NOENT | XML_PARSE_HUGE | XML_PARSE_IGNORE_ENC);

    parser->_private = userData;

    return AdoptRef(*new XMLParserContext(parser));
  }

#pragma endregion

#pragma region Parsing

  bool XMLDocumentParser::SupportsXMLVersion(DOMStringView version) noexcept
  {
    return version == u8"1.0";
  }

  XMLDocumentParser::XMLDocumentParser(boo::Document &document) noexcept
      : _document(CreateWeakPtr(&document)), _currentNode(ShareRefPtr<ContainerNode>(&document))
  {
  }

  XMLDocumentParser::XMLDocumentParser(DocumentFragment &fragment,
                                       HashMap<DOMStringAtom, DOMStringAtom> &&prefixToNamespaceMap,
                                       DOMStringAtom defaultNamespaceURI) noexcept
      : _document(CreateWeakPtr(&fragment.NodeDocument())),
        _currentNode(ShareRefPtr<ContainerNode>(&fragment)), _parsingFragment(true),
        _prefixToNamespaceMap(krys::move(prefixToNamespaceMap)),
        _defaultNamespaceURI(krys::move(defaultNamespaceURI))
  {
  }

  XMLParserContext::~XMLParserContext() noexcept
  {
    if (_context->myDoc)
    {
      xmlFreeDoc(_context->myDoc);
    }
    xmlFreeParserCtxt(_context);
  }

  XMLDocumentParser::~XMLDocumentParser() noexcept
  {
    // A parser is always detached before it's destroyed.
    krys_debug_assert(!_document);
    krys_debug_assert(_currentNodeStack.empty());
    krys_debug_assert(!_currentNode);
  }

  void XMLDocumentParser::DoWrite(const DOMString &parseString) noexcept
  {
    krys_debug_assert(!IsDetached());
    if (!_context)
    {
      InitializeParserContext();
    }

    // Kept alive through the callbacks, which may detach the parser and drop its context.
    RefPtr<XMLParserContext> context = _context;

    // An empty string has nothing to parse.
    if (!parseString.empty())
    {
      Ref<XMLDocumentParser> protectedThis = ShareRef(*this);

      XMLDocumentParserScope scope(&Document()->GetCachedResourceLoader());

      xmlParseChunk(context->Context(), reinterpret_cast<const char *>(parseString.data()),
                    static_cast<int>(parseString.size()), 0);
    }
  }

  namespace
  {
    struct XMLSAX2Namespace
    {
      const xmlChar *Prefix;
      const xmlChar *URI;
    };

    KRYS_NODISCARD bool HandleNamespaceAttributes(List<Pair<QualifiedName, DOMString>> &prefixedAttributes,
                                                  const xmlChar **libxmlNamespaces,
                                                  int numNamespaces) noexcept
    {
      auto namespaces = Span<const XMLSAX2Namespace>(
        reinterpret_cast<const XMLSAX2Namespace *>(libxmlNamespaces), static_cast<size_t>(numNamespaces));
      for (auto &xmlNamespace : namespaces)
      {
        auto namespaceQName = DOMStringAtom(infra::namespace_prefixes::XMLNS);
        auto namespaceURI = ToString(xmlNamespace.URI);
        if (xmlNamespace.Prefix)
        {
          DOMString qualifiedName(u8"xmlns:");
          qualifiedName.append(ToStringView(xmlNamespace.Prefix));
          namespaceQName = DOMStringAtom(qualifiedName);
        }

        auto result = dom::NameValidation::ValidateAndExtract(infra::namespaces::XMLNS, namespaceQName,
                                                              dom::ValidateAndExtractContext::Attribute);
        if (result.HasException())
        {
          return false;
        }

        prefixedAttributes.emplace_back(result.ReleaseValue(), krys::move(namespaceURI));
      }
      return true;
    }

    struct XMLSAX2Attributes
    {
      const xmlChar *LocalName;
      const xmlChar *Prefix;
      const xmlChar *URI;
      const xmlChar *Value;
      const xmlChar *End;
    };

    KRYS_NODISCARD bool HandleElementAttributes(List<Pair<QualifiedName, DOMString>> &prefixedAttributes,
                                                const xmlChar **libxmlAttributes, int numAttributes) noexcept
    {
      auto attributes = Span<const XMLSAX2Attributes>(
        reinterpret_cast<const XMLSAX2Attributes *>(libxmlAttributes), static_cast<size_t>(numAttributes));
      for (auto &attribute : attributes)
      {
        auto valueLength = static_cast<size_t>(attribute.End - attribute.Value);
        auto attrValue = ToString(Span<const xmlChar>(attribute.Value, valueLength));
        auto attrPrefix = ToStringView(attribute.Prefix);
        auto attrURI = attrPrefix.empty() ? null : ToAtomString(attribute.URI);
        DOMStringAtom attrQName = null;
        if (attrPrefix.empty())
        {
          attrQName = ToAtomString(attribute.LocalName);
        }
        else
        {
          DOMString qualifiedName(attrPrefix);
          qualifiedName.push_back(u8':');
          qualifiedName.append(ToStringView(attribute.LocalName));
          attrQName = DOMStringAtom(qualifiedName);
        }

        auto result = dom::NameValidation::ValidateAndExtract(attrURI, attrQName,
                                                              dom::ValidateAndExtractContext::Attribute);
        if (result.HasException())
        {
          return false;
        }

        prefixedAttributes.emplace_back(result.ReleaseValue(), krys::move(attrValue));
      }
      return true;
    }

    void SetAttributes(Element &element, List<Pair<QualifiedName, DOMString>> &attributeVector) noexcept
    {
      for (auto &[name, value] : attributeVector)
      {
        dom::ElementAlgorithms::SetAttributeValue(element, name.LocalName(), krys::move(value),
                                                  name.NamespacePrefix(), name.NamespaceURI());
      }
    }
  }

#pragma endregion

#pragma region SAX callbacks

  void XMLDocumentParser::StartElementNs(const xmlChar *xmlLocalName, const xmlChar *xmlPrefix,
                                         const xmlChar *xmlURI, int numNamespaces,
                                         const xmlChar **libxmlNamespaces, int numAttributes, int,
                                         const xmlChar **libxmlAttributes) noexcept
  {
    if (IsStopped())
    {
      return;
    }

    if (!UpdateLeafTextNode())
    {
      return;
    }

    auto localName = ToAtomString(xmlLocalName);
    auto uri = ToAtomString(xmlURI);
    auto prefix = ToAtomString(xmlPrefix);

    if (_parsingFragment && uri == null)
    {
      if (prefix != null)
      {
        auto it = _prefixToNamespaceMap.find(prefix);
        uri = it != _prefixToNamespaceMap.end() ? it->second : null;
      }
      else if (Is<SVGElement>(_currentNode.get()) || localName == u8"svg")
      {
        uri = infra::namespaces::SVG;
      }
      else
      {
        uri = _defaultNamespaceURI;
      }
    }

    QualifiedName qName(uri, prefix, localName);

    auto &document = _currentNode->NodeDocument();
    auto *registry = html::CustomElementAlgorithms::LookUpCustomElementRegistry(*_currentNode);
    bool willConstructCustomElement =
      !_parsingFragment
      && html::CustomElementAlgorithms::LookupCustomElementDefinition(registry, uri, localName);

    Maybe<html::CustomElementReactionStack> customElementReactionStack;
    if (willConstructCustomElement) KRYS_UNLIKELY
    {
      // (The throw-on-dynamic-markup-insertion counter isn't kept: there's no document.write().)
      html::EventLoop::Current().PerformMicrotaskCheckpoint();
      customElementReactionStack.emplace();
    }

    List<Pair<QualifiedName, DOMString>> prefixedAttributes;
    bool handledAttributes = HandleNamespaceAttributes(prefixedAttributes, libxmlNamespaces, numNamespaces);
    bool success = handledAttributes
                     ? HandleElementAttributes(prefixedAttributes, libxmlAttributes, numAttributes)
                     : false;

    auto newElement =
      dom::ElementFactory::Create(document, qName, null, willConstructCustomElement, registry);

    // The attributes that were valid are set even when parsing stops at one that isn't.
    SetAttributes(*newElement, prefixedAttributes);
    if (!success)
    {
      StopParsing();
      return;
    }

    customElementReactionStack.reset();

    newElement->BeginParsingChildren();

    (void)dom::MutationAlgorithms::Append(*newElement, *_currentNode);
    // What inserting it ran may have taken the current node away.
    if (!_currentNode)
    {
      return;
    }

    if (auto *templateElement = DynamicDowncast<HTMLTemplateElement>(newElement.get()))
    {
      PushCurrentNode(templateElement->Content().get());
    }
    else
    {
      PushCurrentNode(newElement.get());
    }
  }

  void XMLDocumentParser::EndElementNs() noexcept
  {
    if (IsStopped())
    {
      return;
    }

    // Finishing the element can detach the parser.
    Ref<XMLDocumentParser> protectedThis = ShareRef(*this);

    if (!UpdateLeafTextNode())
    {
      return;
    }

    RefPtr<ContainerNode> node = _currentNode;
    if (auto *element = DynamicDowncast<Element>(node.get()))
    {
      element->FinishParsingChildren();
    }

    PopCurrentNode();
  }

  void XMLDocumentParser::Characters(Span<const xmlChar> characters) noexcept
  {
    if (IsStopped())
    {
      return;
    }

    if (!_leafTextNode)
    {
      CreateLeafTextNode();
    }
    _bufferedText.append(ToStringView(characters));
  }

  void XMLDocumentParser::Error(XMLErrors::Type type, const char *message, va_list args) noexcept
  {
    if (IsStopped())
    {
      return;
    }

    va_list preflightArgs;
    va_copy(preflightArgs, args);
    auto stringLength = static_cast<size_t>(std::vsnprintf(nullptr, 0, message, preflightArgs));
    va_end(preflightArgs);

    List<char> buffer(stringLength + 1);
    std::vsnprintf(buffer.data(), stringLength + 1, message, args);

    HandleError(type, buffer.data(), TextPosition());
  }

  void XMLDocumentParser::ProcessingInstruction(const xmlChar *target, const xmlChar *data) noexcept
  {
    if (IsStopped())
    {
      return;
    }

    if (!UpdateLeafTextNode())
    {
      return;
    }

    auto result = _currentNode->NodeDocument().CreateProcessingInstruction(ToString(target), ToString(data));
    if (result.HasException())
    {
      return;
    }
    auto pi = result.ReleaseValue();

    pi->SetCreatedByParser(true);

    (void)dom::MutationAlgorithms::Append(*pi, *_currentNode);

    pi->SetCreatedByParser(false);
  }

  void XMLDocumentParser::CdataBlock(Span<const xmlChar> s) noexcept
  {
    if (IsStopped())
    {
      return;
    }

    if (!UpdateLeafTextNode())
    {
      return;
    }

    auto cdataSection = CreateRef<CDATASection>(_currentNode->NodeDocument(), ToString(s));
    (void)dom::MutationAlgorithms::Append(*cdataSection, *_currentNode);
  }

  void XMLDocumentParser::Comment(const xmlChar *s) noexcept
  {
    if (IsStopped())
    {
      return;
    }

    if (!UpdateLeafTextNode())
    {
      return;
    }

    auto comment = CreateRef<boo::Comment>(_currentNode->NodeDocument(), ToString(s));
    (void)dom::MutationAlgorithms::Append(*comment, *_currentNode);
  }

  namespace
  {
    enum StandaloneInfo
    {
      StandaloneUnspecified = -2,
      NoXMlDeclaration,
      StandaloneNo,
      StandaloneYes,
    };
  }

  void XMLDocumentParser::StartDocument(const xmlChar *version, const xmlChar *encoding,
                                        int standalone) noexcept
  {
    auto standaloneInfo = static_cast<StandaloneInfo>(standalone);
    if (standaloneInfo == NoXMlDeclaration)
    {
      Document()->SetHasXMLDeclaration(false);
      return;
    }

    if (version)
    {
      Document()->SetXMLVersion(ToString(version));
    }
    if (standalone != StandaloneUnspecified)
    {
      Document()->SetXMLStandalone(standaloneInfo == StandaloneYes);
    }
    if (encoding)
    {
      Document()->SetXMLEncoding(ToString(encoding));
    }
    Document()->SetHasXMLDeclaration(true);
  }

  void XMLDocumentParser::EndDocument() noexcept
  {
    UpdateLeafTextNode();
  }

  void XMLDocumentParser::InternalSubset(const xmlChar *name, const xmlChar *externalID,
                                         const xmlChar *systemID) noexcept
  {
    if (IsStopped())
    {
      return;
    }

    if (auto *document = Document())
    {
      auto documentType = document->Implementation().CreateDocumentType(ToString(name), ToString(externalID),
                                                                        ToString(systemID));
      if (!documentType.HasException())
      {
        (void)dom::MutationAlgorithms::Append(*documentType.ReleaseValue(), *document);
      }
    }
  }

  namespace
  {
    KRYS_NODISCARD XMLDocumentParser *GetParser(void *closure) noexcept
    {
      auto ctxt = static_cast<xmlParserCtxtPtr>(closure);
      return static_cast<XMLDocumentParser *>(ctxt->_private);
    }

    void StartElementNsHandler(void *closure, const xmlChar *localname, const xmlChar *prefix,
                               const xmlChar *uri, int numNamespaces, const xmlChar **namespaces,
                               int numAttributes, int numDefaulted, const xmlChar **libxmlAttributes)
    {
      GetParser(closure)->StartElementNs(localname, prefix, uri, numNamespaces, namespaces, numAttributes,
                                         numDefaulted, libxmlAttributes);
    }

    void EndElementNsHandler(void *closure, const xmlChar *, const xmlChar *, const xmlChar *)
    {
      GetParser(closure)->EndElementNs();
    }

    void CharactersHandler(void *closure, const xmlChar *s, int len)
    {
      GetParser(closure)->Characters(Span<const xmlChar>(s, static_cast<size_t>(len)));
    }

    void ProcessingInstructionHandler(void *closure, const xmlChar *target, const xmlChar *data)
    {
      GetParser(closure)->ProcessingInstruction(target, data);
    }

    void CdataBlockHandler(void *closure, const xmlChar *s, int len)
    {
      GetParser(closure)->CdataBlock(Span<const xmlChar>(s, static_cast<size_t>(len)));
    }

    void CommentHandler(void *closure, const xmlChar *comment)
    {
      GetParser(closure)->Comment(comment);
    }

    // NOLINTNEXTLINE(modernize-avoid-variadic-functions): libxml2 callback.
    void WarningHandler(void *closure, const char *message, ...)
    {
      va_list args;
      va_start(args, message);
      GetParser(closure)->Error(XMLErrors::Type::Warning, message, args);
      va_end(args);
    }

    // NOLINTNEXTLINE(modernize-avoid-variadic-functions): libxml2 callback.
    void FatalErrorHandler(void *closure, const char *message, ...)
    {
      va_list args;
      va_start(args, message);
      GetParser(closure)->Error(XMLErrors::Type::Fatal, message, args);
      va_end(args);
    }

    // NOLINTNEXTLINE(modernize-avoid-variadic-functions): libxml2 callback.
    void NormalErrorHandler(void *closure, const char *message, ...)
    {
      va_list args;
      va_start(args, message);
      GetParser(closure)->Error(XMLErrors::Type::NonFatal, message, args);
      va_end(args);
    }

    // One static entity and buffer serve every lookup, so nothing is allocated or freed. That relies on
    // libxml2 not holding on to an entity from one lookup to the next.
    Array<xmlChar, 9> sharedXHTMLEntityResult = {};

    KRYS_NODISCARD xmlEntityPtr SharedXHTMLEntity() noexcept
    {
      static xmlEntity entity;
      if (!entity.type)
      {
        entity.type = XML_ENTITY_DECL;
        entity.orig = sharedXHTMLEntityResult.data();
        entity.content = sharedXHTMLEntityResult.data();
        entity.etype = XML_INTERNAL_PREDEFINED_ENTITY;
      }
      return &entity;
    }

    KRYS_NODISCARD Span<const char32> DecodeNamedHTMLEntityForXMLParser(const char *name) noexcept
    {
      string reference("&");
      reference.append(name);
      reference.push_back(';');
      auto &entries = html::NamedCharacterReferences;
      auto it = std::ranges::lower_bound(entries, stringview(reference), std::less {},
                                         &html::NamedCharacterReferenceEntry::Name);
      if (it == std::ranges::end(entries) || it->Name != reference)
      {
        return {};
      }
      return it->ToSpan();
    }

    KRYS_NODISCARD xmlEntityPtr GetXHTMLEntity(const xmlChar *name) noexcept
    {
      auto decodedEntity = DecodeNamedHTMLEntityForXMLParser(reinterpret_cast<const char *>(name));
      if (decodedEntity.empty())
      {
        return nullptr;
      }

      size_t entityLengthInUTF8 = 0;
      // libxml2 parses an entity's text as markup, so a `&` or `<` in it is given as a character reference.
      auto setResult = [&](utf8_stringview result)
      {
        std::ranges::copy(result, sharedXHTMLEntityResult.begin());
        entityLengthInUTF8 = result.size();
      };
      if (decodedEntity.size() == 1 && decodedEntity[0] == U'&')
      {
        setResult(u8"&#38;");
      }
      else if (decodedEntity.size() == 1 && decodedEntity[0] == U'<')
      {
        setResult(u8"&#x3C;");
      }
      else if (decodedEntity.size() == 2 && decodedEntity[0] == U'<' && decodedEntity[1] == 0x20D2)
      {
        setResult(u8"&#60;⃒");
      }
      else
      {
        StringBuilder builder;
        for (auto codePoint : decodedEntity)
        {
          builder.Append(codePoint);
        }
        auto utf8 = krys::move(builder).ToString();
        krys_debug_assert(utf8.size() < sharedXHTMLEntityResult.size());
        setResult(utf8);
      }
      // libxml2 takes the length but also expects the text null terminated.
      sharedXHTMLEntityResult[entityLengthInUTF8] = '\0';

      xmlEntityPtr entity = SharedXHTMLEntity();
      entity->length = static_cast<int>(entityLengthInUTF8);
      entity->name = name;
      return entity;
    }

    xmlEntityPtr GetEntityHandler(void *closure, const xmlChar *name)
    {
      auto ctxt = static_cast<xmlParserCtxtPtr>(closure);

      xmlEntityPtr ent = xmlGetPredefinedEntity(name);
      if (ent)
      {
        krys_assert(ent->etype == XML_INTERNAL_PREDEFINED_ENTITY);
        return ent;
      }

      ent = xmlGetDocEntity(ctxt->myDoc, name);
      if (!ent && GetParser(closure)->IsXHTMLDocument())
      {
        ent = GetXHTMLEntity(name);
        if (ent)
        {
          ent->etype = XML_INTERNAL_GENERAL_ENTITY;
        }
      }

      return ent;
    }

    void StartDocumentHandler(void *closure)
    {
      auto ctxt = static_cast<xmlParserCtxt *>(closure);
      GetParser(closure)->StartDocument(ctxt->version, ctxt->encoding, ctxt->standalone);
      xmlSAX2StartDocument(closure);
    }

    void EndDocumentHandler(void *closure)
    {
      GetParser(closure)->EndDocument();
      xmlSAX2EndDocument(closure);
    }

    void InternalSubsetHandler(void *closure, const xmlChar *name, const xmlChar *externalID,
                               const xmlChar *systemID)
    {
      GetParser(closure)->InternalSubset(name, externalID, systemID);
      xmlSAX2InternalSubset(closure, name, externalID, systemID);
    }

    // A doctype with one of the XHTML public identifiers gives the document HTML's named character references
    // (https://html.spec.whatwg.org/multipage/xhtml.html#parsing-xhtml-documents).
    void ExternalSubsetHandler(void *closure, const xmlChar *, const xmlChar *externalId, const xmlChar *)
    {
      auto extId = ToStringView(externalId);
      if ((extId == u8"-//W3C//DTD XHTML 1.0 Transitional//EN") || (extId == u8"-//W3C//DTD XHTML 1.1//EN")
          || (extId == u8"-//W3C//DTD XHTML 1.0 Strict//EN")
          || (extId == u8"-//W3C//DTD XHTML 1.0 Frameset//EN")
          || (extId == u8"-//W3C//DTD XHTML Basic 1.0//EN")
          || (extId == u8"-//W3C//DTD XHTML 1.1 plus MathML 2.0//EN")
          || (extId == u8"-//W3C//DTD XHTML 1.1 plus MathML 2.0 plus SVG 1.1//EN")
          || (extId == u8"-//W3C//DTD MathML 2.0//EN") || (extId == u8"-//WAPFORUM//DTD XHTML Mobile 1.0//EN")
          || (extId == u8"-//WAPFORUM//DTD XHTML Mobile 1.1//EN")
          || (extId == u8"-//WAPFORUM//DTD XHTML Mobile 1.2//EN"))
      {
        GetParser(closure)->SetIsXHTMLDocument(true);
      }
    }

    void IgnorableWhitespaceHandler(void *, const xmlChar *, int)
    {
      // Whitespace libxml2 reports as ignorable (in content a DTD makes element-only) isn't added to the
      // tree.
    }
  }

#pragma endregion

#pragma region Parser set-up and end

  void XMLDocumentParser::InitializeParserContext(const utf8_string &chunk) noexcept
  {
    xmlSAXHandler sax;
    std::memset(&sax, 0, sizeof(sax));

    sax.error = NormalErrorHandler;
    sax.fatalError = FatalErrorHandler;
    sax.characters = CharactersHandler;
    sax.processingInstruction = ProcessingInstructionHandler;
    sax.cdataBlock = CdataBlockHandler;
    sax.comment = CommentHandler;
    sax.warning = WarningHandler;
    sax.startElementNs = StartElementNsHandler;
    sax.endElementNs = EndElementNsHandler;
    sax.getEntity = GetEntityHandler;
    sax.startDocument = StartDocumentHandler;
    sax.endDocument = EndDocumentHandler;
    sax.internalSubset = InternalSubsetHandler;
    sax.externalSubset = ExternalSubsetHandler;
    sax.ignorableWhitespace = IgnorableWhitespaceHandler;
    sax.entityDecl = xmlSAX2EntityDecl;
    sax.initialized = XML_SAX2_MAGIC;
    _state = ParserState::Parsing;
    _sawError = false;

    if (_parsingFragment)
    {
      _context = XMLParserContext::CreateMemoryParser(&sax, this, chunk);
    }
    else
    {
      krys_debug_assert(chunk.empty());
      _context = XMLParserContext::CreateStringParser(&sax, this);
    }
  }

  void XMLDocumentParser::DoEnd() noexcept
  {
    if (!IsStopped())
    {
      if (_context)
      {
        // The last, empty, chunk ends the parse.
        {
          XMLDocumentParserScope scope(&Document()->GetCachedResourceLoader());
          xmlParseChunk(Context(), nullptr, 0, 1);
        }

        _context = nullptr;
      }
    }
  }

  SourceLocation XMLDocumentParser::TextPosition() const noexcept
  {
    xmlParserCtxtPtr context = Context();
    if (!context)
    {
      return {};
    }
    return {.Line = static_cast<size_t>(context->input->line),
            .Column = static_cast<size_t>(context->input->col)};
  }

  void XMLDocumentParser::StopParsing() noexcept
  {
    if (_sawError)
    {
      InsertErrorMessageBlock();
    }

    _state = ParserState::Stopped;
    if (Context())
    {
      xmlStopParser(Context());
    }
  }

  bool XMLDocumentParser::AppendFragmentSource(DOMStringView chunk) noexcept
  {
    krys_debug_assert(!_context);
    krys_debug_assert(_parsingFragment);

    utf8_string chunkAsUTF8(chunk);

    // libxml2 takes an int for a length, and therefore can't handle XML chunks larger than 2 GiB.
    if (chunkAsUTF8.size() > static_cast<size_t>(std::numeric_limits<int>::max()))
    {
      return false;
    }

    InitializeParserContext(chunkAsUTF8);
    if (!_context)
    {
      return false;
    }
    XMLDocumentParserScope scope(&Document()->GetCachedResourceLoader());
    xmlParseContent(Context());
    // Adds the text still buffered.
    EndDocument();

    // libxml2 can find a fragment not well formed without reporting an error; only a reported one counts.
    return Context()->wellFormed || !xmlCtxtGetLastError(Context());
  }

#pragma endregion
}
