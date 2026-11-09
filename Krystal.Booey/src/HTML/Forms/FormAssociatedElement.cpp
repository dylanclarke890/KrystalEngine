/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/HTML/Forms/FormAssociatedElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"

namespace krys::boo::html
{
  FormAssociatedElement::~FormAssociatedElement() noexcept
  {
    krys_assert(!_form);
  }

  HTMLFormElement *FormAssociatedElement::Form() const noexcept
  {
    return _form.get();
  }

  void FormAssociatedElement::SetForm(HTMLFormElement *newForm) noexcept
  {
    if (_form.get() != newForm)
    {
      SetFormInternal(newForm);
    }
  }

  RefPtr<HTMLFormElement> FormAssociatedElement::FormForBindings() const noexcept
  {
    return ShareRefPtr(Form());
  }

  void FormAssociatedElement::SetFormSetByParser(HTMLFormElement *form) noexcept
  {
    _formSetByParser = CreateWeakPtr(form);
  }

  void FormAssociatedElement::SetFormInternal(HTMLFormElement *newForm) noexcept
  {
    krys_debug_assert(_form.get() != newForm);
    _form = CreateWeakPtr(newForm);
  }

  void FormAssociatedElement::ElementInsertedIntoAncestor(dom::Element &element, bool) noexcept
  {
    krys_debug_assert(&AsHTMLElement() == &element);
    if (_formSetByParser)
    {
      // The form could have been removed by a script during parsing.
      if (_formSetByParser->IsConnected())
      {
        SetForm(_formSetByParser.get());
      }
      _formSetByParser = nullptr;
    }

    if (_form && &dom::TreeQueries::Root(element) != &dom::TreeQueries::Root(*_form))
    {
      SetForm(nullptr);
    }
  }

  void FormAssociatedElement::ElementRemovedFromAncestor(dom::Element &element, bool) noexcept
  {
    krys_debug_assert(&AsHTMLElement() == &element);
    if (_form && &dom::TreeQueries::Root(element) != &dom::TreeQueries::Root(*_form))
    {
      SetForm(nullptr);
      ResetFormOwner();
    }
  }
}
