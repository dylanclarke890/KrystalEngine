#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/HTMLDataListElement.hpp"
#include "Krystal.Booey/HTML/HTMLHRElement.hpp"
#include "Krystal.Booey/HTML/HTMLOptGroupElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"

namespace krys::boo::html
{
  namespace
  {
    KRYS_NODISCARD bool IsScriptElement(const dom::Node &node) noexcept
    {
      auto *element = DynamicDowncast<dom::Element>(node);
      return element && element->LocalName().View() == u8"script"
             && (element->NamespaceURI().View() == infra::Namespaces::HTML
                 || element->NamespaceURI().View() == infra::Namespaces::SVG);
    }
  }

  HTMLOptionElement::HTMLOptionElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Option)
  {
  }

  dom::ExceptionOr<Ref<HTMLOptionElement>>
    HTMLOptionElement::CreateForLegacyFactoryFunction(dom::Document &document, dom::DOMString &&text,
                                                      Maybe<dom::DOMString> value, bool defaultSelected,
                                                      bool selected) noexcept
  {
    auto element = CreateRef<HTMLOptionElement>(document);

    if (!text.empty())
    {
      auto appendResult = element->AppendChild(*CreateRef<dom::Text>(document, krys::move(text)));
      if (appendResult.HasException())
      {
        return appendResult.ReleaseException();
      }
    }

    if (value)
    {
      (void)element->SetAttribute(dom::DOMStringAtom(u8"value"), krys::move(*value));
    }
    if (defaultSelected)
    {
      (void)element->SetAttribute(dom::DOMStringAtom(u8"selected"), dom::DOMString());
    }
    element->Selected(selected);

    return element;
  }

  bool HTMLOptionElement::IsDisabledFormControl() const noexcept
  {
    if (OwnElementDisabled())
    {
      return true;
    }

    for (auto *ancestor = ParentElement(); ancestor; ancestor = ancestor->ParentElement())
    {
      if (auto *optGroup = DynamicDowncast<HTMLOptGroupElement>(*ancestor))
      {
        return optGroup->IsDisabledFormControl();
      }
      if (Is<HTMLDataListElement>(*ancestor) || Is<HTMLSelectElement>(*ancestor)
          || Is<HTMLOptionElement>(*ancestor) || Is<HTMLHRElement>(*ancestor))
      {
        return false;
      }
    }
    return false;
  }

