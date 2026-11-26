/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 * Copyright (C) 2015-2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Web/Html/CustomElement/CustomElementInterface.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Trees/TreeQueries.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElement.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Web/Html/Elements/HTMLMaybeFormAssociatedCustomElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLUnknownElement.hpp"
#include "Krystal.Web/Infra/Namespaces.hpp"

namespace krys::web::html
{
  CustomElementInterface::CustomElementInterface(DOMStringAtom localName, CustomElementFactory &&factory,
                                                 const CustomElementOptions &options) noexcept
      : _name(infra::namespaces::HTML, null, krys::move(localName)), _factory(krys::move(factory)),
        _isElementInternalsDisabled(options.DisableInternals), _isShadowDisabled(options.DisableShadow),
        _isFormAssociated(options.FormAssociated)
  {
    for (auto &name : options.ObservedAttributes)
    {
      _observedAttributes.insert(name);
    }
  }

  CustomElementInterface::~CustomElementInterface() noexcept = default;

  Ref<Element> CustomElementInterface::ConstructElementWithFallback(Document &document,
                                                                    CustomElementRegistry &registry,
                                                                    const QualifiedName &name) noexcept
  {
    if (auto element = TryToConstructCustomElement(document, registry, name))
    {
      return AdoptRef(*element.release());
    }

    auto element = CreateRef<HTMLUnknownElement>(document);
    element->SetIsCustomElementUpgradeCandidate();
    element->SetIsFailedCustomElement();
    return element;
  }

  Ref<HTMLElement> CustomElementInterface::CreateElement(Document &document) noexcept
  {
    auto element = CreateRef<HTMLMaybeFormAssociatedCustomElement>(document, _name.LocalName());
    if (_isFormAssociated)
    {
      element->SetInterfaceIsFormAssociated();
    }
    return element;
  }

  RefPtr<Element> CustomElementInterface::TryToConstructCustomElement(Document &document,
                                                                      CustomElementRegistry &registry,
                                                                      const QualifiedName &name) noexcept
  {
    (void)registry;
    (void)name;
    krys_debug_assert(name.NamespaceURI() == infra::namespaces::HTML);

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

  // https://html.spec.whatwg.org/multipage/custom-elements.html#concept-upgrade-an-element
  void CustomElementInterface::UpgradeElement(Element &element) noexcept
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
    //    default null.
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
    //      name "formDisabledCallback", and « true ».
    if (_isFormAssociated)
    {
      CustomElementReactionStack customElementReactionStack;
      Downcast<HTMLMaybeFormAssociatedCustomElement>(element).DidUpgradeFormAssociated();
    }
  }

  template <typename TFunction>
  void CustomElementInterface::InvokeCallback(Element &element, TFunction function) noexcept
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

  void CustomElementInterface::InvokeConnectedCallback(Element &element) noexcept
  {
    InvokeCallback(element, [](CustomElement &customElement) { customElement.ConnectedCallback(); });
  }

  void CustomElementInterface::InvokeDisconnectedCallback(Element &element) noexcept
  {
    InvokeCallback(element, [](CustomElement &customElement) { customElement.DisconnectedCallback(); });
  }

  void CustomElementInterface::InvokeConnectedMoveCallback(Element &element) noexcept
  {
    InvokeCallback(element, [](CustomElement &customElement) { customElement.ConnectedMoveCallback(); });
  }

  void CustomElementInterface::InvokeAdoptedCallback(Element &element, Document &oldDocument,
                                                     Document &newDocument) noexcept
  {
    InvokeCallback(element, [&](CustomElement &customElement)
                   { customElement.AdoptedCallback(oldDocument, newDocument); });
  }

  void CustomElementInterface::InvokeAttributeChangedCallback(Element &element,
                                                              const QualifiedName &attributeName,
                                                              const DOMStringAtom &oldValue,
                                                              const DOMStringAtom &newValue) noexcept
  {
    InvokeCallback(element,
                   [&](CustomElement &customElement)
                   {
                     customElement.AttributeChangedCallback(attributeName.LocalName(), oldValue, newValue,
                                                            attributeName.NamespaceURI());
                   });
  }

  void CustomElementInterface::InvokeFormAssociatedCallback(Element &element, HTMLFormElement *form) noexcept
  {
    InvokeCallback(element,
                   [&](CustomElement &customElement) { customElement.FormAssociatedCallback(form); });
  }

  void CustomElementInterface::InvokeFormResetCallback(Element &element) noexcept
  {
    InvokeCallback(element, [](CustomElement &customElement) { customElement.FormResetCallback(); });
  }

  void CustomElementInterface::InvokeFormDisabledCallback(Element &element, bool isDisabled) noexcept
  {
    InvokeCallback(element,
                   [&](CustomElement &customElement) { customElement.FormDisabledCallback(isDisabled); });
  }
}
