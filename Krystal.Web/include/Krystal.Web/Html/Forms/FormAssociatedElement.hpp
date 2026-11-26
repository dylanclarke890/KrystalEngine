#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"

namespace krys::web
{
  class Element;
  class HTMLElement;
  class HTMLFormElement;
}

namespace krys::web::html
{
  /// @brief An element that can have a form owner.
  /// @see https://html.spec.whatwg.org/multipage/forms.html#form-associated-element
  class FormAssociatedElement
  {
    KRYS_NON_COPY_MOVABLE(FormAssociatedElement);

  private:
    WeakPtr<HTMLFormElement> _form;
    WeakPtr<HTMLFormElement> _formSetByParser;

  public:
    virtual ~FormAssociatedElement() noexcept;

    /// @brief The element itself.
    KRYS_NODISCARD virtual HTMLElement &AsHTMLElement() noexcept = 0;

    KRYS_NODISCARD virtual const HTMLElement &AsHTMLElement() const noexcept = 0;

    /// @brief Whether it's a listed element (rather than an image).
    KRYS_NODISCARD virtual bool IsFormListedElement() const noexcept = 0;

    /// @brief Drops the form owner, as the form is destroyed.
    virtual void FormWillBeDestroyed() noexcept
    {
      _form = nullptr;
    }

    /// @brief The form owner, if any.
    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#form-owner
    KRYS_NODISCARD HTMLFormElement *Form() const noexcept;

    /// @brief The form owner, as the `form` IDL attribute exposes it: retargeted against the element, so a
    /// form in a shadow tree it reaches through a reference target shows as that tree's host.
    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-fae-form
    KRYS_NODISCARD virtual RefPtr<Element> FormForBindings() const noexcept;

    /// @brief Changes the form owner, if it's a different form.
    void SetForm(HTMLFormElement *newForm) noexcept;

    /// @brief The form the parser associates the element with, its form element pointer, which it joins when
    /// it's inserted.
    void SetFormSetByParser(HTMLFormElement *form) noexcept;

    /// @brief Called as the element is inserted or removed, to reset its form owner.
    virtual void ElementInsertedIntoAncestor(Element &element, bool connectedToDocument) noexcept;

    virtual void ElementRemovedFromAncestor(Element &element, bool disconnectedFromDocument) noexcept;

    /// @brief The element as a form-associated element.
    KRYS_NODISCARD virtual FormAssociatedElement *AsFormAssociatedElement() noexcept = 0;

  protected:
    FormAssociatedElement() noexcept = default;

    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#reset-the-form-owner
    virtual void ResetFormOwner() noexcept = 0;

    /// @brief Sets the form owner; overrides also leave the old form and join the new one.
    virtual void SetFormInternal(HTMLFormElement *newForm) noexcept;
  };
}
