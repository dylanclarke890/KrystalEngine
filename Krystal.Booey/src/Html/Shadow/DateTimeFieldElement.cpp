/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 * Copyright (C) 2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Html/Shadow/DateTimeFieldElement.hpp"
#include "Krystal.Booey/Css/Properties/PropertyId.hpp"
#include "Krystal.Booey/Dom/Events/EventNames.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"
#include "Krystal.Booey/Dom/Nodes/Text.hpp"
#include "Krystal.Booey/Events/UI/KeyboardEvent.hpp"
#include "Krystal.Booey/Html/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/ResolvedStyle.hpp"

namespace krys::boo::html
{
  DateTimeFieldElement::DateTimeFieldElement(Document &document,
                                             DateTimeFieldElementFieldOwner &fieldOwner) noexcept
      : HTMLDivElement(document, dom::NodeFlag::HasCustomStyleResolveCallbacks), _fieldOwner(&fieldOwner)
  {
  }

  void DateTimeFieldElement::Initialize(DateTimeFieldElement &field, DOMStringAtom part,
                                        utf8_stringview label) noexcept
  {
    field.SetUserAgentPart(krys::move(part));
    // Fields are focusable, and in sequential navigation.
    (void)field.SetAttribute(u8"tabindex", u8"0");
    (void)field.SetAttribute(u8"role", u8"spinbutton");
    if (!label.empty())
    {
      (void)field.SetAttribute(u8"aria-label", DOMString(label));
    }
    field.UpdateAriaValueAttributes();
  }

  void DateTimeFieldElement::UpdateAriaValueAttributes() noexcept
  {
    if (!HasValue())
    {
      RemoveAttribute(u8"aria-valuenow");
      RemoveAttribute(u8"aria-valuetext");
      return;
    }
    (void)SetAttribute(u8"aria-valuenow", detail::IntegerToString(ValueAsInteger()));
    (void)SetAttribute(u8"aria-valuetext", VisibleValue());
  }

  Maybe<style::UnadjustedStyle>
    DateTimeFieldElement::ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                                             const RenderStyle *shadowHostStyle) noexcept
  {
    auto elementStyle = ResolveStyle(resolutionContext);

    auto &style = *elementStyle.Style;
    AdjustMinInlineSize(style);

    if (!HasValue() && shadowHostStyle)
    {
      auto textColor = shadowHostStyle->ColorResolvingCurrentColor(css::PropertyId::Color);
      auto backgroundColor = shadowHostStyle->ColorResolvingCurrentColor(css::PropertyId::BackgroundColor);
      style.SetColor(RenderTheme::Singleton().DatePlaceholderTextColor(textColor, backgroundColor));
    }

    return elementStyle;
  }

  void DateTimeFieldElement::DefaultEventHandler(Event &event) noexcept
  {
    if (event.Type() == event_names::Blur)
    {
      HandleBlurEvent(event);
    }

    if (auto *keyboardEvent = DynamicDowncast<KeyboardEvent>(event))
    {
      if (!IsFieldOwnerDisabled() && !IsFieldOwnerReadOnly())
      {
        HandleKeyboardEvent(*keyboardEvent);
        if (keyboardEvent->DefaultHandled())
        {
          return;
        }
      }

      DefaultKeyboardEventHandler(*keyboardEvent);
      if (keyboardEvent->DefaultHandled())
      {
        return;
      }
    }

    HTMLDivElement::DefaultEventHandler(event);
  }

  void DateTimeFieldElement::DefaultKeyboardEventHandler(KeyboardEvent &keyboardEvent) noexcept
  {
    if (IsFieldOwnerDisabled())
    {
      return;
    }

    if (keyboardEvent.Type() != event_names::KeyDown)
    {
      return;
    }

    auto &key = keyboardEvent.Key();
    auto &code = keyboardEvent.Code();

    bool isHorizontal = IsFieldOwnerHorizontal();
    DOMStringView nextKey = isHorizontal ? u8"ArrowRight" : u8"ArrowDown";
    DOMStringView previousKey = isHorizontal ? u8"ArrowLeft" : u8"ArrowUp";
    DOMStringView stepUpKey = isHorizontal ? u8"ArrowUp" : u8"ArrowRight";
    DOMStringView stepDownKey = isHorizontal ? u8"ArrowDown" : u8"ArrowLeft";

    if (key == previousKey && _fieldOwner && _fieldOwner->FocusOnPreviousField(*this))
    {
      keyboardEvent.SetDefaultHandled();
      return;
    }

    if ((key == nextKey || code == u8"Comma" || code == u8"Minus" || code == u8"Period" || code == u8"Slash"
         || code == u8"Semicolon")
        && _fieldOwner && _fieldOwner->FocusOnNextField(*this))
    {
      keyboardEvent.SetDefaultHandled();
      return;
    }

    if (IsFieldOwnerReadOnly())
    {
      return;
    }

    if (key == stepUpKey)
    {
      StepUp();
      keyboardEvent.SetDefaultHandled();
      return;
    }

    if (key == stepDownKey)
    {
      StepDown();
      keyboardEvent.SetDefaultHandled();
      return;
    }

    // Clear value when pressing backspace or delete.
    if (key == u8"Backspace" || key == u8"Delete")
    {
      SetEmptyValue(EventBehavior::DispatchInputAndChangeEvents);
      keyboardEvent.SetDefaultHandled();
      return;
    }
  }

  bool DateTimeFieldElement::IsFieldOwnerDisabled() const noexcept
  {
    return _fieldOwner && _fieldOwner->IsFieldOwnerDisabled();
  }

  bool DateTimeFieldElement::IsFieldOwnerReadOnly() const noexcept
  {
    return _fieldOwner && _fieldOwner->IsFieldOwnerReadOnly();
  }

  bool DateTimeFieldElement::IsFieldOwnerHorizontal() const noexcept
  {
    if (_fieldOwner)
    {
      return _fieldOwner->IsFieldOwnerHorizontal();
    }
    return true;
  }

  void DateTimeFieldElement::HandleBlurEvent(Event &event) noexcept
  {
    if (_fieldOwner)
    {
      _fieldOwner->DidBlurFromField(event);
    }
  }

  i18n::Locale &DateTimeFieldElement::LocaleForOwner() const noexcept
  {
    return NodeDocument().GetCachedLocale(LocaleIdentifier());
  }

  DOMString DateTimeFieldElement::LocaleIdentifier() const noexcept
  {
    return _fieldOwner ? _fieldOwner->LocaleIdentifier() : DOMString();
  }

  DOMString DateTimeFieldElement::VisibleValue() const noexcept
  {
    if (HasValue())
    {
      return Value();
    }
    return PlaceholderValue();
  }

  void DateTimeFieldElement::UpdateVisibleValue(EventBehavior eventBehavior) noexcept
  {
    if (!FirstChild())
    {
      (void)AppendChild(*CreateRef<Text>(NodeDocument(), DOMString()));
    }

    auto &textNode = Downcast<Text>(*FirstChild());
    auto newVisibleValue = VisibleValue();
    if (textNode.WholeText() != newVisibleValue)
    {
      (void)textNode.Data(krys::move(newVisibleValue));
      // The placeholder's colour comes and goes with the value.
      InvalidateStyle();
    }

    UpdateAriaValueAttributes();

    if (eventBehavior == EventBehavior::DispatchInputAndChangeEvents && _fieldOwner)
    {
      _fieldOwner->FieldValueChanged();
    }
  }
}
