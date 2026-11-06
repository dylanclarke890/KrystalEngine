#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/HTML/Algorithms/FormControlAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDataListElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLHRElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptGroupElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Invalidation/PseudoClassChangeInvalidation.hpp"

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
    HTMLOptionElement::CreateForLegacyFactoryFunction(dom::Document &document, dom::DOMString text,
                                                      Maybe<dom::DOMString> value, bool defaultSelected,
                                                      bool selected) noexcept
  {
    auto element = dom::ElementFactory::CreateHTMLElement<HTMLOptionElement>(document, u8"option");

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

    // 6. If selected is true, then set option's selectedness to true; otherwise set its selectedness to false
    //    (even if defaultSelected is true).
    element->SetSelectedState(selected);

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
    return Reflect<bool>(*this, u8"disabled");
  }

  void HTMLOptionElement::Disabled(bool value) noexcept
  {
    Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  HTMLFormElement *HTMLOptionElement::Form() const noexcept
  {
    if (auto *select = OwnerSelectElement())
    {
      return select->Form();
    }
    return nullptr;
  }

  // https://html.spec.whatwg.org/#dom-option-label
  dom::DOMString HTMLOptionElement::Label() const noexcept
  {
    // 1. Let attribute be this's label attribute.
    // 2. If attribute is null, then return the result of getting an option element's label given this and
    //    false.
    // 3. Return attribute's value.
    if (auto label = GetAttribute(u8"label"))
    {
      return dom::DOMString(*label);
    }
    return OptionLabel();
  }

  void HTMLOptionElement::Label(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"label", krys::move(value));
  }

  bool HTMLOptionElement::DefaultSelected() const noexcept
  {
    return Reflect<bool>(*this, u8"selected");
  }

  void HTMLOptionElement::DefaultSelected(bool value) noexcept
  {
    Reflect<bool>(*this, u8"selected", krys::move(value));
  }

  bool HTMLOptionElement::Selected(AllowStyleInvalidation allowStyleInvalidation) const noexcept
  {
    if (auto *select = OwnerSelectElement())
    {
      select->UpdateListItemSelectedStates(allowStyleInvalidation);
    }
    return _isSelected;
  }

  // https://html.spec.whatwg.org/#dom-option-selected
  void HTMLOptionElement::Selected(bool value) noexcept
  {
    // 2. Set this's dirtiness to true.
    _dirtiness = true;
    if (_isSelected == value)
    {
      return;
    }

    // 1. Set this's selectedness to the given value.
    SetSelectedState(value);

    // 3. Ask for a reset given this.
    // https://html.spec.whatwg.org/#ask-for-a-reset
    // 1. Let select be option's cached nearest ancestor select element.
    // 2. If select is null, then return.
    // 3. Run the selectedness setting algorithm given select.
    // 4. Update a select's descendant selectedcontent elements given select.

    if (RefPtr<HTMLSelectElement> select = ShareRefPtr(OwnerSelectElement()))
    {
      select->OptionSelectionStateChanged(*this, value);
      FormControlAlgorithms::UpdateDescendantSelectedContents(*select);
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

  void HTMLOptionElement::Value(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"value", krys::move(value));
  }

  dom::DOMString HTMLOptionElement::Text() const noexcept
  {
    return CollectOptionInnerTextCollapsingWhitespace();
  }

  void HTMLOptionElement::Text(dom::DOMString value) noexcept
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
    // Same as OptionLabel() but ignores the label content attribute in quirks mode for compatibility with
    // other browsers.
    if (NodeDocument().InQuirksMode())
    {
      return CollectOptionInnerTextCollapsingWhitespace();
    }
    return OptionLabel();
  }

  dom::DOMString HTMLOptionElement::TextIndentedToRespectGroupLabel() const noexcept
  {
    for (auto *ancestor = ParentElement(); ancestor; ancestor = ancestor->ParentElement())
    {
      if (Is<HTMLOptGroupElement>(*ancestor))
      {
        return u8"    " + OptionLabel();
      }
      if (Is<HTMLDataListElement>(*ancestor) || Is<HTMLSelectElement>(*ancestor)
          || Is<HTMLOptionElement>(*ancestor) || Is<HTMLHRElement>(*ancestor))
      {
        return OptionLabel();
      }
    }
    return OptionLabel();
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

    if (auto *select = _ownerSelect.get())
    {
      select->InvalidateSelectedItems();
    }
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
      select->InvalidateLiveCollectionCaches();
      select->SetRecalcListItems();
      select->OptionInserted(*this);
    }
  }

  // https://html.spec.whatwg.org/#the-option-element:html-element-post-connection-steps
  void HTMLOptionElement::OnPostConnection() noexcept
  {
    HTMLElement::OnPostConnection();

    // Update descendant selectedcontent elements for an option given insertedOption.
    FormControlAlgorithms::UpdateDescendantSelectedContentsForOption(*this);
  }

  // https://html.spec.whatwg.org/#the-option-element:html-element-removing-steps
  void HTMLOptionElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);

    // 1. Let selects be the result of update an option's nearest ancestor select given removedNode.
    // 2. For each select of selects: queue a microtask to update a select's descendant selectedcontent
    // elements
    //    given select.
    if (RefPtr<HTMLSelectElement> oldSelect = UpdateOwnerSelect(); oldSelect && _isSelected)
    {
      FormControlAlgorithms::QueueUpdateDescendantSelectedContents(*oldSelect);
    }
  }

  // https://html.spec.whatwg.org/#the-option-element:html-element-moving-steps
  void HTMLOptionElement::OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnMove(isSubtreeRoot, oldAncestor);

    // 1. Let selects be the result of update an option's nearest ancestor select given movedNode.
    // 2. For each select of selects: queue a microtask to update a select's descendant selectedcontent
    // elements
    //    given select.
    RefPtr<HTMLSelectElement> oldSelect = UpdateOwnerSelect();
    if (oldSelect && _isSelected)
    {
      FormControlAlgorithms::QueueUpdateDescendantSelectedContents(*oldSelect);
    }
    OnInsert();
    if (RefPtr<HTMLSelectElement> newSelect = ShareRefPtr(OwnerSelectElement());
        newSelect && newSelect != oldSelect && Selected())
    {
      FormControlAlgorithms::QueueUpdateDescendantSelectedContents(*newSelect);
    }
  }

  // https://html.spec.whatwg.org/#update-an-option's-nearest-ancestor-select
  RefPtr<HTMLSelectElement> HTMLOptionElement::UpdateOwnerSelect() noexcept
  {
    // (The new select is set by OnInsert, which runs the selectedness setting algorithm by finding the list
    // of options again.)
    if (!_ownerSelect)
    {
      return nullptr;
    }

    // 4. If oldSelect is newSelect, then return « ».
    if (auto *select =
          HTMLSelectElement::FindOwnerSelect(ParentNode(), HTMLSelectElement::ExcludeOptGroup::No))
    {
      if (select == _ownerSelect.get())
      {
        return nullptr;
      }
    }

    // 6b. Let hasChanged be the result of running the selectedness setting algorithm given select.
    RefPtr<HTMLSelectElement> select = ShareRefPtr(_ownerSelect.get());
    _ownerSelect = nullptr;
    select->InvalidateLiveCollectionCaches();
    select->SetRecalcListItems();
    return select;
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

        // https://html.spec.whatwg.org/#concept-option-disabled
        // 1. Let select be the option element's cached nearest ancestor select element.
        // 2. If select is null, then return.
        // 3. Reset a select's selectedness given select and false.
        if (RefPtr<HTMLSelectElement> select = ShareRefPtr(OwnerSelectElement()))
        {
          select->ResetSelectedness(false);
        }
      }
    }
    else if (localName == u8"selected")
    {
      style::PseudoClassChangeInvalidation defaultInvalidation(*this, css::SelectorPseudoClass::Default,
                                                               value.has_value());
      _isDefault = value.has_value();

      // https://html.spec.whatwg.org/#concept-option-selectedness
      // 1. Let hasChanged be false.
      // 2. If option's dirtiness is false:
      // 2a. If the selected attribute was added and option's selectedness is false, then set option's
      //     selectedness to true and set hasChanged to true.
      // 2b. If the selected attribute was removed and option's selectedness is true, then set option's
      //     selectedness to false and set hasChanged to true.
      // 3. Let select be option's cached nearest ancestor select element.
      // 4. If select is null, then return.
      // 5. Reset a select's selectedness given select and hasChanged.
      // (Only when it changed: the selectedness setting algorithm changes nothing otherwise.)
      if (oldValue.has_value() != value.has_value() && !_dirtiness && _isSelected != value.has_value())
      {
        SetSelectedState(value.has_value());
        if (RefPtr<HTMLSelectElement> select = ShareRefPtr(OwnerSelectElement()))
        {
          select->OptionSelectionStateChanged(*this, value.has_value());
          FormControlAlgorithms::UpdateDescendantSelectedContents(*select);
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

  // https://html.spec.whatwg.org/#get-html-aware-text-content
  dom::DOMString HTMLOptionElement::CollectOptionInnerText() const noexcept
  {
    // 1. Let text be the empty string.
    dom::DOMString text;

    // 2. For each descendant descendant of element in tree order:
    // 2a. If descendant is a script or SVG script element, then continue skipping all descendants of
    //     descendant.
    // (2c, the img alt step, is only for includeAltText, which no caller passes.)
    // 2d. If descendant is an img element, then continue skipping all descendants of descendant.
    // (img elements have no children.)
    for (auto *node = FirstChild(); node; node = IsScriptElement(*node)
                                                   ? dom::TreeQueries::NextSkippingChildren(*node, this)
                                                   : dom::TreeQueries::Next(*node, this))
    {
      // 2b. If descendant is a Text node, then set text to the concatenation of text and descendant's data.
      if (auto *textNode = DynamicDowncast<dom::Text>(*node))
      {
        text += textNode->Data();
      }
    }
    return text;
  }

  dom::DOMString HTMLOptionElement::CollectOptionInnerTextCollapsingWhitespace() const noexcept
  {
    // 3. Return the result of strip and collapse ASCII whitespace given text.
    return infra::StringAlgorithms::StripAndCollapseASCIIWhitespace(CollectOptionInnerText());
  }

  // https://html.spec.whatwg.org/#concept-option-label
  dom::DOMString HTMLOptionElement::OptionLabel() const noexcept
  {
    // 1. If option has a label attribute whose value is not the empty string, then return that value.
    if (auto label = GetAttribute(u8"label"); label && !label->empty())
    {
      return dom::DOMString(*label);
    }

    // 2. Return the result of get HTML-aware text content given option and includeAltText.
    return CollectOptionInnerTextCollapsingWhitespace();
  }
}
