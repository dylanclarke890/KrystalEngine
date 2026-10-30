#include "Krystal.Booey/HTML/Shadow/DateTimeFieldElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/ResolvedStyle.hpp"

namespace krys::boo::html
{
  DateTimeFieldElement::DateTimeFieldElement(dom::Document &document,
                                             DateTimeFieldElementFieldOwner &fieldOwner) noexcept
      : HTMLDivElement(document, dom::NodeFlags::HasCustomStyleResolveCallbacks), _fieldOwner(&fieldOwner)
  {
  }

  void DateTimeFieldElement::Initialize(DateTimeFieldElement &field, dom::DOMStringAtom part) noexcept
  {
    field.SetUserAgentPart(part);
    // Fields are focusable, and in sequential navigation.
    (void)field.SetAttribute(u8"tabindex", u8"0");
    // FIXME: The fields' accessibility attributes (aria-label, role, aria-valuenow, aria-valuetext), with
    // accessibility.
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
      auto textColor = shadowHostStyle->VisitedDependentColorWithColorFilter(css::PropertyId::Color);
      auto backgroundColor =
        shadowHostStyle->VisitedDependentColorWithColorFilter(css::PropertyId::BackgroundColor);
      style.SetColor(RenderTheme::Singleton().DatePlaceholderTextColor(textColor, backgroundColor));
    }

    return elementStyle;
  }

  void DateTimeFieldElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    if (event.Type() == EventNames::Blur)
    {
      HandleBlurEvent(event);
    }

    if (auto *keyboardEvent = DynamicDowncast<dom::KeyboardEvent>(event))
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

  void DateTimeFieldElement::DefaultKeyboardEventHandler(dom::KeyboardEvent &keyboardEvent) noexcept
  {
    if (IsFieldOwnerDisabled())
    {
      return;
    }

    if (keyboardEvent.Type() != EventNames::KeyDown)
    {
      return;
    }

    auto &key = keyboardEvent.KeyIdentifier();
    auto &code = keyboardEvent.Code();

    bool isHorizontal = IsFieldOwnerHorizontal();
    dom::DOMStringView nextKeyIdentifier = isHorizontal ? u8"Right" : u8"Down";
    dom::DOMStringView previousKeyIdentifier = isHorizontal ? u8"Left" : u8"Up";
    dom::DOMStringView stepUpKeyIdentifier = isHorizontal ? u8"Up" : u8"Right";
    dom::DOMStringView stepDownKeyIdentifier = isHorizontal ? u8"Down" : u8"Left";

    if (key == previousKeyIdentifier && _fieldOwner && _fieldOwner->FocusOnPreviousField(*this))
    {
      keyboardEvent.SetDefaultHandled();
      return;
    }

    if ((key == nextKeyIdentifier || code == u8"Comma" || code == u8"Minus" || code == u8"Period"
         || code == u8"Slash" || code == u8"Semicolon")
        && _fieldOwner && _fieldOwner->FocusOnNextField(*this))
    {
      keyboardEvent.SetDefaultHandled();
      return;
    }

    if (IsFieldOwnerReadOnly())
    {
      return;
    }

    if (key == stepUpKeyIdentifier)
    {
      StepUp();
      keyboardEvent.SetDefaultHandled();
      return;
    }

    if (key == stepDownKeyIdentifier)
    {
      StepDown();
      keyboardEvent.SetDefaultHandled();
      return;
    }

    // Clear value when pressing backspace or delete.
    if (key == u8"U+0008" || key == u8"U+007F")
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

  void DateTimeFieldElement::HandleBlurEvent(dom::Event &event) noexcept
  {
    if (_fieldOwner)
    {
      _fieldOwner->DidBlurFromField(event);
    }
  }

  Locale &DateTimeFieldElement::LocaleForOwner() const noexcept
  {
    return NodeDocument().GetCachedLocale(LocaleIdentifier());
  }

  dom::DOMString DateTimeFieldElement::LocaleIdentifier() const noexcept
  {
    return _fieldOwner ? _fieldOwner->LocaleIdentifier() : dom::DOMString();
  }

  dom::DOMString DateTimeFieldElement::VisibleValue() const noexcept
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
      (void)AppendChild(*CreateRef<dom::Text>(NodeDocument(), dom::DOMString()));
    }

    auto &textNode = Downcast<dom::Text>(*FirstChild());
    auto newVisibleValue = VisibleValue();
    if (textNode.WholeText() != newVisibleValue)
    {
      (void)textNode.Data(krys::move(newVisibleValue));
      // The placeholder's colour comes and goes with the value.
      InvalidateStyle();
    }

    if (eventBehavior == EventBehavior::DispatchInputAndChangeEvents && _fieldOwner)
    {
      _fieldOwner->FieldValueChanged();
    }
  }
}
