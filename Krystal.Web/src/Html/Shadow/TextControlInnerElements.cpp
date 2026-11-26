/*
 * Copyright (C) 2006-2018 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Google Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Web/Html/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Web/Css/Selectors/UserAgentParts.hpp"
#include "Krystal.Web/Dom/Algorithms/TreeQueries.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Events/Pointer/MouseEvent.hpp"
#include "Krystal.Web/Html/Elements/HTMLInputElement.hpp"
#include "Krystal.Web/Html/Internals/BeforeTextInsertedEvent.hpp"
#include "Krystal.Web/Platform/LocalizedStrings.hpp"
#include "Krystal.Web/Rendering/FormControls/RenderTextControl.hpp"
#include "Krystal.Web/Rendering/FormControls/RenderTextControlSingleLine.hpp"
#include "Krystal.Web/Rendering/RenderTheme.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Web/Style/ResolvedStyle.hpp"

namespace krys::web::html
{
#pragma region TextControlInnerContainer

  TextControlInnerContainer::TextControlInnerContainer(Document &document) noexcept
      : HTMLDivElement(document, dom::NodeFlag::HasCustomStyleResolveCallbacks)
  {
  }

  Ref<TextControlInnerContainer> TextControlInnerContainer::Create(Document &document) noexcept
  {
    return CreateRef<TextControlInnerContainer>(document);
  }

  RenderPtr<RenderElement> TextControlInnerContainer::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderTextControlInnerContainer>(*this, krys::move(style));
  }

  Maybe<style::UnadjustedStyle>
    TextControlInnerContainer::ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                                                  const RenderStyle *shadowHostStyle) noexcept
  {
    // FIXME: Strong password fields' container wraps and clips (isStrongPasswordTextField), with autofill.
    auto style = ResolveStyle(resolutionContext);
    if (shadowHostStyle)
    {
      RenderTheme::Singleton().AdjustTextControlInnerContainerStyle(*style.Style, *shadowHostStyle,
                                                                    dom::TreeQueries::ShadowHost(*this));
    }
    return style;
  }

#pragma endregion

#pragma region TextControlInnerElement

  TextControlInnerElement::TextControlInnerElement(Document &document) noexcept
      : HTMLDivElement(document, dom::NodeFlag::HasCustomStyleResolveCallbacks)
  {
  }

  Ref<TextControlInnerElement> TextControlInnerElement::Create(Document &document) noexcept
  {
    return CreateRef<TextControlInnerElement>(document);
  }

  Maybe<style::UnadjustedStyle>
    TextControlInnerElement::ResolveCustomStyle(const style::ResolutionContext &,
                                                const RenderStyle *shadowHostStyle) noexcept
  {
    auto newStyle = RenderStyle::CreatePtr();
    newStyle->InheritFrom(*shadowHostStyle);
    newStyle->SetFlexGrow(style::FlexGrow {1});

    // Needed for correct shrinking.
    newStyle->SetLogicalMinWidth(style::MinimumSize::Fixed {0});

    newStyle->SetDisplay(DisplayType::Block);
    newStyle->SetDirection(text::TextDirection::LTR);
    // We don't want the shadow DOM to be editable, so we set this block to read-only in case the input itself
    // is editable.
    newStyle->SetUserModify(UserModify::ReadOnly);

    // FIXME: Strong password fields' inner block (isStrongPasswordTextField), with autofill.

    return style::UnadjustedStyle {krys::move(newStyle)};
  }

#pragma endregion

#pragma region TextControlInnerTextElement

  TextControlInnerTextElement::TextControlInnerTextElement(Document &document) noexcept
      : HTMLDivElement(document, dom::NodeFlag::HasCustomStyleResolveCallbacks)
  {
  }

  Ref<TextControlInnerTextElement> TextControlInnerTextElement::Create(Document &document,
                                                                       bool isEditable) noexcept
  {
    auto result = CreateRef<TextControlInnerTextElement>(document);
    result->UpdateInnerTextElementEditabilityImpl(isEditable);
    return result;
  }

  void TextControlInnerTextElement::UpdateInnerTextElementEditabilityImpl(bool isEditable) noexcept
  {
    (void)SetAttribute(u8"contenteditable", DOMString(isEditable ? u8"plaintext-only" : u8"false"));
  }

  void TextControlInnerTextElement::DefaultEventHandler(Event &event) noexcept
  {
    // FIXME: In the future, we should add a way to have default event listeners.
    // Then we would add one to the text field's inner div, and we wouldn't need this subclass.
    // Or possibly we could just use a normal event listener.
    if (event.IsBeforeTextInsertedEvent())
    {
      // A TextControlInnerTextElement can have no host if its been detached,
      // but kept alive by an EditCommand. In this case, an undo/redo can
      // cause events to be sent to the TextControlInnerTextElement. To
      // prevent an infinite loop, we must check for this case before sending
      // the event up the chain.
      if (RefPtr<Element> host = ShareRefPtr(dom::TreeQueries::ShadowHost(*this)))
      {
        host->DefaultEventHandler(event);
      }
    }
    if (!event.DefaultHandled())
    {
      HTMLDivElement::DefaultEventHandler(event);
    }
  }

  RenderPtr<RenderElement> TextControlInnerTextElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderTextControlInnerBlock>(*this, krys::move(style));
  }

  RenderTextControlInnerBlock *TextControlInnerTextElement::Renderer() const noexcept
  {
    return Downcast<RenderTextControlInnerBlock>(HTMLDivElement::Renderer());
  }

  Maybe<style::UnadjustedStyle>
    TextControlInnerTextElement::ResolveCustomStyle(const style::ResolutionContext &,
                                                    const RenderStyle *shadowHostStyle) noexcept
  {
    auto &shadowHost = *dom::TreeQueries::ShadowHost(*this);
    auto style = Downcast<HTMLTextFormControlElement>(shadowHost).CreateInnerTextStyle(*shadowHostStyle);

    RenderTheme::Singleton().AdjustTextControlInnerTextStyle(style, *shadowHostStyle, &shadowHost);

    return style::UnadjustedStyle {CreateUnique<RenderStyle>(krys::move(style))};
  }

#pragma endregion

#pragma region TextControlPlaceholderElement

  TextControlPlaceholderElement::TextControlPlaceholderElement(Document &document) noexcept
      : HTMLDivElement(document, dom::NodeFlag::HasCustomStyleResolveCallbacks)
  {
  }

  Ref<TextControlPlaceholderElement> TextControlPlaceholderElement::Create(Document &document) noexcept
  {
    auto element = CreateRef<TextControlPlaceholderElement>(document);
    element->SetUserAgentPart(css::user_agent_parts::Placeholder());
    return element;
  }

  Maybe<style::UnadjustedStyle>
    TextControlPlaceholderElement::ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                                                      const RenderStyle *shadowHostStyle) noexcept
  {
    auto style = ResolveStyle(resolutionContext);

    auto &controlElement =
      Downcast<HTMLTextFormControlElement>(*dom::TreeQueries::ContainingShadowRoot(*this)->Host());
    auto &styleStyle = *style.Style;
    styleStyle.SetDisplay(controlElement.IsPlaceholderVisible() ? DisplayType::Block : DisplayType::None);
    // The user agent sheet can't say this (`-webkit-text-security` isn't parsed), so a password field's mask
    // is turned off here.
    styleStyle.SetTextSecurity(TextSecurity::None);

    if (auto *inputElement = DynamicDowncast<HTMLInputElement>(controlElement))
    {
      styleStyle.SetTextOverflow(inputElement->ShouldTruncateText(*shadowHostStyle) ? TextOverflow::Ellipsis
                                                                                    : TextOverflow::Clip);
      styleStyle.SetPaddingTop(style::PaddingEdge::Fixed {0});
      styleStyle.SetPaddingBottom(style::PaddingEdge::Fixed {0});
    }

    RenderTheme::Singleton().AdjustTextControlInnerPlaceholderStyle(styleStyle, *shadowHostStyle,
                                                                    &controlElement);

    return style;
  }

#pragma endregion

#pragma region SearchFieldResultsButtonElement

  namespace
  {
    KRYS_NODISCARD bool
      SearchFieldStyleHasExplicitlySpecifiedTextFieldAppearance(const RenderStyle &style) noexcept
    {
      auto appearance = style.Appearance();
      return appearance == StyleAppearance::TextField && appearance == style.UsedAppearance();
    }
  }

  SearchFieldResultsButtonElement::SearchFieldResultsButtonElement(Document &document) noexcept
      : HTMLDivElement(document, dom::NodeFlag::HasCustomStyleResolveCallbacks)
  {
  }

  Ref<SearchFieldResultsButtonElement> SearchFieldResultsButtonElement::Create(Document &document) noexcept
  {
    return CreateRef<SearchFieldResultsButtonElement>(document);
  }

  Maybe<style::UnadjustedStyle>
    SearchFieldResultsButtonElement::ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                                                        const RenderStyle *shadowHostStyle) noexcept
  {
    _canAdjustStyleForAppearance = true;

    if (!shadowHostStyle)
    {
      return null;
    }

    if (SearchFieldStyleHasExplicitlySpecifiedTextFieldAppearance(*shadowHostStyle))
    {
      auto elementStyle = ResolveStyle(resolutionContext);
      elementStyle.Style->SetDisplay(DisplayType::None);
      return elementStyle;
    }

    // By default, input[type=search] can use either the searchfield or textfield appearance depending
    // on the platform and writing mode. Only adjust the style when that default is used.
    auto usedAppearance = shadowHostStyle->UsedAppearance();
    if (usedAppearance != StyleAppearance::SearchField && usedAppearance != StyleAppearance::TextField)
    {
      _canAdjustStyleForAppearance = false;
      return ResolveStyle(resolutionContext);
    }

    return null;
  }

  void SearchFieldResultsButtonElement::DefaultEventHandler(Event &event) noexcept
  {
    // A press focuses the field and selects its text.
    if (RefPtr<HTMLInputElement> input =
          ShareRefPtr(DynamicDowncast<HTMLInputElement>(dom::TreeQueries::ShadowHost(*this))))
    {
      auto *mouseEvent = DynamicDowncast<MouseEvent>(event);
      if (event.Type() == event_names::MouseDown && mouseEvent && mouseEvent->Button() == 0)
      {
        input->Focus();
        input->Select();
        event.SetDefaultHandled();
      }
    }

    if (!event.DefaultHandled())
    {
      HTMLDivElement::DefaultEventHandler(event);
    }
  }

#pragma endregion

#pragma region SearchFieldCancelButtonElement

  SearchFieldCancelButtonElement::SearchFieldCancelButtonElement(Document &document) noexcept
      : HTMLDivElement(document, dom::NodeFlag::HasCustomStyleResolveCallbacks)
  {
  }

  Ref<SearchFieldCancelButtonElement> SearchFieldCancelButtonElement::Create(Document &document) noexcept
  {
    auto element = CreateRef<SearchFieldCancelButtonElement>(document);
    element->SetUserAgentPart(css::user_agent_parts::WebkitSearchCancelButton());
    (void)element->SetAttribute(u8"aria-label", AXSearchFieldCancelButtonText());
    (void)element->SetAttribute(u8"role", u8"button");
    return element;
  }

  Maybe<style::UnadjustedStyle>
    SearchFieldCancelButtonElement::ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                                                       const RenderStyle *shadowHostStyle) noexcept
  {
    auto elementStyle = ResolveStyle(resolutionContext);
    auto &inputElement = Downcast<HTMLInputElement>(*dom::TreeQueries::ShadowHost(*this));
    elementStyle.Style->SetVisibility(elementStyle.Style->UsedVisibility() == Visibility::Hidden
                                          || inputElement.Value().empty()
                                        ? Visibility::Hidden
                                        : Visibility::Visible);

    if (shadowHostStyle && SearchFieldStyleHasExplicitlySpecifiedTextFieldAppearance(*shadowHostStyle))
    {
      elementStyle.Style->SetDisplay(DisplayType::None);
    }

    return elementStyle;
  }

  void SearchFieldCancelButtonElement::DefaultEventHandler(Event &event) noexcept
  {
    RefPtr<HTMLInputElement> input =
      ShareRefPtr(DynamicDowncast<HTMLInputElement>(dom::TreeQueries::ShadowHost(*this)));
    if (!input || !input->IsMutable())
    {
      if (!event.DefaultHandled())
      {
        HTMLDivElement::DefaultEventHandler(event);
      }
      return;
    }

    if (auto *mouseEvent = DynamicDowncast<MouseEvent>(event);
        event.Type() == event_names::MouseDown && mouseEvent && mouseEvent->Button() == 0)
    {
      input->Focus();
      input->Select();
      event.SetDefaultHandled();
    }

    if (pointer_events::IsAnyClick(event))
    {
      (void)input->SetValue({}, TextFieldEventBehavior::DispatchChangeEvent);
      event.SetDefaultHandled();
    }

    if (!event.DefaultHandled())
    {
      HTMLDivElement::DefaultEventHandler(event);
    }
  }

#pragma endregion
}
