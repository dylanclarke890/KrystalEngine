#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Dom/Nodes/CharacterData.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"
#include "Krystal.Web/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Web/Loader/Cache/CachedStyleSheetClient.hpp"
#include "Krystal.Web/Xml/PseudoAttributes.hpp"

namespace krys::web::dom
{
  class NodeAlgorithms;
  class TextAlgorithms;
}

namespace krys::web
{
  class CSSStyleSheet;

  /// @brief A processing instruction (`<?target data?>`), whose data holds pseudo-attributes.
  /// @see https://dom.spec.whatwg.org/#interface-processinginstruction
  class ProcessingInstruction : public CharacterData, private CachedStyleSheetClient
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ProcessingInstruction);
    friend class Document;
    friend class dom::NodeAlgorithms;
    friend class dom::TextAlgorithms;

  private:
    /// @see https://dom.spec.whatwg.org/#concept-pi-target
    DOMString _target;
    /// @brief The pseudo-attributes, in order, kept in step with the data.
    /// @see https://dom.spec.whatwg.org/#concept-pi-attribute-map
    List<xml::PseudoAttribute> _attributeMap;
    // An xml-stylesheet's title and media pseudo-attributes, given to its sheet once it arrives.
    DOMString _title;
    DOMString _media;
    /// @brief The load of the sheet, until it arrives.
    CachedResourceHandle<CachedCSSStyleSheet> _cachedSheet;
    /// @see https://drafts.csswg.org/cssom/#associated-css-style-sheet
    RefPtr<CSSStyleSheet> _sheet;
    /// @brief Whether the document is waiting for the sheet.
    bool _loading {false};
    bool _alternate {false};
    bool _createdByParser {false};
    bool _isCSS {false};

  protected:
    /// @brief Initializes a processing instruction whose target and data are known to be valid.
    ProcessingInstruction(Document &document, DOMString target, DOMString data) noexcept;

  public:
    ~ProcessingInstruction() noexcept override;

#pragma region ProcessingInstruction - https://dom.spec.whatwg.org/#processinginstruction

    /// @brief Creates a processing instruction node in @p document; throws an "InvalidCharacterError" if the
    /// target isn't an XML name or the data contains `?>`.
    /// @see https://dom.spec.whatwg.org/#create-a-processing-instruction-node
    /// @see https://dom.spec.whatwg.org/#processinginstruction-initialize
    /// @see https://dom.spec.whatwg.org/#dom-processinginstruction-processinginstruction
    KRYS_NODISCARD static ExceptionOr<Ref<ProcessingInstruction>> Create(Document &document, DOMString target,
                                                                         DOMString data) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-processinginstruction-target
    KRYS_NODISCARD const DOMString &Target() const noexcept
    {
      return _target;
    }

    /// @see https://dom.spec.whatwg.org/#dom-processinginstruction-hasattributes
    KRYS_NODISCARD bool HasAttributes() const noexcept
    {
      return !_attributeMap.empty();
    }

    /// @see https://dom.spec.whatwg.org/#dom-processinginstruction-getattributenames
    KRYS_NODISCARD List<DOMString> GetAttributeNames() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-processinginstruction-getattribute
    /// @see https://dom.spec.whatwg.org/#get-a-processing-instruction-attribute
    KRYS_NODISCARD Maybe<DOMString> GetAttribute(DOMStringView name) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-processinginstruction-setattribute
    ExceptionOr<void> SetAttribute(DOMStringView name, DOMString value) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-processinginstruction-removeattribute
    void RemoveAttribute(DOMStringView name) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-processinginstruction-toggleattribute
    ExceptionOr<bool> ToggleAttribute(DOMStringView name, Maybe<bool> force = null) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-processinginstruction-hasattribute
    KRYS_NODISCARD bool HasAttribute(DOMStringView name) const noexcept;

#pragma endregion

#pragma region LinkStyle - https://drafts.csswg.org/cssom/#the-linkstyle-interface

    /// @see https://drafts.csswg.org/cssom/#dom-linkstyle-sheet
    KRYS_NODISCARD CSSStyleSheet *Sheet() const noexcept;

#pragma endregion

#pragma region Node - https://dom.spec.whatwg.org/#node

    /// @see - https://dom.spec.whatwg.org/#dom-node-nodename
    KRYS_NODISCARD DOMString NodeName() const noexcept final
    {
      return _target;
    }

#pragma endregion

    /// @brief Marks a processing instruction the parser made, which the style scope can add to the end of its
    /// sheets rather than work out its place in the tree.
    void SetCreatedByParser(bool createdByParser) noexcept
    {
      _createdByParser = createdByParser;
    }

    /// @brief Whether this is an `xml-stylesheet` processing instruction for a CSS sheet.
    KRYS_NODISCARD bool IsCSS() const noexcept
    {
      return _isCSS;
    }

    /// @brief Loads the sheet an `xml-stylesheet` processing instruction (a child of the document) links to,
    /// when it's connected or its data changes.
    /// @see https://drafts.csswg.org/cssom/#requirements-on-user-agents-implementing-the-xml-stylesheet-processing-instruction
    void CheckStyleSheet() noexcept;

  protected:
    void OnInsert() noexcept override;

    void OnPostConnection() noexcept override;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

  private:
    /// @see https://dom.spec.whatwg.org/#update-attributes-from-data
    void UpdateAttributesFromData() noexcept;

    /// @see https://dom.spec.whatwg.org/#update-data-from-attributes
    void UpdateDataFromAttributes() noexcept;

    /// @brief Makes the sheet from what arrived, with the title, media and alternate flag.
    void SetCSSStyleSheet(const utf8_string &href, const net::Url &baseUrl, utf8_stringview charset,
                          const CachedCSSStyleSheet *) noexcept override;

    /// @brief Whether the sheet or one of its imports is still loading.
    KRYS_NODISCARD bool IsLoading() const noexcept;

    bool SheetLoaded() noexcept override;

    /// @brief Parses the text that arrived into the sheet, ending the load.
    void ParseStyleSheet(const utf8_string &sheet) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::ProcessingInstruction)
  KRYS_NODISCARD static bool IsType(const krys::web::Node &node) noexcept
  {
    return node.IsProcessingInstructionNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
