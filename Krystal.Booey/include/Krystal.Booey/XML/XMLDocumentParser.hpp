#pragma once

#include "Krystal.Booey/DOM/Parsing/DocumentParser.hpp"
#include "Krystal.Booey/XML/XMLErrors.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Containers/Span.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Utils/SourceLocation.hpp"
#include <cstdarg>

struct _xmlParserCtxt;
struct _xmlParserInput;
struct _xmlSAXHandler;
using xmlChar = unsigned char;

namespace krys::boo
{
  class ContainerNode;
  class DocumentFragment;
  class Element;
  class Text;
}

namespace krys::boo::xml
{
  /// @brief A libxml2 parser context, freed with the object.
  class XMLParserContext : public RefCounted<XMLParserContext>
  {
  private:
    _xmlParserCtxt *_context;

  public:
    /// @brief A parser for the whole of @p chunk, a fragment's UTF-8 text, given at once; null if libxml2
    /// can't make one.
    KRYS_NODISCARD static RefPtr<XMLParserContext> CreateMemoryParser(_xmlSAXHandler *, void *userData,
                                                                      const utf8_string &chunk) noexcept;

    /// @brief A parser for a document's UTF-8 text, given a chunk at a time.
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

  /// @brief Parses an XML document, or a fragment of one, with libxml2, building the tree from its SAX
  /// callbacks. A document that isn't well formed gets the errors shown in it (XMLErrors).
  /// @see https://html.spec.whatwg.org/#parsing-xhtml-documents
  class XMLDocumentParser final : public dom::DocumentParser
  {
  private:
    RefPtr<XMLParserContext> _context;
    /// @brief Character data not yet added to the text node it goes in.
    utf8_string _bufferedText;

    /// @brief The node the parser is adding to, and the nodes it'll return to.
    RefPtr<ContainerNode> _currentNode;
    List<RefPtr<ContainerNode>> _currentNodeStack;

    /// @brief The text node character data is being collected for.
    RefPtr<Text> _leafTextNode;

    /// @brief Whether an error (not a warning) was reported.
    bool _sawError {false};
    /// @brief Whether the doctype names an XHTML DTD, so HTML's named character references apply.
    bool _isXHTMLDocument {false};

    UniquePtr<XMLErrors> _xmlErrors;

    bool _parsingFragment {false};

    /// @brief The namespaces in scope on a fragment's context element.
    HashMap<DOMStringAtom, DOMStringAtom> _prefixToNamespaceMap;
    DOMStringAtom _defaultNamespaceURI {null};

  public:
    /// @brief A parser for @p document.
    KRYS_NODISCARD static Ref<XMLDocumentParser> Create(boo::Document &document) noexcept
    {
      return AdoptRef(*new XMLDocumentParser(document));
    }

    /// @brief A parser for @p fragment, with the given namespaces in scope.
    KRYS_NODISCARD static Ref<XMLDocumentParser>
      Create(DocumentFragment &fragment, HashMap<DOMStringAtom, DOMStringAtom> &&prefixToNamespaceMap,
             DOMStringAtom defaultNamespaceURI) noexcept
    {
      return AdoptRef(
        *new XMLDocumentParser(fragment, krys::move(prefixToNamespaceMap), krys::move(defaultNamespaceURI)));
    }

    XMLDocumentParser() = delete;

    ~XMLDocumentParser() noexcept override;

    /// @brief Parses @p source into @p fragment, with the namespaces in scope on @p contextElement.
    /// @return Whether @p source was well formed.
    /// @see https://html.spec.whatwg.org/#xml-fragment-parsing-algorithm
    static bool ParseDocumentFragment(DOMStringView source, DocumentFragment &fragment,
                                      Element *contextElement = nullptr) noexcept;

    /// @brief Whether the parser understands XML @p version (only 1.0).
    KRYS_NODISCARD static bool SupportsXMLVersion(DOMStringView version) noexcept;

#pragma region dom::DocumentParser

    void Append(DOMString source) noexcept final;

    void Finish() noexcept final;

    void StopParsing() noexcept final;

    void Detach() noexcept final;

#pragma endregion

    /// @brief Records an error or warning at @p position, stopping the parse if it's fatal.
    void HandleError(XMLErrors::Type, const char *message, SourceLocation position) noexcept;

    void SetIsXHTMLDocument(bool isXHTML) noexcept
    {
      _isXHTMLDocument = isXHTML;
    }

    KRYS_NODISCARD bool IsXHTMLDocument() const noexcept
    {
      return _isXHTMLDocument;
    }

    /// @brief Where the parser has got to in the source.
    KRYS_NODISCARD SourceLocation TextPosition() const noexcept;

#pragma region SAX callbacks

    // libxml2's callbacks, called from the parser's handlers with the same arguments.

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

#pragma endregion

  private:
    explicit XMLDocumentParser(boo::Document &) noexcept;

    XMLDocumentParser(DocumentFragment &, HashMap<DOMStringAtom, DOMStringAtom> &&, DOMStringAtom) noexcept;

    /// @brief Finishes the document: ends the parse, shows any errors, and tells the document it's parsed.
    void End() noexcept;

    /// @brief Parses a fragment's whole source.
    /// @return Whether it was well formed.
    bool AppendFragmentSource(DOMStringView) noexcept;

    /// @brief Makes the libxml2 parser: a memory parser over @p chunk for a fragment, a push parser for a
    /// document.
    void InitializeParserContext(const utf8_string &chunk = {}) noexcept;

    /// @brief Makes @p node the one the parser adds to, failing the parse if elements nest too deeply.
    void PushCurrentNode(ContainerNode *node) noexcept;

    void PopCurrentNode() noexcept;

    void ClearCurrentNodeStack() noexcept;

    void InsertErrorMessageBlock() noexcept;

    /// @brief Starts a text node for the character data that follows, if one isn't started.
    void CreateLeafTextNode() noexcept;

    /// @brief Adds the buffered character data to the text node and ends it.
    /// @return Whether parsing can go on.
    bool UpdateLeafTextNode() noexcept;

    void DoWrite(const DOMString &) noexcept;

    void DoEnd() noexcept;

    KRYS_NODISCARD _xmlParserCtxt *Context() const noexcept
    {
      return _context ? _context->Context() : nullptr;
    }
  };

  /// @brief libxml2's external entity loader while an XMLDocumentParserScope is active: loads only what
  /// Booey allows, through libxml2's own loader.
  _xmlParserInput *ExternalEntityLoader(const char *url, const char *id, _xmlParserCtxt *) noexcept;

  /// @brief Sets libxml2 up for Booey, once.
  void InitializeXMLParser() noexcept;
}
