/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

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
