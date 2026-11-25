#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLPlugInElement.hpp"
#include "Krystal.Booey/HTML/Forms/FormListedElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#htmlobjectelement
  class HTMLObjectElement : public HTMLPlugInElement, public html::FormListedElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLObjectElement);

  public:
    // FIXME: An object element using its fallback content is treated like a generic HTML element (it can
    // contain range end points), once there's fallback content.

    HTMLObjectElement(Document &document) noexcept;

    ~HTMLObjectElement() noexcept override;

    KRYS_NODISCARD bool IsReplaced(const RenderStyle * = nullptr) const noexcept override
    {
      return true;
    }

#pragma region HTMLObjectElement - https://html.spec.whatwg.org/multipage/iframe-embed-object.html#htmlobjectelement

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-object-data
    KRYS_NODISCARD dom::USVString Data() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-object-data
    void Data(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-object-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-object-type
    void Type(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-object-name
    KRYS_NODISCARD DOMString Name() const noexcept override;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-object-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-object-width
    KRYS_NODISCARD DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-object-width
    void Width(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-object-height
    KRYS_NODISCARD DOMString Height() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-object-height
    void Height(DOMString value) noexcept;

    // (No `contentDocument`, `contentWindow` or `getSVGDocument()`: an object loads no nested document, as
    // subframes aren't planned; see OVERVIEW.md.)

    /// @brief Objects are never candidates for constraint validation.
    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-cva-willvalidate
    KRYS_NODISCARD bool WillValidate() const noexcept final
    {
      return false;
    }

    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-cva-checkvalidity
    KRYS_NODISCARD static bool CheckValidity() noexcept
    {
      return true;
    }

    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-cva-reportvalidity
    KRYS_NODISCARD static bool ReportValidity() noexcept
    {
      return true;
    }

#pragma endregion

#pragma region HTMLObjectElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLObjectElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-align
    void Align(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-archive
    KRYS_NODISCARD DOMString Archive() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-archive
    void Archive(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-code
    KRYS_NODISCARD DOMString Code() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-code
    void Code(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-declare
    KRYS_NODISCARD bool Declare() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-declare
    void Declare(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-hspace
    KRYS_NODISCARD uint32 HSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-hspace
    void HSpace(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-standby
    KRYS_NODISCARD DOMString Standby() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-standby
    void Standby(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-vspace
    KRYS_NODISCARD uint32 VSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-vspace
    void VSpace(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-codebase
    KRYS_NODISCARD DOMString CodeBase() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-codebase
    void CodeBase(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-codetype
    KRYS_NODISCARD DOMString CodeType() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-codetype
    void CodeType(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-usemap
    KRYS_NODISCARD DOMString UseMap() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-usemap
    void UseMap(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-border
    KRYS_NODISCARD DOMString Border() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-object-border
    void Border(DOMString value) noexcept;

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

    bool AppendFormData(html::DOMFormData &formData) noexcept final;

    KRYS_NODISCARD HTMLElement &AsHTMLElement() noexcept final
    {
      return *this;
    }

    KRYS_NODISCARD const HTMLElement &AsHTMLElement() const noexcept final
    {
      return *this;
    }

    KRYS_NODISCARD html::FormAssociatedElement *AsFormAssociatedElement() noexcept final
    {
      return this;
    }

    KRYS_NODISCARD html::FormListedElement *AsFormListedElement() noexcept final
    {
      return this;
    }

    KRYS_NODISCARD html::ValidatedFormListedElement *AsValidatedFormListedElement() noexcept final
    {
      return nullptr;
    }

  protected:
    void OnInsert() noexcept override;

    void OnPostConnection() noexcept override;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    void OnMove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    void OnAdopt(Document &oldDocument) noexcept override;

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept override;

    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLObjectElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::HTMLElement>(target).IsHTMLObjectElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLObjectElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
