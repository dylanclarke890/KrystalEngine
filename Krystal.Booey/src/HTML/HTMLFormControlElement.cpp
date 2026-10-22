#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"

namespace krys::boo::html
{
  HTMLFormControlElement::HTMLFormControlElement(dom::Document &document, HTMLElementInterface interface,
                                                 HTMLElementFlags flags) noexcept
      : HTMLElement(document, interface, flags)
  {
  }

  HTMLFormControlElement::~HTMLFormControlElement() noexcept
  {
    ClearForm();
  }

  bool HTMLFormControlElement::FormNoValidate() const noexcept
  {
    return HasAttribute(u8"formnovalidate");
  }

  void HTMLFormControlElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();
    if (auto *parent = ParentNode())
    {
      ValidatedFormListedElement::InsertedIntoAncestor(IsConnected(), *parent);
    }
  }

  void HTMLFormControlElement::OnPostConnection() noexcept
  {
    HTMLElement::OnPostConnection();
    ValidatedFormListedElement::DidFinishInsertingNode();
  }

  void HTMLFormControlElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
    ValidatedFormListedElement::RemovedFromAncestor(oldAncestor.IsConnected(), oldAncestor);
  }

  void HTMLFormControlElement::OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnMove(isSubtreeRoot, oldAncestor);

    // A move is a removal and an insertion that keep the element connected.
    ValidatedFormListedElement::RemovedFromAncestor(false, oldAncestor);
    if (auto *parent = ParentNode())
    {
      ValidatedFormListedElement::InsertedIntoAncestor(IsConnected(), *parent);
    }
    if (IsConnected())
    {
      ValidatedFormListedElement::DidFinishInsertingNode();
    }
  }

  void HTMLFormControlElement::OnAdopt(dom::Document &oldDocument) noexcept
  {
    HTMLElement::OnAdopt(oldDocument);
    ValidatedFormListedElement::DidMoveToNewDocument();
  }

  void HTMLFormControlElement::OnAttributeChanged(dom::DOMStringAtom localName,
                                                  Maybe<dom::DOMStringView> oldValue,
                                                  Maybe<dom::DOMStringView> value,
                                                  dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    if (localName == u8"required")
    {
      bool newRequired = value.has_value();
      if (_isRequired != newRequired)
      {
        style::PseudoClassChangeInvalidation requiredInvalidation(
          *this, {
                   {css::SelectorPseudoClass::Required, newRequired},
                   {css::SelectorPseudoClass::Optional, !newRequired},
                 });
        _isRequired = newRequired;
        RequiredStateChanged();
      }
    }
    else
    {
      ValidatedFormListedElement::ParseAttribute(localName, value);
    }
  }

  void HTMLFormControlElement::DisabledStateChanged() noexcept
  {
    ValidatedFormListedElement::DisabledStateChanged();
    if (auto *renderer = Renderer(); renderer && renderer->Style().HasUsedAppearance())
    {
      renderer->Repaint();
    }
  }

  void HTMLFormControlElement::ReadOnlyStateChanged() noexcept
  {
    ValidatedFormListedElement::ReadOnlyStateChanged();

    // Some input pseudo-classes, like :in-range and :out-of-range, depend on the readonly state.
    // FIXME: Use PseudoClassChangeInvalidation instead for :has() support and more efficiency.
    InvalidateStyleForSubtree();
  }

  void HTMLFormControlElement::RequiredStateChanged() noexcept
  {
    UpdateValidity();
  }

  void HTMLFormControlElement::SetChangedSinceLastFormControlChangeEvent(bool changed) noexcept
  {
    _wasChangedSinceLastFormControlChangeEvent = changed;
  }

  void HTMLFormControlElement::DispatchChangeEvent() noexcept
  {
    auto event = dom::Event::Create(dom::DOMStringAtom(EventNames::Change), true, false);
    (void)dom::EventDispatcher::DispatchToTarget(*event, this);
  }

  void HTMLFormControlElement::DispatchCancelEvent() noexcept
  {
    auto event = dom::Event::Create(dom::DOMStringAtom(EventNames::Cancel), true, false);
    (void)dom::EventDispatcher::DispatchToTarget(*event, this);
  }

  void HTMLFormControlElement::DispatchFormControlChangeEvent() noexcept
  {
    DispatchChangeEvent();
    SetChangedSinceLastFormControlChangeEvent(false);
    SetInteractedWithSinceLastFormSubmitEvent(true);
  }

  void HTMLFormControlElement::DispatchFormControlInputEvent() noexcept
  {
    SetChangedSinceLastFormControlChangeEvent(true);
    auto event = dom::Event::Create(dom::DOMStringAtom(EventNames::Input), true, false, true);
    (void)dom::EventDispatcher::DispatchToTarget(*event, this);
  }

  dom::DOMString HTMLFormControlElement::ResultForDialogSubmit() const noexcept
  {
    return GetAttribute(u8"value").value_or(dom::DOMString());
  }
}