#pragma region HTMLOptionElement

  bool HTMLOptionElement::Disabled() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLOptionElement::Disabled(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  HTMLFormElement *HTMLOptionElement::Form() const noexcept
  {
    if (auto *select = OwnerSelectElement())
    {
      return select->Form();
    }
    return nullptr;
  }

  dom::DOMString HTMLOptionElement::Label() const noexcept
  {
    if (auto label = GetAttribute(u8"label"))
    {
      return dom::DOMString(*label);
    }
    return CollectOptionInnerTextCollapsingWhitespace();
  }

  void HTMLOptionElement::Label(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"label", krys::move(value));
  }

  bool HTMLOptionElement::DefaultSelected() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"selected");
  }

  void HTMLOptionElement::DefaultSelected(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"selected", krys::move(value));
  }

  bool HTMLOptionElement::Selected(AllowStyleInvalidation allowStyleInvalidation) const noexcept
  {
    if (auto *select = OwnerSelectElement())
    {
      select->UpdateListItemSelectedStates(allowStyleInvalidation);
    }
    return _isSelected;
  }

  void HTMLOptionElement::Selected(bool value) noexcept
  {
    _dirtiness = true;
    if (_isSelected == value)
    {
      return;
    }

    SetSelectedState(value);

    if (RefPtr<HTMLSelectElement> select = ShareRefPtr(OwnerSelectElement()))
    {
      select->OptionSelectionStateChanged(*this, value);
    }
  }

  dom::DOMString HTMLOptionElement::Value() const noexcept
  {
    if (auto value = GetAttribute(u8"value"))
    {
      return dom::DOMString(*value);
    }
    return CollectOptionInnerTextCollapsingWhitespace();
  }

  void HTMLOptionElement::Value(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"value", krys::move(value));
  }

  dom::DOMString HTMLOptionElement::Text() const noexcept
  {
    return CollectOptionInnerTextCollapsingWhitespace();
  }

  void HTMLOptionElement::Text(dom::DOMString &&value) noexcept
  {
    CEReactionsScope ceReactions;
    Ref<HTMLOptionElement> protectedThis = ShareRef(*this);

    // Changing the text causes a recalc of a select's items, which will reset the selected index to the first
    // item if the select is single selection with a menu list. We attempt to preserve the selected item.
    RefPtr<HTMLSelectElement> select = ShareRefPtr(OwnerSelectElement());
    bool selectIsMenuList = select && select->UsesMenuList();
    int32 oldSelectedIndex = selectIsMenuList ? select->SelectedIndex() : -1;
    (void)TextContent(krys::move(value));

    if (selectIsMenuList && select->SelectedIndex() != oldSelectedIndex)
    {
      select->SelectedIndex(oldSelectedIndex);
    }
  }

  int32 HTMLOptionElement::Index() const noexcept
  {
    // It would be faster to cache the index, but harder to get it right in all cases.
    auto *selectElement = OwnerSelectElement();
    if (!selectElement)
    {
      return 0;
    }

    int32 optionIndex = 0;
    for (auto &item : selectElement->ListItems())
    {
      if (!Is<HTMLOptionElement>(*item))
      {
        continue;
      }
      if (item.get() == this)
      {
        return optionIndex;
      }
      ++optionIndex;
    }

    return 0;
  }

#pragma endregion

  HTMLSelectElement *HTMLOptionElement::OwnerSelectElement() const noexcept
  {
    return _ownerSelect.get();
  }

  dom::DOMString HTMLOptionElement::DisplayLabel() const noexcept
  {
    // Same as Label() but ignores the label content attribute in quirks mode for compatibility with other
    // browsers.
    if (NodeDocument().InQuirksMode())
    {
      return CollectOptionInnerTextCollapsingWhitespace();
    }
    return Label();
  }

  dom::DOMString HTMLOptionElement::TextIndentedToRespectGroupLabel() const noexcept
  {
    for (auto *ancestor = ParentElement(); ancestor; ancestor = ancestor->ParentElement())
    {
      if (Is<HTMLOptGroupElement>(*ancestor))
      {
        return u8"    " + Label();
      }
      if (Is<HTMLDataListElement>(*ancestor) || Is<HTMLSelectElement>(*ancestor)
          || Is<HTMLOptionElement>(*ancestor) || Is<HTMLHRElement>(*ancestor))
      {
        return Label();
      }
    }
    return Label();
  }

  void HTMLOptionElement::SetSelectedState(bool selected,
                                           AllowStyleInvalidation allowStyleInvalidation) noexcept
  {
    if (_isSelected == selected)
    {
      return;
    }

    Maybe<style::PseudoClassChangeInvalidation> checkedInvalidation;
    if (allowStyleInvalidation == AllowStyleInvalidation::Yes)
    {
      checkedInvalidation.emplace(*this, css::SelectorPseudoClass::Checked, selected);
    }

    _isSelected = selected;
  }

  bool HTMLOptionElement::AccessKeyAction(bool) noexcept
  {
    if (RefPtr<HTMLSelectElement> select = ShareRefPtr(OwnerSelectElement()))
    {
      select->AccessKeySetSelectedIndex(Index());
      return true;
    }
    return false;
  }

