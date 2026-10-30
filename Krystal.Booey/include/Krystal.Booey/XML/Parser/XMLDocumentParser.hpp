#pragma once

#include "Krystal.Booey/DOM/DocumentParser.hpp"
#include "Krystal.Booey/Types/SourceLocation.hpp"
#include "Krystal.Booey/XML/XMLErrors.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <cstdarg>

struct _xmlParserCtxt;
struct _xmlParserInput;
struct _xmlSAXHandler;
typedef unsigned char xmlChar;

namespace krys::boo::dom
{
  class ContainerNode;
  class DocumentFragment;
  class Element;
  class Text;
}

namespace krys::boo::xml
{
  class XMLParserContext : public RefCounted<XMLParserContext>
  {
  private:
    _xmlParserCtxt *_context;

  public:
    KRYS_NODISCARD static RefPtr<XMLParserContext> CreateMemoryParser(_xmlSAXHandler *, void *userData,
                                                                      const utf8_string &chunk) noexcept;
    KRYS_NODISCARD static Ref<XMLParserContext> CreateStringParser(_xmlSAXHandler *, void *userData) noexcept;
    XMLParserContext() = delete;
    ~XMLParserContext() noexcept;
    KRYS_NODISCARD _xmlParserCtxt *Context() const noexcept
    {
      return _context;
    }

  private:
    explicit XMLParserContext(_xmlParserCtxt *context) noexcept : _context(context)
    {
    }
  };

  /// @brief Parses an XML document, or a fragment of one, with libxml2.
  class XMLDocumentParser final : public dom::DocumentParser
  {
  private:
    RefPtr<XMLParserContext> _context;
    utf8_string _bufferedText;

    RefPtr<dom::ContainerNode> _currentNode;
    List<RefPtr<dom::ContainerNode>> _currentNodeStack;

    RefPtr<dom::Text> _leafTextNode;

    bool _sawError {false};
    bool _isXHTMLDocument {false};

    UniquePtr<XMLErrors> _xmlErrors;

    bool _parsingFragment {false};

    HashMap<dom::DOMStringAtom, dom::DOMStringAtom> _prefixToNamespaceMap;
    dom::DOMStringAtom _defaultNamespaceURI {dom::DOMStringAtom::Null()};

  public:
    KRYS_NODISCARD static Ref<XMLDocumentParser> Create(dom::Document &document) noexcept
    {
      return AdoptRef(*new XMLDocumentParser(document));
    }
    KRYS_NODISCARD static Ref<XMLDocumentParser>
      Create(dom::DocumentFragment &fragment,
             HashMap<dom::DOMStringAtom, dom::DOMStringAtom> &&prefixToNamespaceMap,
             dom::DOMStringAtom defaultNamespaceURI) noexcept
    {
      return AdoptRef(
        *new XMLDocumentParser(fragment, krys::move(prefixToNamespaceMap), defaultNamespaceURI));
    }

    XMLDocumentParser() = delete;
    ~XMLDocumentParser() noexcept override;

    // Exposed for callbacks:
    void HandleError(XMLErrors::Type, const char *message, SourceLocation) noexcept;

    void SetIsXHTMLDocument(bool isXHTML) noexcept
    {
      _isXHTMLDocument = isXHTML;
    }
    KRYS_NODISCARD bool IsXHTMLDocument() const noexcept
    {
      return _isXHTMLDocument;
    }

    static bool ParseDocumentFragment(dom::DOMStringView, dom::DocumentFragment &,
                                      dom::Element *parent = nullptr) noexcept;

    /// @brief Whether the document parsed was well formed.
    KRYS_NODISCARD bool WellFormed() const noexcept
    {
      return !_sawError;
    }

    KRYS_NODISCARD static bool SupportsXMLVersion(dom::DOMStringView) noexcept;

    void Append(dom::DOMString &&source) noexcept final;
    void Finish() noexcept final;
    void StopParsing() noexcept final;
    void Detach() noexcept final;

    KRYS_NODISCARD SourceLocation TextPosition() const noexcept;

  private:
    explicit XMLDocumentParser(dom::Document &) noexcept;
    XMLDocumentParser(dom::DocumentFragment &, HashMap<dom::DOMStringAtom, dom::DOMStringAtom> &&,
                      dom::DOMStringAtom) noexcept;

    void End() noexcept;

    bool AppendFragmentSource(dom::DOMStringView) noexcept;

  public:
    // Callbacks from parser SAX, and other functions needed inside
    // the parser implementation, but outside this class.

    void Error(XMLErrors::Type, const char *message, va_list args) noexcept;
    void StartElementNs(const xmlChar *xmlLocalName, const xmlChar *xmlPrefix, const xmlChar *xmlURI,
                        int numNamespaces, const xmlChar **namespaces, int numAttributes, int numDefaulted,
                        const xmlChar **libxmlAttributes) noexcept;
    void EndElementNs() noexcept;
    void Characters(Span<const xmlChar>) noexcept;
    void ProcessingInstruction(const xmlChar *target, const xmlChar *data) noexcept;
    void CdataBlock(Span<const xmlChar>) noexcept;
    void Comment(const xmlChar *) noexcept;
    void StartDocument(const xmlChar *version, const xmlChar *encoding, int standalone) noexcept;
    void InternalSubset(const xmlChar *name, const xmlChar *externalID, const xmlChar *systemID) noexcept;
    void EndDocument() noexcept;

  private:
    void InitializeParserContext(const utf8_string &chunk = {}) noexcept;

    void PushCurrentNode(dom::ContainerNode *) noexcept;
    void PopCurrentNode() noexcept;
    void ClearCurrentNodeStack() noexcept;

    void InsertErrorMessageBlock() noexcept;

    void CreateLeafTextNode() noexcept;
    bool UpdateLeafTextNode() noexcept;

    void DoWrite(const dom::DOMString &) noexcept;
    void DoEnd() noexcept;

    KRYS_NODISCARD _xmlParserCtxt *Context() const noexcept
    {
      return _context ? _context->Context() : nullptr;
    }
  };

  _xmlParserInput *ExternalEntityLoader(const char *url, const char *id, _xmlParserCtxt *) noexcept;
  void InitializeXMLParser() noexcept;

  /// @brief Parses @p string as the attributes of an element (a processing instruction's pseudo-attributes).
  KRYS_NODISCARD Maybe<HashMap<dom::DOMString, dom::DOMString>>
    ParseAttributes(dom::DOMStringView string) noexcept;
}
