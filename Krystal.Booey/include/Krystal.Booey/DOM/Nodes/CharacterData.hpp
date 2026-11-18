#pragma once

#include "Krystal.Booey/DOM/Nodes/Node.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/DOM/Types/NodeOrString.hpp"
#include "Krystal.Booey/HTML/Sanitizer/SetHTMLOptions.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>

namespace krys::boo::dom
{
  class TextAlgorithms;
}

namespace krys::boo
{
  class Document;

  /// @brief A node with data: a Text, Comment or ProcessingInstruction node.
  /// @see https://dom.spec.whatwg.org/#interface-characterdata
  class CharacterData : public Node
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CharacterData);

    friend class dom::TextAlgorithms;

  private:
    /// @see https://dom.spec.whatwg.org/#concept-cd-data
    DOMString _data;

  protected:
    CharacterData(Document &document, DOMString data, dom::NodeType type,
                  dom::NodeFlags flags = dom::NodeFlags::None) noexcept;

  public:
#pragma region CharacterData - https://dom.spec.whatwg.org/#characterdata

    /// @see https://dom.spec.whatwg.org/#dom-characterdata-data
    KRYS_NODISCARD const DOMString &Data() const noexcept
    {
      return _data;
    }

    /// @see https://dom.spec.whatwg.org/#dom-characterdata-data
    dom::ExceptionOr<void> Data(DOMString data) noexcept;

    /// @brief Whether the data is empty or only ASCII whitespace.
    KRYS_NODISCARD bool ContainsOnlyASCIIWhitespace() const noexcept
    {
      return std::ranges::all_of(_data, [](char8_t c) { return text::IsASCIIWhitespace(c); });
    }

    /// @see https://dom.spec.whatwg.org/#dom-characterdata-length
    KRYS_NODISCARD size_t Length() const noexcept
    {
      return _data.size();
    }

    /// @see https://dom.spec.whatwg.org/#dom-characterdata-substringdata
    KRYS_NODISCARD dom::ExceptionOr<DOMString> SubstringData(size_t offset, size_t count) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-characterdata-appenddata
    dom::ExceptionOr<void> AppendData(DOMString data) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-characterdata-insertdata
    dom::ExceptionOr<void> InsertData(size_t offset, DOMString data) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-characterdata-deletedata
    dom::ExceptionOr<void> DeleteData(size_t offset, size_t count) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-characterdata-replacedata
    dom::ExceptionOr<void> ReplaceData(size_t offset, size_t count, DOMString data) noexcept;

#pragma endregion

#pragma region Node

    /// @see https://dom.spec.whatwg.org/#dom-node-nodevalue
    KRYS_NODISCARD Maybe<DOMString> NodeValue() const noexcept final
    {
      return _data;
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-nodevalue
    dom::ExceptionOr<void> NodeValue(DOMString value) noexcept final
    {
      return Data(krys::move(value));
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-textcontent
    KRYS_NODISCARD Maybe<DOMString> TextContent() const noexcept final
    {
      return _data;
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-textcontent
    dom::ExceptionOr<void> TextContent(DOMString value) noexcept final
    {
      return Data(krys::move(value));
    }

#pragma endregion

#pragma region ChildNode Mixin - https://dom.spec.whatwg.org/#childnode

    /// @brief Inserts `nodes` just before `this`, while replacing strings in `nodes` with equivalent Text
    /// nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-before
    dom::ExceptionOr<void> Before(const List<dom::NodeOrString> &nodes) noexcept;

    /// @brief Inserts `nodes` just after `this`, while replacing strings in `nodes` with equivalent Text
    /// nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-after
    dom::ExceptionOr<void> After(const List<dom::NodeOrString> &nodes) noexcept;

    /// @brief Replaces `this` with `nodes`, while replacing strings in `nodes` with equivalent Text nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-replacewith
    dom::ExceptionOr<void> ReplaceWith(const List<dom::NodeOrString> &nodes) noexcept;

    /// @brief Removes `this`.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-remove
    dom::ExceptionOr<void> Remove() noexcept;

#pragma endregion

#pragma region NonDocumentTypeChildNode (DOM Parsing) - https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-parsing-and-serialization

    /// @brief Inserts before the node @p html, parsed as HTML in its parent's context, sanitized by @p
    /// options (the default configuration if it gives none) with unsafe content removed.
    /// @see https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-beforehtml
    dom::ExceptionOr<void> BeforeHTML(DOMStringView html, const html::SetHTMLOptions &options = {}) noexcept;

    /// @brief Inserts after the node @p html, parsed as HTML in its parent's context, sanitized by @p options
    /// (the default configuration if it gives none) with unsafe content removed.
    /// @see https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-afterhtml
    dom::ExceptionOr<void> AfterHTML(DOMStringView html, const html::SetHTMLOptions &options = {}) noexcept;

    /// @brief Replaces the node with @p html, parsed as HTML in its parent's context, sanitized by @p options
    /// (the default configuration if it gives none) with unsafe content removed.
    /// @see https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-replacewithhtml
    dom::ExceptionOr<void> ReplaceWithHTML(DOMStringView html,
                                           const html::SetHTMLOptions &options = {}) noexcept;

    /// @brief Inserts before the node @p html, parsed as HTML in its parent's context; sanitized only if @p
    /// options gives a sanitizer.
    /// @see https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-beforehtmlunsafe
    dom::ExceptionOr<void> BeforeHTMLUnsafe(DOMStringView html,
                                            const html::SetHTMLUnsafeOptions &options = {}) noexcept;

    /// @brief Inserts after the node @p html, parsed as HTML in its parent's context; sanitized only if @p
    /// options gives a sanitizer.
    /// @see https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-afterhtmlunsafe
    dom::ExceptionOr<void> AfterHTMLUnsafe(DOMStringView html,
                                           const html::SetHTMLUnsafeOptions &options = {}) noexcept;

    /// @brief Replaces the node with @p html, parsed as HTML in its parent's context; sanitized only if @p
    /// options gives a sanitizer.
    /// @see https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-replacewithhtmlunsafe
    dom::ExceptionOr<void> ReplaceWithHTMLUnsafe(DOMStringView html,
                                                 const html::SetHTMLUnsafeOptions &options = {}) noexcept;

#pragma endregion

#pragma region NonDocumentTypeChildNode Mixin - https://dom.spec.whatwg.org/#interface-nondocumenttypechildnode

    /// @brief Returns the first preceding sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-previouselementsibling
    KRYS_NODISCARD RefPtr<const Element> PreviousElementSibling() const noexcept;

    /// @brief Returns the first preceding sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-previouselementsibling
    KRYS_NODISCARD RefPtr<Element> PreviousElementSibling() noexcept;

    /// @brief Returns the first following sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-nextelementsibling
    KRYS_NODISCARD RefPtr<const Element> NextElementSibling() const noexcept;

    /// @brief Returns the first following sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-nextelementsibling
    KRYS_NODISCARD RefPtr<Element> NextElementSibling() noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::CharacterData)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &node) noexcept
  {
    return node.IsCharacterDataNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
