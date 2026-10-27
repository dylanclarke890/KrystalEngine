#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/CDATASection.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/DOMImplementation.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLTemplateElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLCharacterReference.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Booey/XML/Parser/XMLDocumentParser.hpp"
#include "Krystal.Booey/XML/Parser/XMLDocumentParserScope.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Array.hpp"
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
  namespace
  {
    int globalDescriptor = 0;
    bool didInitializeXMLParser = false;

    KRYS_NODISCARD dom::DOMStringView ToStringView(Span<const xmlChar> string) noexcept
    {
      return dom::DOMStringView(reinterpret_cast<const char8 *>(string.data()), string.size());
    }

    KRYS_NODISCARD dom::DOMStringView ToStringView(const xmlChar *string) noexcept
    {
      if (!string)
      {
        return {};
      }
      return dom::DOMStringView(reinterpret_cast<const char8 *>(string),
                                std::strlen(reinterpret_cast<const char *>(string)));
    }

    KRYS_NODISCARD dom::DOMString ToString(Span<const xmlChar> string) noexcept
    {
      return dom::DOMString(ToStringView(string));
    }

    KRYS_NODISCARD dom::DOMString ToString(const xmlChar *string) noexcept
    {
      return dom::DOMString(ToStringView(string));
    }

    KRYS_NODISCARD dom::DOMStringAtom ToAtomString(Span<const xmlChar> string) noexcept
    {
      return dom::DOMStringAtom(ToStringView(string));
    }

    KRYS_NODISCARD dom::DOMStringAtom ToAtomString(const xmlChar *string) noexcept
    {
      if (!string)
      {
        return dom::DOMStringAtom::Null();
      }
      return dom::DOMStringAtom(ToStringView(string));
    }

    int MatchFunc(const char *) noexcept
    {
      // Only match loads initiated due to uses of libxml2 from within XMLDocumentParser to avoid
      // interfering with client applications that also use libxml2.
      // http://bugs.webkit.org/show_bug.cgi?id=17353
      return XMLDocumentParserScope::CurrentCachedResourceLoader() ? 1 : 0;
    }

    void SwitchToUTF8(xmlParserCtxtPtr ctxt) noexcept
    {
      // Hack around libxml2's lack of encoding overide support by manually
      // resetting the encoding to UTF-8 before every chunk. Otherwise libxml
      // will detect <?xml version="1.0" encoding="<encoding name>"?> blocks
      // and switch encodings, causing the parse to fail.
      // FIXME: Can we just use XML_PARSE_IGNORE_ENC now?
      xmlSwitchEncoding(ctxt, XML_CHAR_ENCODING_UTF8);
    }

    KRYS_NODISCARD bool ShouldAllowExternalLoad(const url::URL &url) noexcept
    {
      utf8_stringview urlString = url.String();

      auto startsWithIgnoringASCIICase = [&](utf8_stringview prefix)
      {
        return urlString.size() >= prefix.size()
               && text::IsASCIICaselessEqual(urlString.substr(0, prefix.size()), prefix);
      };
      auto endsWithIgnoringASCIICase = [&](utf8_stringview suffix)
      {
        return urlString.size() >= suffix.size()
               && text::IsASCIICaselessEqual(urlString.substr(urlString.size() - suffix.size()), suffix);
      };

      // On non-Windows platforms libxml asks for this URL, the "XML_XML_DEFAULT_CATALOG", on initialization.
      if (urlString == u8"file:///etc/xml/catalog")
      {
        return false;
      }

      // On Windows, libxml computes a URL relative to where its DLL resides.
      if (startsWithIgnoringASCIICase(u8"file:///") && endsWithIgnoringASCIICase(u8"/etc/catalog"))
      {
        return false;
      }

      // The most common DTD. There isn't much point in hammering www.w3c.org by requesting this for every
      // XHTML document.
      if (startsWithIgnoringASCIICase(u8"http://www.w3.org/tr/xhtml"))
      {
        return false;
      }

      // Similarly, there isn't much point in requesting the SVG DTD.
      if (startsWithIgnoringASCIICase(u8"http://www.w3.org/graphics/svg"))
      {
        return false;
      }

      // FIXME: Same-origin loads of external entities and DTDs (upstream allows them through a synchronous
      // load), once there are origins and synchronous loads.
      return false;
    }

    void *OpenFunc(const char *) noexcept
    {
      krys_debug_assert(XMLDocumentParserScope::CurrentCachedResourceLoader());

      // FIXME: Load the resource synchronously through the scope's loader, once there are origins and
      // synchronous loads; until then every load libxml2 asks for reads nothing.
      return &globalDescriptor;
    }

    int ReadFunc(void *context, char *, int) noexcept
    {
      // Do 0-byte reads in case of a null descriptor
      krys_debug_assert(context == &globalDescriptor);
      return 0;
    }

    int WriteFunc(void *, const char *, int) noexcept
    {
      // Always just do 0-byte writes
      return 0;
    }

    int CloseFunc(void *) noexcept
    {
      return 0;
    }

    xmlExternalEntityLoader defaultEntityLoader {nullptr};
  }

  xmlParserInputPtr ExternalEntityLoader(const char *url, const char *id, xmlParserCtxtPtr context) noexcept
  {
    if (!ShouldAllowExternalLoad(
          url::URL(dom::DOMStringView(reinterpret_cast<const char8 *>(url), std::strlen(url)))))
    {
      return nullptr;
    }
    krys_assert_msg(defaultEntityLoader, "Missing call to InitializeXMLParser()");
    return defaultEntityLoader(url, id, context);
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
    defaultEntityLoader = xmlGetExternalEntityLoader();
    krys_assert_msg(defaultEntityLoader != ExternalEntityLoader,
                    "XMLDocumentParserScope was created too early");
  }

  Ref<XMLParserContext> XMLParserContext::CreateStringParser(xmlSAXHandlerPtr handlers,
                                                             void *userData) noexcept
  {
    InitializeXMLParser();

    xmlParserCtxtPtr parser = xmlCreatePushParserCtxt(handlers, nullptr, nullptr, 0, nullptr);
    parser->_private = userData;

    // Substitute entities.
    xmlCtxtUseOptions(parser, XML_PARSE_NOENT | XML_PARSE_HUGE);

    SwitchToUTF8(parser);

    return AdoptRef(*new XMLParserContext(parser));
  }

  // Chunk should be encoded in UTF-8
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

    // Substitute entities.
    // FIXME: Why is XML_PARSE_NODICT needed? This is different from what CreateStringParser does.
    xmlCtxtUseOptions(parser, XML_PARSE_NODICT | XML_PARSE_NOENT | XML_PARSE_HUGE);

    parser->_private = userData;

    return AdoptRef(*new XMLParserContext(parser));
  }

  // --------------------------------

  bool XMLDocumentParser::SupportsXMLVersion(dom::DOMStringView version) noexcept
  {
    return version == u8"1.0";
  }

  XMLDocumentParser::XMLDocumentParser(dom::Document &document) noexcept
      : DocumentParser(document), _currentNode(ShareRefPtr<dom::ContainerNode>(&document))
  {
  }

  XMLDocumentParser::XMLDocumentParser(dom::DocumentFragment &fragment,
                                       HashMap<dom::DOMStringAtom, dom::DOMStringAtom> &&prefixToNamespaceMap,
                                       dom::DOMStringAtom defaultNamespaceURI) noexcept
      : DocumentParser(fragment.NodeDocument()), _currentNode(ShareRefPtr<dom::ContainerNode>(&fragment)),
        _parsingFragment(true), _prefixToNamespaceMap(krys::move(prefixToNamespaceMap)),
        _defaultNamespaceURI(defaultNamespaceURI)
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
    // The XMLDocumentParser will always be detached before being destroyed.
    krys_debug_assert(_currentNodeStack.empty());
    krys_debug_assert(!_currentNode);
  }

  void XMLDocumentParser::DoWrite(const dom::DOMString &parseString) noexcept
  {
    krys_debug_assert(!IsDetached());
    if (!_context)
    {
      InitializeParserContext();
    }

    // Protect the libxml context from deletion during a callback
    RefPtr<XMLParserContext> context = _context;

    // libXML throws an error if you try to switch the encoding for an empty string.
    if (!parseString.empty())
    {
      // What runs under xmlParseChunk may cause the parser to detach; keep this alive until this function is
      // done.
      Ref<XMLDocumentParser> protectedThis = ShareRef(*this);

      XMLDocumentParserScope scope(&Document()->GetCachedResourceLoader());

      SwitchToUTF8(context->Context());
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

    KRYS_NODISCARD bool
      HandleNamespaceAttributes(List<Pair<dom::QualifiedName, dom::DOMString>> &prefixedAttributes,
                                const xmlChar **libxmlNamespaces, int numNamespaces) noexcept
    {
      auto namespaces = Span<const XMLSAX2Namespace>(
        reinterpret_cast<const XMLSAX2Namespace *>(libxmlNamespaces), static_cast<size_t>(numNamespaces));
      for (auto &xmlNamespace : namespaces)
      {
        auto namespaceQName = dom::DOMStringAtom(infra::NamespacePrefixes::XMLNS);
        auto namespaceURI = ToString(xmlNamespace.URI);
        if (xmlNamespace.Prefix)
        {
          dom::DOMString qualifiedName(u8"xmlns:");
          qualifiedName.append(ToStringView(xmlNamespace.Prefix));
          namespaceQName = dom::DOMStringAtom(qualifiedName);
        }

        auto result =
          dom::NameValidation::ValidateAndExtract(dom::DOMStringAtom(infra::Namespaces::XMLNS),
                                                  namespaceQName, dom::ValidateAndExtractContext::Attribute);
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

    KRYS_NODISCARD bool
      HandleElementAttributes(List<Pair<dom::QualifiedName, dom::DOMString>> &prefixedAttributes,
                              const xmlChar **libxmlAttributes, int numAttributes) noexcept
    {
      auto attributes = Span<const XMLSAX2Attributes>(
        reinterpret_cast<const XMLSAX2Attributes *>(libxmlAttributes), static_cast<size_t>(numAttributes));
      for (auto &attribute : attributes)
      {
        auto valueLength = static_cast<size_t>(attribute.End - attribute.Value);
        auto attrValue = ToString(Span<const xmlChar>(attribute.Value, valueLength));
        auto attrPrefix = ToStringView(attribute.Prefix);
        auto attrURI = attrPrefix.empty() ? dom::DOMStringAtom::Null() : ToAtomString(attribute.URI);
        auto attrQName = dom::DOMStringAtom::Null();
        if (attrPrefix.empty())
        {
          attrQName = ToAtomString(attribute.LocalName);
        }
        else
        {
          dom::DOMString qualifiedName(attrPrefix);
          qualifiedName.push_back(u8':');
          qualifiedName.append(ToStringView(attribute.LocalName));
          attrQName = dom::DOMStringAtom(qualifiedName);
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

    void SetAttributes(dom::Element &element,
                       List<Pair<dom::QualifiedName, dom::DOMString>> &attributeVector) noexcept
    {
      for (auto &[name, value] : attributeVector)
      {
        dom::ElementAlgorithms::SetAttributeValue(element, name.LocalName(), krys::move(value),
                                                  name.NamespacePrefix(), name.NamespaceURI());
      }
    }
  }

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

    if (_parsingFragment && uri == dom::DOMStringAtom::Null())
    {
      if (prefix != dom::DOMStringAtom::Null())
      {
        auto it = _prefixToNamespaceMap.find(prefix);
        uri = it != _prefixToNamespaceMap.end() ? it->second : dom::DOMStringAtom::Null();
      }
      else if (Is<svg::SVGElement>(_currentNode.get()) || localName == u8"svg")
      {
        uri = dom::DOMStringAtom(infra::Namespaces::SVG);
      }
      else
      {
        uri = _defaultNamespaceURI;
      }
    }

    dom::QualifiedName qName(uri, prefix, localName);

    auto &document = _currentNode->NodeDocument();
    RefPtr<html::CustomElementRegistry> registry =
      ShareRefPtr(html::CustomElementAlgorithms::LookUpCustomElementRegistry(*_currentNode));
    bool willConstructCustomElement =
      !_parsingFragment
      && html::CustomElementAlgorithms::LookupCustomElementDefinition(registry.get(), uri, localName);

    Maybe<html::CustomElementReactionStack> customElementReactionStack;
    if (willConstructCustomElement) KRYS_UNLIKELY
    {
      // FIXME: Increment the document's throw-on-dynamic-markup-insertion counter, once there's
      // document.write().
      html::EventLoop::Current().PerformMicrotaskCheckpoint();
      customElementReactionStack.emplace();
    }

    List<Pair<dom::QualifiedName, dom::DOMString>> prefixedAttributes;
    bool handledAttributes = HandleNamespaceAttributes(prefixedAttributes, libxmlNamespaces, numNamespaces);
    bool success = handledAttributes
                     ? HandleElementAttributes(prefixedAttributes, libxmlAttributes, numAttributes)
                     : false;

    auto newElement = dom::ElementFactory::Create(document, qName, dom::DOMStringAtom::Null(),
                                                  willConstructCustomElement, registry);

    if (!handledAttributes)
    {
      SetAttributes(*newElement, prefixedAttributes);
      StopParsing();
      return;
    }

    SetAttributes(*newElement, prefixedAttributes);
    if (!success)
    {
      StopParsing();
      return;
    }

    customElementReactionStack.reset();

    newElement->BeginParsingChildren();

    (void)dom::MutationAlgorithms::Append(*newElement, *_currentNode);
    if (!_currentNode) // What the insertion ran may have removed the current node.
    {
      return;
    }

    if (auto *templateElement = DynamicDowncast<html::HTMLTemplateElement>(newElement.get()))
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

    // What runs can detach the parser. Make sure this is not released before the end of this method.
    Ref<XMLDocumentParser> protectedThis = ShareRef(*this);

    if (!UpdateLeafTextNode())
    {
      return;
    }

    RefPtr<dom::ContainerNode> node = _currentNode;
    if (auto *element = DynamicDowncast<dom::Element>(node.get()))
    {
      element->FinishParsingChildren();
    }

    // FIXME: Run the scripts the parser inserts, once there's scripting.
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

    auto cdataSection = CreateRef<dom::CDATASection>(_currentNode->NodeDocument(), ToString(s));
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

    auto comment = CreateRef<dom::Comment>(_currentNode->NodeDocument(), ToString(s));
    (void)dom::MutationAlgorithms::Append(*comment, *_currentNode);
  }

  namespace
  {
    enum StandaloneInfo
    {
      StandaloneUnspecified = -2,
      NoXMlDeclaration,
      StandaloneNo,
      StandaloneYes
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

    void WarningHandler(void *closure, const char *message, ...)
    {
      va_list args;
      va_start(args, message);
      GetParser(closure)->Error(XMLErrors::Type::Warning, message, args);
      va_end(args);
    }

    void FatalErrorHandler(void *closure, const char *message, ...)
    {
      va_list args;
      va_start(args, message);
      GetParser(closure)->Error(XMLErrors::Type::Fatal, message, args);
      va_end(args);
    }

    void NormalErrorHandler(void *closure, const char *message, ...)
    {
      va_list args;
      va_start(args, message);
      GetParser(closure)->Error(XMLErrors::Type::NonFatal, message, args);
      va_end(args);
    }

    // Using a static entity and marking it XML_INTERNAL_PREDEFINED_ENTITY is
    // a hack to avoid malloc/free. Using a global variable like this could cause trouble
    // if libxml implementation details were to change
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
      if (it == entries.end() || it->Name != reference)
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
      // Unlike HTML parser, XML parser parses the content of named
      // entities. So we need to escape '&' and '<'.
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
      // Even though we must pass the length, libxml expects the entity string to be null terminated.
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
      SwitchToUTF8(ctxt);
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
        GetParser(closure)->SetIsXHTMLDocument(true); // controls if we replace entities or not.
      }
    }

    void IgnorableWhitespaceHandler(void *, const xmlChar *, int)
    {
      // nothing to do, but we need this to work around a crasher
      // http://bugzilla.gnome.org/show_bug.cgi?id=172255
      // http://bugs.webkit.org/show_bug.cgi?id=5792
    }
  }

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
    DocumentParser::StartParsing();
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
        // Tell libxml we're done.
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

    DocumentParser::StopParsing();
    if (Context())
    {
      xmlStopParser(Context());
    }
  }

  bool XMLDocumentParser::AppendFragmentSource(dom::DOMStringView chunk) noexcept
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
    EndDocument(); // Close any open text nodes.

    // No error if the chunk is well formed or it is not but we have no error.
    return Context()->wellFormed || !xmlCtxtGetLastError(Context());
  }

  // --------------------------------

  namespace
  {
    using AttributeParseState = Maybe<HashMap<dom::DOMString, dom::DOMString>>;

    void AttributesStartElementNsHandler(void *closure, const xmlChar *xmlLocalName, const xmlChar *,
                                         const xmlChar *, int, const xmlChar **, int numAttributes, int,
                                         const xmlChar **libxmlAttributes)
    {
      if (ToStringView(xmlLocalName) != u8"attrs")
      {
        return;
      }

      auto &state = *static_cast<AttributeParseState *>(static_cast<xmlParserCtxtPtr>(closure)->_private);

      state = HashMap<dom::DOMString, dom::DOMString> {};

      auto attributes = Span<const XMLSAX2Attributes>(
        reinterpret_cast<const XMLSAX2Attributes *>(libxmlAttributes), static_cast<size_t>(numAttributes));
      for (auto &attribute : attributes)
      {
        auto attrLocalName = ToString(attribute.LocalName);
        auto valueLength = static_cast<size_t>(attribute.End - attribute.Value);
        auto attrValue = ToString(Span<const xmlChar>(attribute.Value, valueLength));
        auto attrPrefix = ToString(attribute.Prefix);
        auto attrQName = attrPrefix.empty() ? attrLocalName : attrPrefix + u8":" + attrLocalName;

        state->insert_or_assign(krys::move(attrQName), krys::move(attrValue));
      }
    }
  }

  Maybe<HashMap<dom::DOMString, dom::DOMString>> ParseAttributes(dom::DOMStringView string) noexcept
  {
    dom::DOMString parseString(u8"<?xml version=\"1.0\"?><attrs ");
    parseString.append(string);
    parseString.append(u8" />");

    AttributeParseState attributes;

    xmlSAXHandler sax;
    std::memset(&sax, 0, sizeof(sax));
    sax.startElementNs = AttributesStartElementNsHandler;
    sax.initialized = XML_SAX2_MAGIC;

    auto parser = XMLParserContext::CreateStringParser(&sax, &attributes);

    XMLDocumentParserScope scope(nullptr);
    xmlParseChunk(parser->Context(), reinterpret_cast<const char *>(parseString.data()),
                  static_cast<int>(parseString.size()), 1);

    return attributes;
  }
}
