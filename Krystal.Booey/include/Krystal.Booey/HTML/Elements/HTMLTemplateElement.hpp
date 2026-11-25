#pragma once

#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/scripting.html#htmltemplateelement
  class HTMLTemplateElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTemplateElement);

  private:
    Ref<DocumentFragment> _content;

    // The content patching state, while the parser has the template open (processing instruction markers).
    RefPtr<ContainerNode> _insertionTarget;
    RefPtr<Node> _insertionStartMarker;
    RefPtr<Node> _insertionEndMarker;

  public:
    HTMLTemplateElement(Document &document) noexcept;

#pragma region HTMLTemplateElement - https://html.spec.whatwg.org/multipage/scripting.html#htmltemplateelement

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-template-content
    RefPtr<DocumentFragment> Content() const noexcept;

    /// @brief The template contents: a document fragment, or the shadow root of the host a declarative shadow
    /// root was attached to (the template element is then never inserted).
    /// @see https://html.spec.whatwg.org/multipage/scripting.html#template-contents
    KRYS_NODISCARD DocumentFragment &TemplateContents() const noexcept
    {
      return *_content;
    }

    /// @brief Sets the template contents to the declarative shadow root @p shadow.
    void SetTemplateContents(boo::ShadowRoot &shadow) noexcept;

    /// @brief Where the parser puts this template's children instead of its template contents, when it
    /// patches content in: the parent of the markers its for attribute names.
    /// @see https://html.spec.whatwg.org/multipage/scripting.html#insertion-target
    KRYS_NODISCARD ContainerNode *InsertionTarget() const noexcept
    {
      return _insertionTarget.get();
    }

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#insertion-start-marker
    KRYS_NODISCARD Node *InsertionStartMarker() const noexcept
    {
      return _insertionStartMarker.get();
    }

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#insertion-end-marker
    KRYS_NODISCARD Node *InsertionEndMarker() const noexcept
    {
      return _insertionEndMarker.get();
    }

    /// @brief Sets the insertion target and markers to null, once the parser has closed the template.
    void ClearInsertionTarget() noexcept;

    /// @brief Finds the markers named by the for attribute in @p scope, and makes their parent the insertion
    /// target, removing what's between them; false if there aren't any.
    /// @see https://html.spec.whatwg.org/multipage/scripting.html#prepare-content-patching
    KRYS_NODISCARD bool PrepareContentPatching(ContainerNode &scope) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-template-htmlfor
    KRYS_NODISCARD DOMString HtmlFor() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-template-htmlfor
    void HtmlFor(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-template-shadowrootmode
    KRYS_NODISCARD DOMString ShadowRootMode() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-template-shadowrootmode
    void ShadowRootMode(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-template-shadowrootdelegatesfocus
    KRYS_NODISCARD bool ShadowRootDelegatesFocus() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-template-shadowrootdelegatesfocus
    void ShadowRootDelegatesFocus(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-template-shadowrootslotassignment
    KRYS_NODISCARD DOMString ShadowRootSlotAssignment() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-template-shadowrootslotassignment
    void ShadowRootSlotAssignment(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-template-shadowrootclonable
    KRYS_NODISCARD bool ShadowRootClonable() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-template-shadowrootclonable
    void ShadowRootClonable(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-template-shadowrootserializable
    KRYS_NODISCARD bool ShadowRootSerialisable() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-template-shadowrootserializable
    void ShadowRootSerialisable(bool value) noexcept;

    /// @brief The `shadowrootreferencetarget` attribute: the reference target of the declarative shadow root
    /// the template makes, or null without one.
    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootreferencetarget
    KRYS_NODISCARD Maybe<DOMString> ShadowRootReferenceTarget() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootreferencetarget
    void ShadowRootReferenceTarget(Maybe<DOMString> value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-template-shadowrootcustomelementregistry
    KRYS_NODISCARD DOMString ShadowRootCustomElementRegistry() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-template-shadowrootcustomelementregistry
    void ShadowRootCustomElementRegistry(DOMString value) noexcept;

#pragma endregion

  protected:
    void OnAdopt(Document &oldDocument) noexcept override;

    /// @see https://html.spec.whatwg.org/#template-clone
    void OnClone(Node &copy, bool subtree) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLTemplateElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLTemplateElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return Is<krys::boo::HTMLElement>(target)
           && Downcast<krys::boo::HTMLElement>(target).IsHTMLTemplateElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
