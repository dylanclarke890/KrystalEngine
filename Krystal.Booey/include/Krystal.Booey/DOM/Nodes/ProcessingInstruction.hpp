#pragma once

#include "Krystal.Booey/DOM/Nodes/CharacterData.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/Loader/Cache/CachedStyleSheetClient.hpp"
#include "Krystal.Booey/XML/PseudoAttributes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class CSSStyleSheet;
}

namespace krys::boo::dom
{
  /// @brief A processing instruction (`<?target data?>`), whose data holds pseudo-attributes.
  /// @see https://dom.spec.whatwg.org/#interface-processinginstruction
  class ProcessingInstruction : public CharacterData, private CachedStyleSheetClient
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ProcessingInstruction);
    friend class Document;
    friend class NodeAlgorithms;
    friend class TextAlgorithms;

  private:
    /// @see https://dom.spec.whatwg.org/#concept-pi-target
    DOMString _target;
    /// @brief The pseudo-attributes, in order, kept in step with the data.
    /// @see https://dom.spec.whatwg.org/#concept-pi-attribute-map
    List<xml::PseudoAttribute> _attributeMap;
    DOMString _localHref;
    DOMString _title;
    DOMString _media;
    CachedResourceHandle<CachedCSSStyleSheet> _cachedSheet;
    RefPtr<css::CSSStyleSheet> _sheet;
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
    KRYS_NODISCARD css::CSSStyleSheet *Sheet() const noexcept;
#pragma endregion

#pragma region Node
    /// @see - https://dom.spec.whatwg.org/#dom-node-nodename
    KRYS_NODISCARD DOMString NodeName() const noexcept final
    {
      return _target;
    }
#pragma endregion

    void SetCreatedByParser(bool createdByParser) noexcept
    {
      _createdByParser = createdByParser;
    }

    KRYS_NODISCARD const DOMString &LocalHref() const noexcept
    {
      return _localHref;
    }

    /// @brief Whether this is an `xml-stylesheet` processing instruction for a CSS sheet.
    KRYS_NODISCARD bool IsCSS() const noexcept
    {
      return _isCSS;
    }

    /// @brief Loads the sheet an `xml-stylesheet` processing instruction (a child of the document) links to.
    /// @see https://www.w3.org/TR/xml-stylesheet/
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

    void SetCSSStyleSheet(const utf8_string &href, const url::URL &baseURL, utf8_stringview charset,
                          const CachedCSSStyleSheet *) noexcept override;

    KRYS_NODISCARD bool IsLoading() const noexcept;
    bool SheetLoaded() noexcept override;

    void ParseStyleSheet(const utf8_string &sheet) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::ProcessingInstruction)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsProcessingInstructionNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
