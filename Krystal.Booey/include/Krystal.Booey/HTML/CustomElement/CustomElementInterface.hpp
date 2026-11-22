/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 * Copyright (C) 2015-2025 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Core/Containers/HashSet.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo
{
  class Document;
  class Element;
  class HTMLElement;
  class HTMLFormElement;
}

namespace krys::boo::html
{
  class CustomElement;
  class CustomElementRegistry;

  /// @brief Creates the behaviour of a custom element for @p element; null if it can't (the element is then
  /// a failed custom element, as when a constructor throws).
  using CustomElementFactory = Func<UniquePtr<CustomElement>(HTMLElement &element)>;

  /// @brief What a custom element definition says besides its name and its behaviour: the attributes whose
  /// changes reach `AttributeChangedCallback`, and the features it disables or enables.
  struct CustomElementOptions
  {
    /// @see https://html.spec.whatwg.org/#concept-custom-element-definition-observed-attributes
    List<DOMStringAtom> ObservedAttributes;
    /// @brief `attachInternals()` throws for the elements it defines.
    /// @see https://html.spec.whatwg.org/#concept-custom-element-definition-disable-internals
    bool DisableInternals {false};
    /// @brief `attachShadow()` throws for the elements it defines.
    /// @see https://html.spec.whatwg.org/#concept-custom-element-definition-disable-shadow
    bool DisableShadow {false};
    /// @brief The elements it defines are form-associated custom elements.
    /// @see https://html.spec.whatwg.org/#concept-custom-element-definition-form-associated
    bool FormAssociated {false};
  };

  /// @brief A custom element definition: its name, the factory of its behaviour, and its options.
  /// @see https://html.spec.whatwg.org/#custom-element-definition
  class CustomElementInterface : public RefCounted<CustomElementInterface>
  {
  private:
    QualifiedName _name;
    CustomElementFactory _factory;
    HashSet<DOMStringAtom> _observedAttributes;
    bool _isElementInternalsDisabled : 1 {false};
    bool _isShadowDisabled : 1 {false};
    bool _isFormAssociated : 1 {false};

  public:
    CustomElementInterface(DOMStringAtom localName, CustomElementFactory &&factory,
                           const CustomElementOptions &options) noexcept;

    ~CustomElementInterface() noexcept;

    KRYS_NODISCARD static Ref<CustomElementInterface> Create(const DOMStringAtom &localName,
                                                             CustomElementFactory &&factory,
                                                             CustomElementOptions options) noexcept
    {
      return CreateRef<CustomElementInterface>(localName, krys::move(factory), krys::move(options));
    }

    /// @brief Creates an element for this definition synchronously; an `HTMLUnknownElement` that is a failed
    /// custom element if the factory gives no behaviour.
    /// @see https://dom.spec.whatwg.org/#concept-create-element (step 6.2.1, synchronous custom elements)
    KRYS_NODISCARD Ref<Element> ConstructElementWithFallback(Document &document,
                                                             CustomElementRegistry &registry,
                                                             const QualifiedName &name) noexcept;

    /// @brief Creates an element for this definition that isn't yet defined: upgrading it creates its
    /// behaviour.
    KRYS_NODISCARD Ref<HTMLElement> CreateElement(Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/#concept-upgrade-an-element
    void UpgradeElement(Element &element) noexcept;

    void InvokeConnectedCallback(Element &element) noexcept;

    void InvokeDisconnectedCallback(Element &element) noexcept;

    void InvokeConnectedMoveCallback(Element &element) noexcept;

    void InvokeAdoptedCallback(Element &element, Document &oldDocument, Document &newDocument) noexcept;

    void InvokeAttributeChangedCallback(Element &element, const QualifiedName &attributeName,
                                        const DOMStringAtom &oldValue,
                                        const DOMStringAtom &newValue) noexcept;

    void InvokeFormAssociatedCallback(Element &element, HTMLFormElement *form) noexcept;

    void InvokeFormResetCallback(Element &element) noexcept;

    void InvokeFormDisabledCallback(Element &element, bool isDisabled) noexcept;

    KRYS_NODISCARD bool ObservesAttribute(const DOMStringAtom &name) const noexcept
    {
      return _observedAttributes.contains(name);
    }

    KRYS_NODISCARD bool IsElementInternalsDisabled() const noexcept
    {
      return _isElementInternalsDisabled;
    }

    KRYS_NODISCARD bool IsShadowDisabled() const noexcept
    {
      return _isShadowDisabled;
    }

    KRYS_NODISCARD bool IsFormAssociated() const noexcept
    {
      return _isFormAssociated;
    }

    /// @see https://html.spec.whatwg.org/#concept-custom-element-definition-name
    KRYS_NODISCARD const QualifiedName &Name() const noexcept
    {
      return _name;
    }

  private:
    /// @brief Creates the element and its behaviour; null if the factory gives none or breaks the rules for
    /// a newly constructed element.
    KRYS_NODISCARD RefPtr<Element> TryToConstructCustomElement(Document &document,
                                                               CustomElementRegistry &registry,
                                                               const QualifiedName &name) noexcept;

    template <typename TFunction>
    void InvokeCallback(Element &element, TFunction function) noexcept;
  };
}
