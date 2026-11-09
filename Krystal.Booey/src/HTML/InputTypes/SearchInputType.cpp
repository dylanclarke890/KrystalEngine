/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2014-2018 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/HTML/InputTypes/SearchInputType.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/DOM/Events/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/HTML/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::html
{
  SearchInputType::SearchInputType(HTMLInputElement &element) noexcept
      : BaseTextInputType(Type::Search, element)
  {
    krys_debug_assert(NeedsShadowSubtree());
  }

  bool SearchInputType::NeedsContainer() const noexcept
  {
    return true;
  }

  void SearchInputType::CreateShadowSubtree() noexcept
  {
    krys_debug_assert(NeedsShadowSubtree());
    krys_debug_assert(!_resultsButton);
    krys_debug_assert(!_cancelButton);
    krys_debug_assert(Element());

    TextFieldInputType::CreateShadowSubtree();
    auto &document = Element()->NodeDocument();
    auto *container = ContainerElement();
    auto *textWrapper = InnerBlockElement();
    krys_debug_assert(container);
    krys_debug_assert(textWrapper);

    // Without the `results` attribute (which isn't standard), the results button is only a decoration.
    auto resultsButton = SearchFieldResultsButtonElement::Create(document);
    (void)container->InsertBefore(*resultsButton, textWrapper);
    resultsButton->SetUserAgentPart(css::UserAgentParts::WebkitSearchDecoration());
    _resultsButton = krys::move(resultsButton);

    auto cancelButton = SearchFieldCancelButtonElement::Create(document);
    (void)container->InsertBefore(*cancelButton, textWrapper->NextSibling());
    _cancelButton = krys::move(cancelButton);
  }

  HTMLElement *SearchInputType::ResultsButtonElement() const noexcept
  {
    return _resultsButton.get();
  }

  HTMLElement *SearchInputType::CancelButtonElement() const noexcept
  {
    return _cancelButton.get();
  }

  InputType::ShouldCallBaseEventHandler
    SearchInputType::HandleKeydownEvent(dom::KeyboardEvent &event) noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    if (!element.IsMutable())
    {
      return TextFieldInputType::HandleKeydownEvent(event);
    }

    if (event.KeyIdentifier() == u8"U+001B")
    {
      (void)element.SetValue({}, TextFieldEventBehavior::DispatchChangeEvent);
      event.SetDefaultHandled();
      return ShouldCallBaseEventHandler::Yes;
    }
    return TextFieldInputType::HandleKeydownEvent(event);
  }

  void SearchInputType::RemoveShadowSubtree() noexcept
  {
    TextFieldInputType::RemoveShadowSubtree();
    _resultsButton = nullptr;
    _cancelButton = nullptr;
  }

  void SearchInputType::DidSetValueByUserEdit() noexcept
  {
    krys_debug_assert(Element());
    // The cancel button's visibility follows whether the field is empty (RenderSearchField's
    // updateCancelButtonVisibility), which its style resolution decides.
    if (_cancelButton)
    {
      _cancelButton->InvalidateStyleInternal();
    }

    TextFieldInputType::DidSetValueByUserEdit();
  }

  bool SearchInputType::SizeShouldIncludeDecoration(int32, int32 &preferredSize) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    preferredSize = static_cast<int32>(element.Size());
    // https://html.spec.whatwg.org/multipage/input.html#the-size-attribute
    // If the attribute is present, then its value must be parsed using the rules for parsing non-negative
    // integers, and if the result is a number greater than zero, then the user agent should ensure that at
    // least that many characters are visible.
    auto size = element.GetAttribute(u8"size");
    if (!size)
    {
      return false;
    }
    if (auto parsedSize = Numbers::ParseNonNegativeInteger(*size); parsedSize.Success())
    {
      return static_cast<int32>(parsedSize.Value) == preferredSize;
    }
    return false;
  }

  float SearchInputType::DecorationWidth(float) const noexcept
  {
    float width = 0;
    if (_resultsButton && _resultsButton->ExistingComputedStyle())
    {
      // FIXME: Document what invariant holds to allow only using fixed logical widths?
      auto *renderStyle = _resultsButton->ExistingComputedStyle();
      if (auto fixedLogicalWidth = renderStyle->LogicalWidth().TryFixed())
      {
        width += fixedLogicalWidth->ResolveZoom(renderStyle->UsedZoomForLength());
      }
    }
    if (_cancelButton && _cancelButton->ExistingComputedStyle())
    {
      // FIXME: Document what invariant holds to allow only using fixed logical widths?
      auto *renderStyle = _cancelButton->ExistingComputedStyle();
      if (auto fixedLogicalWidth = renderStyle->LogicalWidth().TryFixed())
      {
        width += fixedLogicalWidth->ResolveZoom(renderStyle->UsedZoomForLength());
      }
    }
    return width;
  }

  void SearchInputType::SetValue(dom::DOMStringView sanitizedValue, bool valueChanged,
                                 TextFieldEventBehavior eventBehavior,
                                 TextControlSetValueSelection selection) noexcept
  {
    bool emptinessChanged = valueChanged && sanitizedValue.empty() != Element()->Value().empty();

    BaseTextInputType::SetValue(sanitizedValue, valueChanged, eventBehavior, selection);

    if (!emptinessChanged)
    {
      return;
    }

    if (_cancelButton)
    {
      _cancelButton->InvalidateStyleInternal();
    }
  }
}
