#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::html
{
  class HTMLElement;
  class HTMLFormElement;

  /// @brief The behaviour of a custom element, written in C++: what a custom element's class is in script.
  /// A custom element definition creates one for each element it defines (when the element is created, or
  /// when it's upgraded), which the element holds for as long as it lives; the element's lifecycle callbacks
  /// are its virtual methods.
  /// @note The element stays the element it was created as, so that an element defined after it was created
  /// keeps its identity: state belongs here, and the element is reached through `Element()`.
  /// @see https://html.spec.whatwg.org/#custom-elements
  class CustomElement : public CanMakeWeakPtr<CustomElement>
  {
    KRYS_NON_COPY_MOVABLE(CustomElement);

  private:
    HTMLElement &_element;

  public:
    explicit CustomElement(HTMLElement &element) noexcept : _element(element)
    {
    }

    virtual ~CustomElement() noexcept = default;

    /// @brief The element this is the behaviour of.
    KRYS_NODISCARD HTMLElement &Element() const noexcept
    {
      return _element;
    }

    /// @brief The element became connected.
    /// @see https://html.spec.whatwg.org/#concept-custom-element-definition-lifecycle-callbacks
    virtual void ConnectedCallback() noexcept
    {
    }

    /// @brief The element became disconnected.
    virtual void DisconnectedCallback() noexcept
    {
    }

    /// @brief The element was moved within its document by `moveBefore()`, staying connected. By default, it
    /// was disconnected and connected again, as for a definition without this callback.
    virtual void ConnectedMoveCallback() noexcept
    {
      DisconnectedCallback();
      ConnectedCallback();
    }

    /// @brief The element was adopted into @p newDocument from @p oldDocument.
    virtual void AdoptedCallback(dom::Document &oldDocument, dom::Document &newDocument) noexcept
    {
      (void)oldDocument;
      (void)newDocument;
    }

    /// @brief One of the definition's observed attributes was set, changed or removed.
    /// @param oldValue Null if the attribute was added.
    /// @param newValue Null if the attribute was removed.
    virtual void AttributeChangedCallback(dom::DOMStringAtom localName, dom::DOMStringAtom oldValue,
                                          dom::DOMStringAtom newValue,
                                          dom::DOMStringAtom namespaceURI) noexcept
    {
      (void)localName;
      (void)oldValue;
      (void)newValue;
      (void)namespaceURI;
    }

    /// @brief A form-associated element's form owner changed to @p form (null if it has none now).
    virtual void FormAssociatedCallback(HTMLFormElement *form) noexcept
    {
      (void)form;
    }

    /// @brief A form-associated element's form was reset.
    virtual void FormResetCallback() noexcept
    {
    }

    /// @brief A form-associated element became disabled or enabled (by its own attribute or a fieldset).
    virtual void FormDisabledCallback(bool disabled) noexcept
    {
      (void)disabled;
    }
  };
}
