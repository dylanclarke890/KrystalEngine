/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004-2024 Apple Inc. All rights reserved.
 *           (C) 2006 Alexey Proskuryakov (ap@nypop.com)
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

#include "Krystal.Booey/HTML/Forms/FormListedElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Internals/IdTargetObserver.hpp"
#include "Krystal.Booey/DOM/Internals/IdTargetObserverRegistry.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"

namespace krys::boo::html
{
  /// @brief Resets its element's form owner when the element its `form` attribute names changes.
  class FormAttributeTargetObserver final : private dom::IdTargetObserver
  {
  private:
    FormListedElement &_element;

  public:
    FormAttributeTargetObserver(dom::DOMStringAtom id, FormListedElement &element) noexcept;

  private:
    void IdTargetChanged(dom::Element &) noexcept override;
  };

  namespace
  {
    /// @brief The element with the ID @p id in @p element's tree.
    KRYS_NODISCARD dom::Element *ElementForId(const HTMLElement &element, dom::DOMStringView id) noexcept
    {
      auto &root = dom::TreeQueries::Root(const_cast<HTMLElement &>(element));
      if (auto *document = DynamicDowncast<dom::Document>(root))
      {
        return document->GetElementById(id).get();
      }
      if (auto *fragment = DynamicDowncast<dom::DocumentFragment>(root))
      {
        return fragment->GetElementById(id).get();
      }
      return nullptr;
    }

    KRYS_NODISCARD HTMLFormElement *FindAssociatedForm(const HTMLElement &element,
                                                       HTMLFormElement *currentAssociatedForm) noexcept
    {
      if (element.IsConnected())
      {
        if (auto formId = element.GetAttribute(u8"form"))
        {
          // The element is associated with the first element in its tree whose ID is the value of the form
          // attribute, so the tree's element by that ID wins over the given form.
          auto *newFormCandidate = DynamicDowncast<HTMLFormElement>(ElementForId(element, *formId));
          if (!newFormCandidate)
          {
            return nullptr;
          }
          if (&dom::TreeQueries::Root(element) == &dom::TreeQueries::Root(*newFormCandidate))
          {
            return newFormCandidate;
          }
        }
      }

      return currentAssociatedForm ? currentAssociatedForm
                                   : HTMLFormElement::FindClosestFormAncestor(element);
    }
  }

  FormListedElement::FormListedElement() noexcept = default;

  FormListedElement::~FormListedElement() noexcept = default;

  void FormListedElement::DidMoveToNewDocument() noexcept
  {
    auto &element = AsHTMLElement();
    if (element.HasAttribute(u8"form") && element.IsConnected())
    {
      ResetFormAttributeTargetObserver();
    }
  }

  void FormListedElement::ElementInsertedIntoAncestor(dom::Element &element,
                                                      bool connectedToDocument) noexcept
  {
    FormAssociatedElement::ElementInsertedIntoAncestor(element, connectedToDocument);
    if (!connectedToDocument)
    {
      return;
    }

    if (element.HasAttribute(u8"form"))
    {
      ResetFormAttributeTargetObserver();
    }
  }

  void FormListedElement::ElementRemovedFromAncestor(dom::Element &element,
                                                     bool disconnectedFromDocument) noexcept
  {
    krys_debug_assert(&AsHTMLElement() == &element);
    _formAttributeTargetObserver = nullptr;
    FormAssociatedElement::ElementRemovedFromAncestor(element, disconnectedFromDocument);
    if (disconnectedFromDocument && element.HasAttribute(u8"form"))
    {
      SetForm(nullptr);
      ResetFormOwner();
    }
  }

  void FormListedElement::FormOwnerRemovedFromTree(const dom::Node &formRoot) noexcept
  {
    krys_debug_assert(Form());

    auto formHasSameRootNode = [&]() -> Maybe<bool>
    {
      const dom::Node *rootNode = &AsHTMLElement();
      if (auto *currentForm = Form())
      {
        for (auto *ancestor = rootNode->ParentNode(); ancestor; ancestor = ancestor->ParentNode())
        {
          if (ancestor == currentForm)
          {
            return std::nullopt;
          }
          rootNode = ancestor;
        }
      }
      return rootNode == &formRoot;
    }();

    if (!formHasSameRootNode)
    {
      // Form is our ancestor so we don't need to reset our owner, we also no longer need an id observer since
      // we are no longer connected.
      _formAttributeTargetObserver = nullptr;
      return;
    }

    // We are no longer in the same tree as our form owner so clear our owner.
    if (!*formHasSameRootNode)
    {
      SetForm(nullptr);
    }
  }

  void FormListedElement::SetFormInternal(HTMLFormElement *newForm) noexcept
  {
    WillChangeForm();
    if (auto *oldForm = Form())
    {
      oldForm->UnregisterFormListedElement(*this);
    }
    FormAssociatedElement::SetFormInternal(newForm);
    if (newForm)
    {
      newForm->RegisterFormListedElement(*this);
    }
    DidChangeForm();
  }

