/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/HTML/Elements/HTMLOptGroupElement.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDivElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLLegendElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Style/Invalidation/PseudoClassChangeInvalidation.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::html
{
  HTMLOptGroupElement::HTMLOptGroupElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::OptGroup)
  {
  }

#pragma region HTMLOptGroupElement

  bool HTMLOptGroupElement::Disabled() const noexcept
  {
    return Reflect<bool>(*this, u8"disabled");
  }

  void HTMLOptGroupElement::Disabled(bool value) noexcept
  {
    Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  dom::DOMString HTMLOptGroupElement::Label() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"label").value_or(u8"");
  }

  void HTMLOptGroupElement::Label(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"label", krys::move(value));
  }

#pragma endregion

  Ref<HTMLOptGroupElement> HTMLOptGroupElement::Create(dom::Document &document) noexcept
  {
    auto optgroup = CreateRef<HTMLOptGroupElement>(document);
    optgroup->EnsureUserAgentShadowRoot();
    return optgroup;
  }

  // https://html.spec.whatwg.org/#the-select-element-2
  void HTMLOptGroupElement::DidAddUserAgentShadowRoot(dom::ShadowRoot &root) noexcept
  {
    auto &document = NodeDocument();
    auto legendSlot = dom::ElementFactory::CreateHTMLElement<HTMLSlotElement>(document, u8"slot");
    _legendSlot = CreateWeakPtr(legendSlot.get());
    auto labelElement = dom::ElementFactory::CreateHTMLElement<HTMLDivElement>(document, u8"div");
    labelElement->SetUserAgentPart(css::UserAgentParts::InternalOptgroupLabel());
    _labelElement = CreateWeakPtr<dom::Element>(labelElement.get());
    (void)legendSlot->AppendChild(*labelElement);
    (void)root.AppendChild(*legendSlot);
    auto contentsSlot = dom::ElementFactory::CreateHTMLElement<HTMLSlotElement>(document, u8"slot");
    _contentsSlot = CreateWeakPtr(contentsSlot.get());
    (void)root.AppendChild(*contentsSlot);
  }

  HTMLSlotElement *HTMLOptGroupElement::SlotForHostChild(const dom::Node &child) const noexcept
  {
    krys_debug_assert(child.ParentNode() == this);
    if (Is<HTMLLegendElement>(child) && dom::TreeQueries::FirstElementChild(*this) == &child)
    {
      return _legendSlot.get();
    }
    return _contentsSlot.get();
  }

  void HTMLOptGroupElement::OnChildrenChanged() noexcept
  {
    // Only for what the slots can't tell from the insertion or removal of the other children.
    auto *firstChild = dom::TreeQueries::FirstElementChild(*this);
    auto *legend = firstChild && Is<HTMLLegendElement>(*firstChild) ? firstChild : nullptr;
    if (legend != _slottedLegend.get())
    {
      _slottedLegend = CreateWeakPtr(legend);
      if (auto *root = UserAgentShadowRoot())
      {
        dom::SlotAlgorithms::AssignSlottablesForTree(*root);
      }
    }
    HTMLElement::OnChildrenChanged();
  }

  bool HTMLOptGroupElement::ChildShouldCreateRenderer(const dom::Node &child) const noexcept
  {
    if (&child != _labelElement.get())
    {
      return HTMLElement::ChildShouldCreateRenderer(child);
    }
    // The label shows only with base appearance; a list box draws it itself.
    auto *select = HTMLSelectElement::FindOwnerSelect(ParentNode(), HTMLSelectElement::ExcludeOptGroup::No);
    return select && select->ContentsUseBaseAppearance();
  }

  dom::DOMString HTMLOptGroupElement::GroupLabelText() const noexcept
  {
    // FIXME: A legend first child's text (get an optgroup element's label), once there are customizable
    // selects.
    // Leading and trailing white space is ignored in options and optgroups, and the rest is collapsed.
    return infra::StringAlgorithms::StripAndCollapseASCIIWhitespace(
      GetAttribute(u8"label").value_or(dom::DOMString()));
  }

  bool HTMLOptGroupElement::AccessKeyAction(bool) noexcept
  {
    // Send to the parent to bring focus to the list box.
    RefPtr<HTMLSelectElement> select = ShareRefPtr(OwnerSelectElement());
    if (select && !select->Focused())
    {
      return select->AccessKeyAction(false);
    }
    return false;
  }

#pragma region Extension Hooks

  void HTMLOptGroupElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();
    if (_ownerSelect)
    {
      return;
    }

    if (RefPtr<HTMLSelectElement> select = ShareRefPtr(
          HTMLSelectElement::FindOwnerSelect(ParentNode(), HTMLSelectElement::ExcludeOptGroup::Yes)))
    {
      _ownerSelect = select;
      select->SetRecalcListItems();
      select->OptGroupInserted(*this);
    }
  }

  void HTMLOptGroupElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
    UpdateOwnerSelect();
  }

  void HTMLOptGroupElement::OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnMove(isSubtreeRoot, oldAncestor);
    UpdateOwnerSelect();
    OnInsert();
  }

  void HTMLOptGroupElement::UpdateOwnerSelect() noexcept
  {
    if (!_ownerSelect)
    {
      return;
    }

    if (auto *select =
          HTMLSelectElement::FindOwnerSelect(ParentNode(), HTMLSelectElement::ExcludeOptGroup::Yes))
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

  void HTMLOptGroupElement::OnAttributeChanged(dom::DOMStringAtom localName,
                                               Maybe<dom::DOMStringView> oldValue,
                                               Maybe<dom::DOMStringView> value,
                                               dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);

    if (namespaceURI == dom::DOMStringAtom::Null() && localName == u8"label" && _labelElement)
    {
      (void)_labelElement->TextContent(dom::DOMString(value.value_or(dom::DOMStringView())));
    }

    if (bool newDisabled = value.has_value();
        namespaceURI == dom::DOMStringAtom::Null() && localName == u8"disabled" && _isDisabled != newDisabled)
    {
      style::PseudoClassChangeInvalidation disabledInvalidation(
        *this, {{css::SelectorPseudoClass::Disabled, newDisabled},
                {css::SelectorPseudoClass::Enabled, !newDisabled}});
      List<UniquePtr<style::PseudoClassChangeInvalidation>> optionInvalidation;
      for (auto &descendant : dom::DescendantRange(*this))
      {
        if (auto *option = DynamicDowncast<HTMLOptionElement>(descendant))
        {
          optionInvalidation.push_back(CreateUnique<style::PseudoClassChangeInvalidation>(
            *option, std::initializer_list<Pair<css::SelectorPseudoClass, bool>> {
                       {css::SelectorPseudoClass::Disabled, newDisabled},
                       {css::SelectorPseudoClass::Enabled, !newDisabled}}));
        }
      }
      _isDisabled = newDisabled;
    }

    // https://html.spec.whatwg.org/#attr-optgroup-disabled
    // 1. Let select be the optgroup element's nearest ancestor select.
    // 2. If select is null, then return.
    // 3. Reset a select's selectedness given select and false.
    // (For any attribute, as the label is shown too.)
    RecalcSelectOptions();
  }

#pragma endregion

  void HTMLOptGroupElement::RecalcSelectOptions() noexcept
  {
    if (RefPtr<HTMLSelectElement> selectElement = ShareRefPtr(OwnerSelectElement()))
    {
      selectElement->ResetSelectedness(false);
    }
  }
}
