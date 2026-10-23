#include "Krystal.Booey/HTML/HTMLOptGroupElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"
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
    return Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLOptGroupElement::Disabled(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  dom::DOMString HTMLOptGroupElement::Label() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"label").value_or(u8"");
  }

  void HTMLOptGroupElement::Label(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"label", krys::move(value));
  }

#pragma endregion

  dom::DOMString HTMLOptGroupElement::GroupLabelText() const noexcept
  {
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

    if (RefPtr<HTMLSelectElement> select = ShareRefPtr(HTMLSelectElement::FindOwnerSelect(ParentNode(), HTMLSelectElement::ExcludeOptGroup::Yes)))
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
    RecalcSelectOptions();

    if (namespaceURI != dom::DOMStringAtom::Null() || localName != u8"disabled")
    {
      return;
    }

    bool newDisabled = value.has_value();
    if (_isDisabled == newDisabled)
    {
      return;
    }

    style::PseudoClassChangeInvalidation disabledInvalidation(
      *this,
      {{css::SelectorPseudoClass::Disabled, newDisabled}, {css::SelectorPseudoClass::Enabled, !newDisabled}});
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

#pragma endregion

  void HTMLOptGroupElement::RecalcSelectOptions() noexcept
  {
    if (RefPtr<HTMLSelectElement> selectElement = ShareRefPtr(OwnerSelectElement()))
    {
      selectElement->SetRecalcListItems();
      selectElement->UpdateValidity();
    }
  }
}
