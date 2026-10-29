#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/FormListedElement.hpp"
#include "Krystal.Booey/HTML/HTMLPlugInElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlobjectelement
  class HTMLObjectElement : public HTMLPlugInElement, public FormListedElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLObjectElement);

  public:
    // FIXME: An object element using its fallback content is treated like a generic HTML element (it can
    // contain range end points), once there's fallback content.

    HTMLObjectElement(dom::Document &document) noexcept;
    ~HTMLObjectElement() noexcept override;

    KRYS_NODISCARD bool IsReplaced(const RenderStyle * = nullptr) const noexcept override
    {
      // FIXME: A plug-in replacement that creates a renderer makes the element not replaced.
      return true;
    }

#pragma region HTMLObjectElement - https://html.spec.whatwg.org/#htmlobjectelement

    /// @see https://html.spec.whatwg.org/#dom-object-data
    KRYS_NODISCARD dom::USVString Data() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-data
    void Data(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-type
    void Type(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept override;

    /// @see https://html.spec.whatwg.org/#dom-object-name
    void Name(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-width
    KRYS_NODISCARD dom::DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-width
    void Width(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-height
    KRYS_NODISCARD dom::DOMString Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-height
    void Height(dom::DOMString &&value) noexcept;

    // FIXME: The rest of the interface:
    // readonly attribute Document? contentDocument;
    // readonly attribute WindowProxy? contentWindow;
    // Document? getSVGDocument();

    /// @brief Objects are never candidates for constraint validation.
    /// @see https://html.spec.whatwg.org/#dom-cva-willvalidate
    KRYS_NODISCARD bool WillValidate() const noexcept final
    {
      return false;
    }

    /// @see https://html.spec.whatwg.org/#dom-cva-checkvalidity
    KRYS_NODISCARD static bool CheckValidity() noexcept
    {
      return true;
    }

    /// @see https://html.spec.whatwg.org/#dom-cva-reportvalidity
    KRYS_NODISCARD static bool ReportValidity() noexcept
    {
      return true;
    }

#pragma endregion

#pragma region HTMLObjectElement Obsolete members - https://html.spec.whatwg.org/#HTMLObjectElement-partial

    /// @see https://html.spec.whatwg.org/#dom-object-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-align
    void Align(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-archive
    KRYS_NODISCARD dom::DOMString Archive() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-archive
    void Archive(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-code
    KRYS_NODISCARD dom::DOMString Code() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-code
    void Code(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-declare
    KRYS_NODISCARD bool Declare() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-declare
    void Declare(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-hspace
    KRYS_NODISCARD uint32 HSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-hspace
    void HSpace(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-standby
    KRYS_NODISCARD dom::DOMString Standby() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-standby
    void Standby(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-vspace
    KRYS_NODISCARD uint32 VSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-vspace
    void VSpace(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-codebase
    KRYS_NODISCARD dom::DOMString CodeBase() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-codebase
    void CodeBase(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-codetype
    KRYS_NODISCARD dom::DOMString CodeType() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-codetype
    void CodeType(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-usemap
    KRYS_NODISCARD dom::DOMString UseMap() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-usemap
    void UseMap(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-border
    KRYS_NODISCARD dom::DOMString Border() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-border
    void Border(dom::DOMString &&value) noexcept;

#pragma endregion

    KRYS_NODISCARD bool IsFormListedElement() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsValidatedFormListedElement() const noexcept final
    {
      return false;
    }

    KRYS_NODISCARD bool IsEnumeratable() const noexcept final
    {
      return true;
    }

    bool AppendFormData(DOMFormData &formData) noexcept final;

    KRYS_NODISCARD HTMLElement &AsHTMLElement() noexcept final
    {
      return *this;
    }

    KRYS_NODISCARD const HTMLElement &AsHTMLElement() const noexcept final
    {
      return *this;
    }

    KRYS_NODISCARD FormAssociatedElement *AsFormAssociatedElement() noexcept final
    {
      return this;
    }

    KRYS_NODISCARD FormListedElement *AsFormListedElement() noexcept final
    {
      return this;
    }

    KRYS_NODISCARD ValidatedFormListedElement *AsValidatedFormListedElement() noexcept final
    {
      return nullptr;
    }

  protected:
    void OnInsert() noexcept override;
    void OnPostConnection() noexcept override;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;
    void OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;
    void OnAdopt(dom::Document &oldDocument) noexcept override;
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;

    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLObjectElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLObjectElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLObjectElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();