#pragma once

#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmltemplateelement
  class HTMLTemplateElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTemplateElement);

  private:
    Ref<dom::DocumentFragment> _content;

    // The content patching state, while the parser has the template open (processing instruction markers).
    RefPtr<dom::ContainerNode> _insertionTarget;
    RefPtr<dom::Node> _insertionStartMarker;
    RefPtr<dom::Node> _insertionEndMarker;

  public:
    HTMLTemplateElement(dom::Document &document) noexcept;

#pragma region HTMLTemplateElement - https://html.spec.whatwg.org/#htmltemplateelement

    /// @see https://html.spec.whatwg.org/#dom-template-content
    RefPtr<dom::DocumentFragment> Content() const noexcept;

    /// @brief The template contents: a document fragment, or the shadow root of the host a declarative shadow
    /// root was attached to (the template element is then never inserted).
    /// @see https://html.spec.whatwg.org/#template-contents
    KRYS_NODISCARD dom::DocumentFragment &TemplateContents() const noexcept
    {
      return *_content;
    }

    /// @brief Sets the template contents to the declarative shadow root @p shadow.
    void SetTemplateContents(dom::ShadowRoot &shadow) noexcept;

    /// @brief Where the parser puts this template's children instead of its template contents, when it
    /// patches content in: the parent of the markers its for attribute names.
    /// @see https://html.spec.whatwg.org/#insertion-target
    KRYS_NODISCARD dom::ContainerNode *InsertionTarget() const noexcept
    {
      return _insertionTarget.get();
    }

    /// @see https://html.spec.whatwg.org/#insertion-start-marker
    KRYS_NODISCARD dom::Node *InsertionStartMarker() const noexcept
    {
      return _insertionStartMarker.get();
    }

    /// @see https://html.spec.whatwg.org/#insertion-end-marker
    KRYS_NODISCARD dom::Node *InsertionEndMarker() const noexcept
    {
      return _insertionEndMarker.get();
    }

    /// @brief Sets the insertion target and markers to null, once the parser has closed the template.
    void ClearInsertionTarget() noexcept;

    /// @brief Finds the markers named by the for attribute in @p scope, and makes their parent the insertion
    /// target, removing what's between them; false if there aren't any.
    /// @see https://html.spec.whatwg.org/#prepare-content-patching
    KRYS_NODISCARD bool PrepareContentPatching(dom::ContainerNode &scope) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-htmlfor
    KRYS_NODISCARD dom::DOMString HtmlFor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-htmlfor
    void HtmlFor(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootmode
    KRYS_NODISCARD dom::DOMString ShadowRootMode() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootmode
    void ShadowRootMode(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootdelegatesfocus
    KRYS_NODISCARD bool ShadowRootDelegatesFocus() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootdelegatesfocus
    void ShadowRootDelegatesFocus(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootslotassignment
    KRYS_NODISCARD dom::DOMString ShadowRootSlotAssignment() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootslotassignment
    void ShadowRootSlotAssignment(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootclonable
    KRYS_NODISCARD bool ShadowRootClonable() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootclonable
    void ShadowRootClonable(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootserializable
    KRYS_NODISCARD bool ShadowRootSerialisable() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootserializable
    void ShadowRootSerialisable(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootcustomelementregistry
    KRYS_NODISCARD dom::DOMString ShadowRootCustomElementRegistry() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootcustomelementregistry
    void ShadowRootCustomElementRegistry(dom::DOMString value) noexcept;

#pragma endregion

  protected:
    void OnAdopt(dom::Document &oldDocument) noexcept override;

    /// @see https://html.spec.whatwg.org/#template-clone
    void OnClone(dom::Node &copy, bool subtree) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTemplateElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTemplateElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return Is<krys::boo::html::HTMLElement>(target)
           && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLTemplateElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();