#pragma region Extension Hooks

  void HTMLOptionElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();
    if (_ownerSelect)
    {
      return;
    }

    if (RefPtr<HTMLSelectElement> select = ShareRefPtr(
          HTMLSelectElement::FindOwnerSelect(ParentNode(), HTMLSelectElement::ExcludeOptGroup::No)))
    {
      _ownerSelect = select;
      select->SetRecalcListItems();
      select->OptionInserted(*this);
    }
  }

  void HTMLOptionElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
    UpdateOwnerSelect();
  }

  void HTMLOptionElement::OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnMove(isSubtreeRoot, oldAncestor);
    UpdateOwnerSelect();
    OnInsert();
  }

  void HTMLOptionElement::UpdateOwnerSelect() noexcept
  {
    if (!_ownerSelect)
    {
      return;
    }

    if (auto *select =
          HTMLSelectElement::FindOwnerSelect(ParentNode(), HTMLSelectElement::ExcludeOptGroup::No))
    {
      if (select == _ownerSelect.get())
      {
        return;
      }
    }

    RefPtr<HTMLSelectElement> select = ShareRefPtr(_ownerSelect.get());
    _ownerSelect = nullptr;
    select->SetRecalcListItems();
  }

  void HTMLOptionElement::OnChildrenChanged() noexcept
  {
    // FIXME: Tell ancestor datalists (optionElementChildrenChanged) once datalists are ported.

    if (RefPtr<HTMLSelectElement> select = ShareRefPtr(OwnerSelectElement()))
    {
      select->OptionElementChildrenChanged();
    }

    HTMLElement::OnChildrenChanged();
  }

  void HTMLOptionElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                             Maybe<dom::DOMStringView> value,
                                             dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    if (localName == u8"disabled")
    {
      bool newDisabled = value.has_value();
      if (_disabled != newDisabled)
      {
        style::PseudoClassChangeInvalidation disabledInvalidation(
          *this, {{css::SelectorPseudoClass::Disabled, newDisabled},
                  {css::SelectorPseudoClass::Enabled, !newDisabled}});
        _disabled = newDisabled;
        if (auto *renderer = Renderer(); renderer && renderer->Style().HasUsedAppearance())
        {
          renderer->Repaint();
        }
      }
    }
    else if (localName == u8"selected")
    {
      style::PseudoClassChangeInvalidation defaultInvalidation(*this, css::SelectorPseudoClass::Default,
                                                               value.has_value());
      _isDefault = value.has_value();

      // Whenever an option element's selected attribute is added or removed, if its dirtiness is false, its
      // selectedness must follow it.
      if (oldValue.has_value() != value.has_value() && !_dirtiness && _isSelected != value.has_value())
      {
        SetSelectedState(value.has_value());
        if (RefPtr<HTMLSelectElement> select = ShareRefPtr(OwnerSelectElement()))
        {
          select->OptionSelectionStateChanged(*this, value.has_value());
        }
      }
    }
    else if (localName == u8"label")
    {
      if (RefPtr<HTMLSelectElement> select = ShareRefPtr(OwnerSelectElement()))
      {
        select->OptionElementChildrenChanged();
      }
    }
    // FIXME: Tell ancestor datalists when the value changes (optionElementChildrenChanged) once datalists are
    // ported.
  }

#pragma endregion

  dom::DOMString HTMLOptionElement::CollectOptionInnerText() const noexcept
  {
    dom::DOMString text;
    // Text nodes inside script elements are not part of the option text.
    for (auto *node = FirstChild(); node; node = IsScriptElement(*node)
                                                   ? dom::TreeQueries::NextSkippingChildren(*node, this)
                                                   : dom::TreeQueries::Next(*node, this))
    {
      if (auto *textNode = DynamicDowncast<dom::Text>(*node))
      {
        text += textNode->Data();
      }
    }
    return text;
  }

  dom::DOMString HTMLOptionElement::CollectOptionInnerTextCollapsingWhitespace() const noexcept
  {
    return infra::StringAlgorithms::StripAndCollapseASCIIWhitespace(CollectOptionInnerText());
  }
}
