#pragma once

#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/Loader/Cache/CachedStyleSheetClient.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::css
{
  class CSSStyleSheet;
}

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#interface-processinginstruction
  class ProcessingInstruction : public CharacterData, private CachedStyleSheetClient
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ProcessingInstruction);
    friend class Document;
    friend class NodeAlgorithms;

  private:
    DOMString _target;
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
    ProcessingInstruction(Document &document, DOMString &&target, DOMString &&data) noexcept;

  public:
    ~ProcessingInstruction() noexcept override;

#pragma region ProcessingInstruction - https://dom.spec.whatwg.org/#processinginstruction
    /// @see - https://dom.spec.whatwg.org/#dom-processinginstruction-target
    KRYS_NODISCARD const DOMString &Target() const noexcept
    {
      return _target;
    }
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
