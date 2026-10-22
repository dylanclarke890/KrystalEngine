#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::html
{
  class HTMLElement;
  class HTMLFormElement;

  /// @brief An element that can have a form owner.
  /// @see https://html.spec.whatwg.org/#form-associated-element
  class FormAssociatedElement
  {
    KRYS_NON_COPY_MOVABLE(FormAssociatedElement);

  private:
    WeakPtr<HTMLFormElement> _form;
    WeakPtr<HTMLFormElement> _formSetByParser;

  public:
    virtual ~FormAssociatedElement() noexcept;

    KRYS_NODISCARD virtual HTMLElement &AsHTMLElement() noexcept = 0;
    KRYS_NODISCARD virtual const HTMLElement &AsHTMLElement() const noexcept = 0;

    KRYS_NODISCARD virtual bool IsFormListedElement() const noexcept = 0;

    virtual void FormWillBeDestroyed() noexcept
    {
      _form = nullptr;
    }

    KRYS_NODISCARD HTMLFormElement *Form() const noexcept;

    /// @brief The form owner, as the `form` IDL attribute exposes it.
    // FIXME: Retargeting against the element's tree, once there are reference targets.
    KRYS_NODISCARD virtual RefPtr<HTMLFormElement> FormForBindings() const noexcept;

    void SetForm(HTMLFormElement *newForm) noexcept;

    /// @brief The form the parser associates the element with, its form element pointer, which it joins when
    /// it's inserted.
    void SetFormSetByParser(HTMLFormElement *form) noexcept;

    virtual void ElementInsertedIntoAncestor(dom::Element &element, bool connectedToDocument) noexcept;
    virtual void ElementRemovedFromAncestor(dom::Element &element, bool disconnectedFromDocument) noexcept;

    KRYS_NODISCARD virtual FormAssociatedElement *AsFormAssociatedElement() noexcept = 0;

  protected:
    FormAssociatedElement() noexcept = default;

    /// @see https://html.spec.whatwg.org/#reset-the-form-owner
    virtual void ResetFormOwner() noexcept = 0;

    virtual void SetFormInternal(HTMLFormElement *newForm) noexcept;
  };
}
