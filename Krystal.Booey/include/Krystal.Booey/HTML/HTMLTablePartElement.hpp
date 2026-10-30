#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  class HTMLTableElement;

  /// @brief The base of the table parts that share presentational hints: cells, columns, rows and row
  /// groups.
  class HTMLTablePartElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTablePartElement);

  protected:
    HTMLTablePartElement(dom::Document &document, HTMLElementInterface interface) noexcept;

  public:
    /// @brief The nearest ancestor table.
    KRYS_NODISCARD const HTMLTableElement *FindParentTable() const noexcept;

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}
namespace krys::boo::html::detail
{
  /// @brief A new HTML element named @p localName in @p document, for the table members that create rows,
  /// cells, sections and captions.
  KRYS_NODISCARD Ref<HTMLElement> CreateTableElement(dom::Document &document,
                                                     dom::DOMStringView localName) noexcept;
}

namespace krys::boo::html
{
  /// @brief @p node as a @p TElement, if it's one.
  template <typename TElement, typename TNode>
  KRYS_NODISCARD match_constness_t<TNode, TElement> *AsTableElement(TNode *node) noexcept
  {
    auto *element = node ? DynamicDowncast<HTMLElement>(*node) : nullptr;
    return element ? DynamicDowncast<TElement>(*element) : nullptr;
  }

  /// @brief A new HTML element named @p localName in @p document, as a @p TElement.
  template <typename TElement>
  KRYS_NODISCARD Ref<TElement> CreateTableElement(dom::Document &document,
                                                  dom::DOMStringView localName) noexcept
  {
    auto element = detail::CreateTableElement(document, localName);
    return ShareRef(Downcast<TElement>(*element));
  }
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTablePartElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTablePartElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