  void FormListedElement::WillChangeForm() noexcept
  {
  }

  void FormListedElement::DidChangeForm() noexcept
  {
  }

  void FormListedElement::FormWillBeDestroyed() noexcept
  {
    krys_debug_assert(Form());
    if (!Form())
    {
      return;
    }

    WillChangeForm();
    FormAssociatedElement::FormWillBeDestroyed();
    DidChangeForm();
  }

  void FormListedElement::ResetFormOwner() noexcept
  {
    // FIXME: Telling the host about newly associated controls (Document::didAssociateFormControl), for
    // autofill.
    SetForm(FindAssociatedForm(AsHTMLElement(), Form()));
  }

  void FormListedElement::ParseAttribute(dom::DOMStringAtom localName,
                                         Maybe<dom::DOMStringView> value) noexcept
  {
    if (localName == u8"form")
    {
      ParseFormAttribute(value);
    }
  }

  void FormListedElement::ParseFormAttribute(Maybe<dom::DOMStringView> value) noexcept
  {
    auto &element = AsHTMLElement();
    if (!value)
    {
      // The form attribute removed. We need to reset form owner here.
      // Instead of SetForm(FindAssociatedForm(element, Form())), this effectively performs
      // SetForm(FindAssociatedForm(element, nullptr)), as the current form is obsolete and can't be used as a
      // fallback.
      SetForm(HTMLFormElement::FindClosestFormAncestor(element));
      _formAttributeTargetObserver = nullptr;
    }
    else
    {
      ResetFormOwner();
      if (element.IsConnected())
      {
        ResetFormAttributeTargetObserver();
      }
    }
  }

  bool FormListedElement::CustomError() const noexcept
  {
    return !_customValidationMessage.empty();
  }

  bool FormListedElement::HasBadInput() const noexcept
  {
    return false;
  }

  bool FormListedElement::PatternMismatch() const noexcept
  {
    return false;
  }

  bool FormListedElement::RangeOverflow() const noexcept
  {
    return false;
  }

  bool FormListedElement::RangeUnderflow() const noexcept
  {
    return false;
  }

  bool FormListedElement::StepMismatch() const noexcept
  {
    return false;
  }

  bool FormListedElement::TooShort() const noexcept
  {
    return false;
  }

  bool FormListedElement::TooLong() const noexcept
  {
    return false;
  }

  bool FormListedElement::TypeMismatch() const noexcept
  {
    return false;
  }

  bool FormListedElement::ComputeValidity() const noexcept
  {
    bool someError = TypeMismatch() || StepMismatch() || RangeUnderflow() || RangeOverflow() || TooShort()
                     || TooLong() || PatternMismatch() || ValueMissing() || HasBadInput() || CustomError();
    return !someError;
  }

  bool FormListedElement::ValueMissing() const noexcept
  {
    return false;
  }

  dom::DOMString FormListedElement::CustomValidationMessage() const noexcept
  {
    return _customValidationMessage;
  }

  dom::DOMString FormListedElement::ValidationMessage() const noexcept
  {
    return WillValidate() ? _customValidationMessage : dom::DOMString();
  }

  // https://html.spec.whatwg.org/#dom-cva-setcustomvalidity
  void FormListedElement::SetCustomValidity(dom::DOMStringView error) noexcept
  {
    // 1. Set error to the result of normalizing newlines given error.
    // 2. Set the custom validity error message to error.
    SetCustomValidationMessage(infra::StringAlgorithms::NormalizeNewlines(error));
  }

  void FormListedElement::SetCustomValidationMessage(dom::DOMStringView message) noexcept
  {
    _customValidationMessage = dom::DOMString(message);
  }

  void FormListedElement::ResetFormAttributeTargetObserver() noexcept
  {
    auto &element = AsHTMLElement();
    krys_assert(element.IsConnected());
    _formAttributeTargetObserver = CreateUnique<FormAttributeTargetObserver>(
      dom::DOMStringAtom(element.GetAttribute(u8"form").value_or(dom::DOMString())), *this);
  }

  void FormListedElement::FormAttributeTargetChanged() noexcept
  {
    ResetFormOwner();
  }

  dom::DOMString FormListedElement::Name() const noexcept
  {
    return AsHTMLElement().GetAttribute(u8"name").value_or(dom::DOMString());
  }

  FormAttributeTargetObserver::FormAttributeTargetObserver(dom::DOMStringAtom id,
                                                           FormListedElement &element) noexcept
      : IdTargetObserver(element.AsHTMLElement().TreeScope().IdTargetObserverRegistry(), id),
        _element(element)
  {
  }

  void FormAttributeTargetObserver::IdTargetChanged(dom::Element &) noexcept
  {
    _element.FormAttributeTargetChanged();
  }
}
