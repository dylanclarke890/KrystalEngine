#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElement.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  class Document;
  class Element;
}

namespace krys::boo::html
{
  class CustomElementRegistry;
  class HTMLElement;
  class HTMLFormElement;

  /// @brief Creates the behaviour of a custom element for @p element; null if it can't (the element is then
  /// a failed custom element, as when a constructor throws).
  using CustomElementFactory = Func<UniquePtr<CustomElement>(HTMLElement &element)>;

  /// @brief What a custom element definition says besides its name and its behaviour: the attributes whose
  /// changes reach `AttributeChangedCallback`, and the features it disables or enables.
  struct CustomElementOptions
  {
    /// @see https://html.spec.whatwg.org/#concept-custom-element-definition-observed-attributes
    List<dom::DOMStringAtom> ObservedAttributes;
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
    dom::QualifiedName _name;
    CustomElementFactory _factory;
    HashSet<dom::DOMStringAtom> _observedAttributes;
    bool _isElementInternalsDisabled : 1 {false};
    bool _isShadowDisabled : 1 {false};
    bool _isFormAssociated : 1 {false};

  public:
    CustomElementInterface(dom::DOMStringAtom localName, CustomElementFactory &&factory,
                           CustomElementOptions &&options) noexcept;
    ~CustomElementInterface() noexcept;

    KRYS_NODISCARD static Ref<CustomElementInterface> Create(dom::DOMStringAtom localName,
                                                             CustomElementFactory &&factory,
                                                             CustomElementOptions &&options) noexcept
    {
      return CreateRef<CustomElementInterface>(localName, krys::move(factory), krys::move(options));
    }

    /// @brief Creates an element for this definition synchronously; an `HTMLUnknownElement` that is a failed
    /// custom element if the factory gives no behaviour.
    /// @see https://dom.spec.whatwg.org/#concept-create-element (step 6.2.1, synchronous custom elements)
    KRYS_NODISCARD Ref<dom::Element> ConstructElementWithFallback(dom::Document &document,
                                                                  CustomElementRegistry &registry,
                                                                  const dom::QualifiedName &name) noexcept;

    /// @brief Creates an element for this definition that isn't yet defined: upgrading it creates its
    /// behaviour.
    KRYS_NODISCARD Ref<HTMLElement> CreateElement(dom::Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/#concept-upgrade-an-element
    void UpgradeElement(dom::Element &element) noexcept;

    void InvokeConnectedCallback(dom::Element &element) noexcept;
    void InvokeDisconnectedCallback(dom::Element &element) noexcept;
    void InvokeConnectedMoveCallback(dom::Element &element) noexcept;
    void InvokeAdoptedCallback(dom::Element &element, dom::Document &oldDocument,
                               dom::Document &newDocument) noexcept;
    void InvokeAttributeChangedCallback(dom::Element &element, const dom::QualifiedName &attributeName,
                                        dom::DOMStringAtom oldValue, dom::DOMStringAtom newValue) noexcept;
    void InvokeFormAssociatedCallback(dom::Element &element, HTMLFormElement *form) noexcept;
    void InvokeFormResetCallback(dom::Element &element) noexcept;
    void InvokeFormDisabledCallback(dom::Element &element, bool isDisabled) noexcept;

    KRYS_NODISCARD bool ObservesAttribute(dom::DOMStringAtom name) const noexcept
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
    KRYS_NODISCARD const dom::QualifiedName &Name() const noexcept
    {
      return _name;
    }

  private:
    /// @brief Creates the element and its behaviour; null if the factory gives none or breaks the rules for
    /// a newly constructed element.
    KRYS_NODISCARD RefPtr<dom::Element> TryToConstructCustomElement(dom::Document &document,
                                                                    CustomElementRegistry &registry,
                                                                    const dom::QualifiedName &name) noexcept;

    template <typename TFunction>
    void InvokeCallback(dom::Element &element, TFunction &&function) noexcept;
  };
}
