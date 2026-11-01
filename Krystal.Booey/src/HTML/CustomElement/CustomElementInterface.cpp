#include "Krystal.Booey/HTML/CustomElement/CustomElementInterface.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLMaybeFormAssociatedCustomElement.hpp"
#include "Krystal.Booey/HTML/HTMLUnknownElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::html
{
  CustomElementInterface::CustomElementInterface(dom::DOMStringAtom localName, CustomElementFactory &&factory,
                                                 CustomElementOptions &&options) noexcept
      : _name(dom::DOMStringAtom(infra::Namespaces::HTML), dom::DOMStringAtom::Null(), localName),
        _factory(krys::move(factory)), _isElementInternalsDisabled(options.DisableInternals),
        _isShadowDisabled(options.DisableShadow), _isFormAssociated(options.FormAssociated)
  {
    for (auto &name : options.ObservedAttributes)
    {
      _observedAttributes.insert(name);
    }
  }

  CustomElementInterface::~CustomElementInterface() noexcept = default;

  Ref<dom::Element> CustomElementInterface::ConstructElementWithFallback(
    dom::Document &document, CustomElementRegistry &registry, const dom::QualifiedName &name) noexcept
  {
    if (auto element = TryToConstructCustomElement(document, registry, name))
    {
      return AdoptRef(*element.release());
    }

    // FIXME: Report the failure, once there's a console.
    auto element = CreateRef<HTMLUnknownElement>(document);
    element->SetIsCustomElementUpgradeCandidate();
    element->SetIsFailedCustomElement();
    return element;
  }

  Ref<HTMLElement> CustomElementInterface::CreateElement(dom::Document &document) noexcept
  {
    auto element = CreateRef<HTMLMaybeFormAssociatedCustomElement>(document, _name.LocalName());
    if (_isFormAssociated)
    {
      element->SetInterfaceIsFormAssociated();
    }
    return element;
  }

  RefPtr<dom::Element> CustomElementInterface::TryToConstructCustomElement(
    dom::Document &document, CustomElementRegistry &registry, const dom::QualifiedName &name) noexcept
  {
    (void)registry;
    (void)name;
    krys_debug_assert(name.NamespaceURI() == infra::Namespaces::HTML);

    // As an element constructor does when it isn't upgrading, the element is defined (custom, with this
    // definition) before its behaviour is constructed.
    auto element = CreateElement(document);
    element->SetIsDefinedCustomElement(*this);

    auto customElement = _factory(*element);
    if (customElement == nullptr || &customElement->Element() != element.get())
    {
      return nullptr;
    }
    element->SetCustomElement(krys::move(customElement));

    // A newly constructed custom element must not have attributes, children or a parent, and must belong to
    // the document it's created in.
    if (element->HasAttributes() || element->HasChildNodes() || element->ParentNode() != nullptr
        || &element->NodeDocument() != &document)
    {
      return nullptr;
    }

    return element;
  }

  // https://html.spec.whatwg.org/#concept-upgrade-an-element
  void CustomElementInterface::UpgradeElement(dom::Element &element) noexcept
  {
    krys_debug_assert(element.TagQualifiedName().LocalName() == _name.LocalName()
                      && element.NamespaceURI() == _name.NamespaceURI());

    // 1. If element's custom element state is not "undefined" or "uncustomized", then return.
    // (Only an undefined element has a reaction queue to be upgraded from.)
    if (!element.IsCustomElementUpgradeCandidate())
    {
      return;
    }

    auto *htmlElement = DynamicDowncast<HTMLElement>(element);
    krys_debug_assert(htmlElement != nullptr);

    // 4. For each attribute in element's attribute list, in order, enqueue a custom element callback reaction
    //    with element, callback name "attributeChangedCallback", and « attribute's local name, null,
    //    attribute's value, attribute's namespace ».
    // 5. If element is connected, then enqueue a custom element callback reaction with element, callback name
    //    "connectedCallback", and « ».
    // (Before steps 2 and 3, as they're for upgrade candidates.)
    CustomElementReactionQueue::EnqueuePostUpgradeReactions(element);

    // 2. Set element's custom element definition to definition.
    // (The definition is the reaction queue's.)
    // 3. Set element's custom element state to "failed".
    element.SetIsFailedCustomElementWithoutClearingReactionQueue();

    // 6. Add element to the end of definition's construction stack.
    // 7. Let C be definition's constructor.
    // 8. Let previousRegistry be the surrounding agent's active custom element constructor map[C] with
    // default
    //    null.
    // 9. Set the surrounding agent's active custom element constructor map[C] to element's custom element
    //    registry.
    // (Skipped: the factory is given the element, so there's no construction stack or constructor map.)

    // 10. Run the following steps while catching any exceptions:
    // 10a. If definition's disable shadow is true and element's shadow root is non-null, then throw a
    //      "NotSupportedError" DOMException.
    if (_isShadowDisabled && dom::TreeQueries::IsShadowHost(element))
    {
      // 12. Finally, if the above steps threw an exception:
      // 12a. Set element's custom element definition to null.
      // 12b. Empty element's custom element reaction queue.
      // 12c. Rethrow the exception (thus terminating this algorithm).
      // FIXME: Report a NotSupportedError, once there's a console.
      element.ClearReactionQueueFromFailedCustomElement();
      return;
    }

    // 10b. Set element's custom element state to "precustomized".
    element.SetIsPrecustomizedCustomElement();

    if (_isFormAssociated)
    {
      Downcast<HTMLMaybeFormAssociatedCustomElement>(element).WillUpgradeFormAssociated();
    }

    // 10c. Let constructResult be the result of constructing C, with no arguments.
    // 10d. If SameValue(constructResult, element) is false, then throw a TypeError.
    auto customElement = _factory(*htmlElement);
    if (customElement == nullptr || &customElement->Element() != htmlElement)
    {
      // 12. Finally, if the above steps threw an exception: [...]
      // FIXME: Report the failure, once there's a console.
      element.SetIsFailedCustomElementWithoutClearingReactionQueue();
      element.ClearReactionQueueFromFailedCustomElement();
      return;
    }

    // 14. Set element's custom element state to "custom".
    // (Before step 13, whose callbacks are for a custom element.)
    element.SetCustomElement(krys::move(customElement));
    element.SetIsDefinedCustomElement(*this);

    // 13. If element is a form-associated custom element:
    // 13a. Reset the form owner of element. If element is associated with a form element, then enqueue a
    //      custom element callback reaction with element, callback name "formAssociatedCallback", and « the
    //      associated form ».
    // 13b. If element is disabled, then enqueue a custom element callback reaction with element, callback
    // name
    //      "formDisabledCallback", and « true ».
    if (_isFormAssociated)
    {
      CustomElementReactionStack customElementReactionStack;
      Downcast<HTMLMaybeFormAssociatedCustomElement>(element).DidUpgradeFormAssociated();
    }
  }

  template <typename TFunction>
  void CustomElementInterface::InvokeCallback(dom::Element &element, TFunction &&function) noexcept
  {
    auto *customElement = element.GetCustomElement();
    if (customElement == nullptr)
    {
      return;
    }

    // The callback is host code: the reactions of the methods it calls are its own.
    CustomElementReactionStackSuspension suspension;
    function(*customElement);
  }

  void CustomElementInterface::InvokeConnectedCallback(dom::Element &element) noexcept
  {
    InvokeCallback(element, [](CustomElement &customElement) { customElement.ConnectedCallback(); });
  }

  void CustomElementInterface::InvokeDisconnectedCallback(dom::Element &element) noexcept
  {
    InvokeCallback(element, [](CustomElement &customElement) { customElement.DisconnectedCallback(); });
  }

  void CustomElementInterface::InvokeConnectedMoveCallback(dom::Element &element) noexcept
  {
    InvokeCallback(element, [](CustomElement &customElement) { customElement.ConnectedMoveCallback(); });
  }

  void CustomElementInterface::InvokeAdoptedCallback(dom::Element &element, dom::Document &oldDocument,
                                                     dom::Document &newDocument) noexcept
  {
    InvokeCallback(element, [&](CustomElement &customElement)
                   { customElement.AdoptedCallback(oldDocument, newDocument); });
  }

  void CustomElementInterface::InvokeAttributeChangedCallback(dom::Element &element,
                                                              const dom::QualifiedName &attributeName,
                                                              dom::DOMStringAtom oldValue,
                                                              dom::DOMStringAtom newValue) noexcept
  {
    InvokeCallback(element,
                   [&](CustomElement &customElement)
                   {
                     customElement.AttributeChangedCallback(attributeName.LocalName(), oldValue, newValue,
                                                            attributeName.NamespaceURI());
                   });
  }

  void CustomElementInterface::InvokeFormAssociatedCallback(dom::Element &element,
                                                            HTMLFormElement *form) noexcept
  {
    InvokeCallback(element,
                   [&](CustomElement &customElement) { customElement.FormAssociatedCallback(form); });
  }

  void CustomElementInterface::InvokeFormResetCallback(dom::Element &element) noexcept
  {
    InvokeCallback(element, [](CustomElement &customElement) { customElement.FormResetCallback(); });
  }

  void CustomElementInterface::InvokeFormDisabledCallback(dom::Element &element, bool isDisabled) noexcept
  {
    InvokeCallback(element,
                   [&](CustomElement &customElement) { customElement.FormDisabledCallback(isDisabled); });
  }

  void CustomElementInterface::InvokeFormStateRestoreCallback(dom::Element &element,
                                                              const CustomElementFormValue &state) noexcept
  {
    InvokeCallback(element, [&](CustomElement &customElement)
                   { customElement.FormStateRestoreCallback(state, FormStateRestoreMode::Restore); });
  }
}
