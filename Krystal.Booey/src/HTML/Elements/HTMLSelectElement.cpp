#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Events/MouseEvent.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/Internals/PopoverData.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Algorithms/FormControlAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Collections/HTMLOptionsCollection.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDataListElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDivElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLHRElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptGroupElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/Forms/Autofill.hpp"
#include "Krystal.Booey/HTML/Forms/DOMFormData.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderListBox.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderMenuList.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include <climits>
#include <unicode/uchar.h>

namespace krys::boo::html
{
  namespace
  {
    /// @brief A select's selected options.
    class SelectedOptionsCollection final : public dom::LiveHTMLCollection
    {
    private:
      HTMLSelectElement &_select;

    public:
      explicit SelectedOptionsCollection(HTMLSelectElement &select) noexcept
          : LiveHTMLCollection(select,
                               [&select](const dom::Element &element)
                               {
                                 auto *option = DynamicDowncast<HTMLOptionElement>(element);
                                 return option && option->OwnerSelectElement() == &select
                                        && option->Selected();
                               }),
            _select(select)
      {
      }

    protected:
      void WillValidateIndexCache() const noexcept override
      {
        // Reading whether an option is selected can update what's selected, invalidating the cache.
        _select.UpdateListItemSelectedStates();
        LiveHTMLCollection::WillValidateIndexCache();
      }
    };

    // Upper limit agreed upon with representatives of Opera and Mozilla.
    constexpr uint32 MaxSelectItems = 100'000u;

    KRYS_NODISCARD uint32 LimitToOnlyHTMLNonNegative(Maybe<dom::DOMStringView> value) noexcept
    {
      if (!value)
      {
        return 0u;
      }
      auto result = Numbers::ParseNonNegativeInteger(*value);
      if (!result.Success() || result.Value > static_cast<int64>(INT32_MAX))
      {
        return 0u;
      }
      return static_cast<uint32>(result.Value);
    }
  }

  HTMLSelectElement::HTMLSelectElement(dom::Document &document) noexcept
      : HTMLFormControlElement(document, HTMLElementInterface::Select), _typeAhead(this)
  {
  }

  namespace
  {
    /// @brief The select popover: a press in it isn't a press on the select, which would close it.
    class SelectPopoverElement final : public HTMLDivElement
    {
      KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SelectPopoverElement);

    public:
      using HTMLDivElement::HTMLDivElement;

      void DefaultEventHandler(dom::Event &event) noexcept final
      {
        if (event.Type() == EventNames::MouseDown)
        {
          event.SetDefaultHandled();
          return;
        }
        HTMLDivElement::DefaultEventHandler(event);
      }
    };
  }

  Ref<HTMLSelectElement> HTMLSelectElement::Create(dom::Document &document) noexcept
  {
    auto select = CreateRef<HTMLSelectElement>(document);
    select->EnsureUserAgentShadowRoot();
    return select;
  }

  // https://html.spec.whatwg.org/#the-select-element-2
  void HTMLSelectElement::DidAddUserAgentShadowRoot(dom::ShadowRoot &root) noexcept
  {
    auto &document = NodeDocument();

    auto selectButtonSlot = dom::ElementFactory::CreateHTMLElement<HTMLSlotElement>(document, u8"slot");
    _selectButtonSlot = CreateWeakPtr(selectButtonSlot.get());
    auto fallbackButtonText = dom::ElementFactory::CreateHTMLElement<HTMLDivElement>(document, u8"div");
    _selectFallbackButtonText = CreateWeakPtr<dom::Element>(fallbackButtonText.get());
    (void)selectButtonSlot->AppendChild(*fallbackButtonText);
    (void)root.AppendChild(*selectButtonSlot);

    auto selectPopover = CreateRef<SelectPopoverElement>(document);
    selectPopover->SetUserAgentPart(css::UserAgentParts::InternalSelectPicker());
    // As an auto popover but without the attribute, whose styles would reach the options of a native select.
    selectPopover->EnsurePopoverData().SetPopoverState(dom::PopoverState::Auto);
    _selectPopover = CreateWeakPtr<HTMLElement>(selectPopover.get());
    auto selectPopoverSlot = dom::ElementFactory::CreateHTMLElement<HTMLSlotElement>(document, u8"slot");
    _selectPopoverSlot = CreateWeakPtr(selectPopoverSlot.get());
    (void)selectPopover->AppendChild(*selectPopoverSlot);
    (void)root.AppendChild(*selectPopover);

    auto selectListBoxSlot = dom::ElementFactory::CreateHTMLElement<HTMLSlotElement>(document, u8"slot");
    _selectListBoxSlot = CreateWeakPtr(selectListBoxSlot.get());
    (void)root.AppendChild(*selectListBoxSlot);
  }

  HTMLSlotElement *HTMLSelectElement::SlotForHostChild(const dom::Node &child) const noexcept
  {
    krys_debug_assert(child.ParentNode() == this);
    if (!UsesMenuList())
    {
      return _selectListBoxSlot.get();
    }
    if (Is<HTMLButtonElement>(child) && dom::TreeQueries::FirstElementChild(*this) == &child)
    {
      return _selectButtonSlot.get();
    }
    return _selectPopoverSlot.get();
  }

  void HTMLSelectElement::ReassignSlots() noexcept
  {
    // Only for what the slots' names can't say. Inserting and removing other children assigns them already,
    // and reassigning during a removal would walk the composed tree through a slot that's still to hear of
    // it.
    auto *firstChild = dom::TreeQueries::FirstElementChild(*this);
    auto *selectButton =
      UsesMenuList() && firstChild && Is<HTMLButtonElement>(*firstChild) ? firstChild : nullptr;
    if (selectButton == _selectButton.get() && UsesMenuList() == _slotsAreForMenuList)
    {
      return;
    }
    _selectButton = CreateWeakPtr(selectButton);
    _slotsAreForMenuList = UsesMenuList();
    if (auto *root = UserAgentShadowRoot())
    {
      dom::SlotAlgorithms::AssignSlottablesForTree(*root);
    }
  }

  bool HTMLSelectElement::SetContentsUseBaseAppearance(bool value) noexcept
  {
    if (_contentsUseBaseAppearance == value)
    {
      return false;
    }
    _contentsUseBaseAppearance = value;
    if (!value && IsPickerOpen())
    {
      (void)_selectPopover->HidePopoverInternal(dom::FocusPreviousElement::No, dom::FireEvents::No, false,
                                                nullptr);
    }
    return true;
  }

  bool HTMLSelectElement::IsPickerOpen() const noexcept
  {
    auto *selectPopover = _selectPopover.get();
    return selectPopover && selectPopover->IsPopoverShowing();
  }

  void HTMLSelectElement::CloneSelectedOptionIntoSelectButton() noexcept
  {
    // 1. Let option be the first element of select's option list whose selectedness is set to true, if such
    // an
    //    element exists; otherwise null.
    auto *option = FirstSelectedOption();
    // 2. Let text be the empty string.
    // 3. If option is not null, then set text to option's label.
    dom::DOMString text = option ? dom::DOMString(option->Label()) : dom::DOMString();
    // 4. Set select's select fallback button text to text.
    if (auto *fallbackButtonText = _selectFallbackButtonText.get();
        fallbackButtonText && fallbackButtonText->TextContent().value_or(dom::DOMString()) != text)
    {
      (void)fallbackButtonText->TextContent(krys::move(text));
    }
  }

  // https://html.spec.whatwg.org/#select-open-picker
  void HTMLSelectElement::OpenPicker(bool byKeyboard) noexcept
  {
    auto *selectPopover = _selectPopover.get();
    if (!selectPopover || IsPickerOpen())
    {
      return;
    }
    // 3. Run the show popover algorithm given select's select popover, false, and select.
    (void)selectPopover->ShowPopoverInternal(this, false);

    // The focus goes to the selected option, for the keys to move on from.
    if (!IsPickerOpen())
    {
      return;
    }
    (void)NodeDocument().UpdateLayout();
    auto *option = FirstSelectedOption();
    if (!option || option->IsDisabledFormControl())
    {
      for (auto &item : ListItems())
      {
        option = DynamicDowncast<HTMLOptionElement>(item.get());
        if (option && !option->IsDisabledFormControl())
        {
          break;
        }
        option = nullptr;
      }
    }
    if (option)
    {
      // As for any focus by the mouse, it isn't shown after a press.
      dom::FocusOptions focusOptions;
      focusOptions.FocusVisible = byKeyboard;
      option->Focus(focusOptions);
    }
  }

  void HTMLSelectElement::ClosePicker() noexcept
  {
    if (!IsPickerOpen())
    {
      return;
    }
    (void)_selectPopover->HidePopoverInternal(dom::FocusPreviousElement::No, dom::FireEvents::Yes, false,
                                              nullptr);
    // The options are this element's children, so the popover doesn't see that the focus was inside it.
    auto *focused = NodeDocument().FocusedElement();
    if (focused && focused != this && Contains(focused))
    {
      Focus();
    }
  }

  // https://html.spec.whatwg.org/#concept-select-pick
  void HTMLSelectElement::PickOption(HTMLOptionElement &option) noexcept
  {
    // 1. If select has the multiple attribute or select is disabled, then return.
    if (_multiple || IsDisabledFormControl())
    {
      return;
    }
    // (Steps 2 and 3 are the callers'. Disabled options can't be picked.)
    if (!option.IsDisabledFormControl())
    {
      // 4. Set option's selectedness to true.
      // 5. Set option's dirtiness to true.
      // 6. Send select update notifications given select.
      OptionSelectedByUser(option.Index(), true);
    }
    // 7. If select is being rendered as a drop-down box with base appearance, then run the hide popover
    //    algorithm given select's select popover.
    ClosePicker();
  }

  // https://html.spec.whatwg.org/#focus-another-option
  void HTMLSelectElement::FocusOptionForKey(dom::DOMStringView keyIdentifier) noexcept
  {
    List<HTMLOptionElement *> options;
    for (auto &item : ListItems())
    {
      if (auto *option = DynamicDowncast<HTMLOptionElement>(item.get());
          option && !option->IsDisabledFormControl() && option->Renderer())
      {
        options.push_back(option);
      }
    }
    if (options.empty())
    {
      return;
    }

    constexpr int32 pageStep = 10;
    int32 lastIndex = static_cast<int32>(options.size()) - 1;
    int32 current = -1;
    auto *focused = NodeDocument().FocusedElement();
    for (int32 index = 0; index <= lastIndex; ++index)
    {
      if (options[static_cast<size_t>(index)] == focused)
      {
        current = index;
        break;
      }
    }

    int32 next = current;
    if (keyIdentifier == u8"Down")
    {
      next = current + 1;
    }
    else if (keyIdentifier == u8"Up")
    {
      next = current < 0 ? lastIndex : current - 1;
    }
    else if (keyIdentifier == u8"Home")
    {
      next = 0;
    }
    else if (keyIdentifier == u8"End")
    {
      next = lastIndex;
    }
    else if (keyIdentifier == u8"PageDown")
    {
      next = current + pageStep;
    }
    else
    {
      next = current - pageStep;
    }
    options[static_cast<size_t>(std::clamp(next, 0, lastIndex))]->Focus();
  }

  void HTMLSelectElement::BaseAppearanceDefaultEventHandler(dom::Event &event) noexcept
  {
    if (event.Type() == EventNames::MouseDown)
    {
      auto *mouseEvent = DynamicDowncast<dom::MouseEvent>(event);
      if (!mouseEvent || mouseEvent->Button() != 0)
      {
        return;
      }
      if (IsPickerOpen())
      {
        ClosePicker();
      }
      else
      {
        OpenPicker(false);
      }
      event.SetDefaultHandled();
      return;
    }

    if (event.Type() != EventNames::KeyDown)
    {
      return;
    }
    auto *keyboardEvent = DynamicDowncast<dom::KeyboardEvent>(event);
    if (!keyboardEvent)
    {
      return;
    }
    auto keyIdentifier = keyboardEvent->KeyIdentifier();
    bool isActivationKey = keyIdentifier == u8"Enter" || keyboardEvent->Key() == u8" ";
    bool isOptionKey = keyIdentifier == u8"Down" || keyIdentifier == u8"Up" || keyIdentifier == u8"Home"
                       || keyIdentifier == u8"End" || keyIdentifier == u8"PageDown"
                       || keyIdentifier == u8"PageUp";

    if (!IsPickerOpen())
    {
      if (isActivationKey || keyIdentifier == u8"Down" || keyIdentifier == u8"Up")
      {
        OpenPicker(true);
        event.SetDefaultHandled();
      }
      return;
    }

    if (isOptionKey)
    {
      FocusOptionForKey(keyIdentifier);
      event.SetDefaultHandled();
    }
    else if (isActivationKey)
    {
      if (auto *option = DynamicDowncast<HTMLOptionElement>(NodeDocument().FocusedElement()))
      {
        PickOption(*option);
      }
      event.SetDefaultHandled();
    }
    else if (keyIdentifier == u8"U+0009")
    {
      // The focus moves on, with the picker closed behind it.
      ClosePicker();
    }
  }

  HTMLSelectElement *HTMLSelectElement::FindOwnerSelect(dom::ContainerNode *startNode,
                                                        ExcludeOptGroup excludeOptGroup) noexcept
  {
    if (!startNode)
    {
      return nullptr;
    }
    if (auto *select = DynamicDowncast<HTMLSelectElement>(*startNode))
    {
      return select;
    }
    if (Is<HTMLOptGroupElement>(*startNode))
    {
      if (excludeOptGroup == ExcludeOptGroup::Yes)
      {
        return nullptr;
      }
      return FindOwnerSelect(startNode->ParentNode(), ExcludeOptGroup::Yes);
    }
    if (Is<HTMLDataListElement>(*startNode) || Is<HTMLHRElement>(*startNode)
        || Is<HTMLOptionElement>(*startNode))
    {
      return nullptr;
    }
    return FindOwnerSelect(startNode->ParentNode(), excludeOptGroup);
  }

  void HTMLSelectElement::DidRecalcStyle(EnumSet<style::Change> styleChange) noexcept
  {
    // Even though the options didn't necessarily change, we will call SetOptionsChangedOnRenderer for its
    // side effect of recomputing the width of the element. We need to do that if the style change included a
    // change in zoom level.
    SetOptionsChangedOnRenderer();
    HTMLFormControlElement::DidRecalcStyle(styleChange);
  }

  dom::DOMString HTMLSelectElement::FormControlType() const noexcept
  {
    return _multiple ? u8"select-multiple" : u8"select-one";
  }

  void HTMLSelectElement::OptionSelectedByUser(int32 optionIndex, bool fireOnChangeNow,
                                               bool allowMultipleSelection) noexcept
  {
    // User interaction such as mousedown events can cause list box select elements to send change events.
    // This produces that same behavior for changes triggered by other code running on behalf of the user.
    if (!UsesMenuList())
    {
      UpdateSelectedState(OptionToListIndex(optionIndex), allowMultipleSelection, false);
      UpdateValidity();
      if (auto *renderer = Renderer())
      {
        renderer->UpdateFromElement();
      }
      if (fireOnChangeNow)
      {
        ListBoxOnChange();
      }
      return;
    }

    // Bail out if this index is already the selected one, to avoid running unnecessary script when there is
    // no actual change. SelectOption doesn't behave this way, possibly because other callers need a change
    // event even in cases where the selected option is not changed.
    if (optionIndex == SelectedIndex())
    {
      return;
    }

    SelectOptionFlags flags {SelectOptionFlag::DeselectOtherOptions, SelectOptionFlag::UserDriven};
    if (fireOnChangeNow)
    {
      flags.Add(SelectOptionFlag::DispatchChangeEvent);
    }
    SelectOption(optionIndex, flags);
  }

  // https://html.spec.whatwg.org/#placeholder-label-option
  bool HTMLSelectElement::HasPlaceholderLabelOption() const noexcept
  {
    // The select element has no placeholder label option if it has an attribute "multiple" specified or a
    // display size of non-1.
    //
    // The condition "Size() > 1" is used rather than "Size() != 1" because Size() is 0 when the size
    // attribute is absent or invalid, in which case the display size is the default: 1 for non-multiple
    // select elements and 4 for multiple ones.
    if (Multiple() || Size() > 1)
    {
      return false;
    }

    // The first option element in the list of options, if its value is the empty string and it has no
    // ancestor optgroup in between itself and the select element.
    auto *option = Item(0);
    if (!option || !option->Value().empty())
    {
      return false;
    }
    for (auto *ancestor = option->ParentNode(); ancestor && ancestor != this;
         ancestor = ancestor->ParentNode())
    {
      if (Is<HTMLOptGroupElement>(*ancestor))
      {
        return false;
      }
    }
    return true;
  }

  bool HTMLSelectElement::ValueMissing() const noexcept
  {
    if (!WillValidate())
    {
      return false;
    }

    if (!IsRequired())
    {
      return false;
    }

    int32 firstSelectionIndex = SelectedIndex();

    // If a non-placeholder label option is selected (firstSelectionIndex > 0), it's not value-missing.
    return firstSelectionIndex < 0 || (!firstSelectionIndex && HasPlaceholderLabelOption());
  }

  void HTMLSelectElement::ListBoxSelectItem(int32 listIndex, bool allowMultiplySelections, bool shift,
                                            bool fireOnChangeNow) noexcept
  {
    if (!Multiple())
    {
      OptionSelectedByUser(ListToOptionIndex(listIndex), fireOnChangeNow, false);
    }
    else
    {
      UpdateSelectedState(listIndex, allowMultiplySelections, shift);
      UpdateValidity();
      if (fireOnChangeNow)
      {
        ListBoxOnChange();
      }
    }
  }

  bool HTMLSelectElement::UsesMenuList() const noexcept
  {
    if (RenderTheme::Singleton().DelegatesMenuListRendering())
    {
      return true;
    }

    return !_multiple && _size <= 1;
  }

  int32 HTMLSelectElement::ActiveSelectionStartListIndex() const noexcept
  {
    if (_activeSelectionAnchorIndex >= 0)
    {
      return _activeSelectionAnchorIndex;
    }
    return OptionToListIndex(SelectedIndex());
  }

  int32 HTMLSelectElement::ActiveSelectionEndListIndex() const noexcept
  {
    if (_activeSelectionEndIndex >= 0)
    {
      return _activeSelectionEndIndex;
    }
    return LastSelectedListIndex();
  }

  dom::ExceptionOr<void> HTMLSelectElement::Add(const OptionOrOptGroupElement &element,
                                                const Maybe<HTMLElementOrInt> &before) noexcept
  {
    // The options collection's add(element, before), which this acts like.
    CEReactionsScope ceReactions;
    Ref<HTMLElement> toInsert = std::visit([](const auto &htmlElement) -> Ref<HTMLElement>
                                           { return ShareRef<HTMLElement>(*htmlElement); }, element);
    RefPtr<HTMLElement> beforeElement;
    if (before && std::holds_alternative<RefPtr<HTMLElement>>(*before))
    {
      beforeElement = std::get<RefPtr<HTMLElement>>(*before);
    }

    // 1. If element is an ancestor of the select element on which this is rooted, then throw a
    //    "HierarchyRequestError" DOMException.
    if (dom::TreeQueries::IsAncestor(*toInsert, *this))
    {
      return dom::ExceptionCode::HierarchyRequestError;
    }

    // 2. If before is an element, but that element isn't a descendant of the select element on which this is
    //    rooted, then throw a "NotFoundError" DOMException.
    if (beforeElement && !dom::TreeQueries::IsDescendant(*beforeElement, *this))
    {
      return dom::ExceptionCode::NotFoundError;
    }

    // 3. If element and before are the same element, then return.
    if (beforeElement.get() == toInsert.get())
    {
      return {};
    }

    // 4. Let reference be null.
    // 5. If before is a node, then set reference to before. Otherwise, if before is an integer and there is a
    //    beforeth node in this, then set reference to that node.
    RefPtr<HTMLElement> reference = beforeElement;
    if (before && std::holds_alternative<int32>(*before) && std::get<int32>(*before) >= 0)
    {
      reference = ShareRefPtr<HTMLElement>(Item(static_cast<uint32>(std::get<int32>(*before))));
    }

    // 6. Let parent be reference's parent node if reference is not null; otherwise the select element on
    // which
    //    this is rooted.
    Ref<dom::ContainerNode> parent =
      reference ? ShareRef(*reference->ParentNode()) : ShareRef<dom::ContainerNode>(*this);

    // 7. Pre-insert element into parent node before reference.
    auto result = parent->InsertBefore(*toInsert, reference.get());
    if (result.HasException())
    {
      return result.ReleaseException();
    }
    return {};
  }

  void HTMLSelectElement::Remove(int32 optionIndex) noexcept
  {
    // To remove an option from the options collection given index:
    // https://html.spec.whatwg.org/#htmloptionscollection-remove-an-option
    CEReactionsScope ceReactions;

    // 1. If the number of nodes represented by collection is 0, then return.
    // 2. If index is not a number greater than or equal to 0 and less than the number of nodes represented by
    //    collection, then return.
    int32 listIndex = OptionToListIndex(optionIndex);
    if (listIndex < 0)
    {
      return;
    }

    // 3. Let element be the indexth element in collection.
    // 4. Remove element from its parent node.
    Ref<HTMLElement> item = ShareRef(*ListItems()[static_cast<size_t>(listIndex)]);
    (void)item->Remove();
  }

  dom::DOMString HTMLSelectElement::Value() const noexcept
  {
    for (auto &item : ListItems())
    {
      if (auto *option = DynamicDowncast<HTMLOptionElement>(item.get()))
      {
        if (option->Selected())
        {
          return option->Value();
        }
      }
    }
    return {};
  }

  // https://html.spec.whatwg.org/#dom-select-value
  void HTMLSelectElement::Value(dom::DOMStringView value) noexcept
  {
    // 1. Let firstMatchingOption be null.
    // 2. For each option of this's list of options:
    // 2a. Set option's selectedness to false.
    // 2b. If firstMatchingOption is null and option's value is equal to the given value, then set
    //     firstMatchingOption to option.
    // 3. If firstMatchingOption is not null, then set firstMatchingOption's selectedness to true and set
    //    firstMatchingOption's dirtiness to true.
    // 4. Update a select's descendant selectedcontent elements given this.
    // (Selecting the matching option's index, or none.)
    int32 optionIndex = 0;
    for (auto &item : ListItems())
    {
      if (auto *option = DynamicDowncast<HTMLOptionElement>(item.get()))
      {
        if (option->Value() == value)
        {
          SelectedIndex(optionIndex);
          return;
        }
        ++optionIndex;
      }
    }

    SelectedIndex(-1);
  }

  void HTMLSelectElement::DisabledStateChanged() noexcept
  {
    HTMLFormControlElement::DisabledStateChanged();

    // The options and optgroups in a disabled select are actually disabled, which :disabled and :enabled
    // match.
    // FIXME: Use PseudoClassChangeInvalidation instead for :has() support and more efficiency.
    InvalidateStyleForSubtree();
  }

  void HTMLSelectElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                             Maybe<dom::DOMStringView> value,
                                             dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLFormControlElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }
    if (localName == u8"size" || localName == u8"multiple")
    {
      ReassignSlots();
    }

    if (localName == u8"size")
    {
      uint32 oldSize = _size;
      uint32 size = LimitToOnlyHTMLNonNegative(value);

      // Ensure that we've determined selectedness of the items at least once prior to changing the size.
      if (oldSize != size)
      {
        UpdateListItemSelectedStates();
      }

      _size = size;
      UpdateValidity();
      if (_size != oldSize)
      {
        InvalidateStyleAndRenderersForSubtree();

        // Whenever a select element's size attribute is set, changed, or removed, reset a select's
        // selectedness given the select element and false.
        ResetSelectedness(false);
      }
    }
    else if (localName == u8"multiple")
    {
      ParseMultipleAttribute(value.has_value());
    }
  }

  bool HTMLSelectElement::CanSelectAll() const noexcept
  {
    return !UsesMenuList();
  }

  RenderPtr<RenderElement> HTMLSelectElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    if (UsesMenuList())
    {
      if (style.UsedAppearance() == StyleAppearance::BaseSelect)
      {
        return HTMLFormControlElement::CreateElementRenderer(krys::move(style));
      }
      return CreateRenderer<RenderMenuList>(*this, krys::move(style));
    }
    return CreateRenderer<RenderListBox>(*this, krys::move(style));
  }

  bool HTMLSelectElement::ChildShouldCreateRenderer(const dom::Node &child) const noexcept
  {
    if (!HTMLFormControlElement::ChildShouldCreateRenderer(child))
    {
      return false;
    }
    if (!UsesMenuList())
    {
      return Is<HTMLOptionElement>(child) || Is<HTMLOptGroupElement>(child)
             || &child == _selectListBoxSlot.get();
    }
    return _contentsUseBaseAppearance;
  }

  Ref<dom::HTMLCollection> HTMLSelectElement::SelectedOptions() noexcept
  {
    if (auto *selectedOptions = _selectedOptions.get())
    {
      return ShareRef(*selectedOptions);
    }
    Ref<dom::LiveHTMLCollection> selectedOptions = CreateRef<SelectedOptionsCollection>(*this);
    _selectedOptions = CreateWeakPtr<dom::HTMLCollection>(selectedOptions.get());
    return selectedOptions;
  }

  Ref<HTMLOptionsCollection> HTMLSelectElement::Options() noexcept
  {
    if (auto *options = _options.get())
    {
      // Only ever an HTMLOptionsCollection.
      return ShareRef(static_cast<HTMLOptionsCollection &>(*options));
    }
    auto options = CreateRef<HTMLOptionsCollection>(*this);
    _options = CreateWeakPtr<dom::HTMLCollection>(options.get());
    return options;
  }

  void HTMLSelectElement::UpdateListItemSelectedStates(AllowStyleInvalidation allowStyleInvalidation) noexcept
  {
    if (_shouldRecalcListItems)
    {
      RecalcListItems(true, allowStyleInvalidation);
    }
  }

  void HTMLSelectElement::OptionInserted(HTMLOptionElement &option) noexcept
  {
    if (Multiple())
    {
      return;
    }

    if (option.SelectedWithoutUpdate())
    {
      OptionSelectionStateChanged(option, true);
    }
    else
    {
      ScrollToSelection();
    }
  }

  void HTMLSelectElement::OptGroupInserted(HTMLOptGroupElement &optGroup) noexcept
  {
    if (Multiple())
    {
      return;
    }

    for (auto *child = optGroup.LastChild(); child; child = child->PreviousSibling())
    {
      if (auto *option = DynamicDowncast<HTMLOptionElement>(*child);
          option && option->SelectedWithoutUpdate())
      {
        OptionSelectionStateChanged(*option, true);
        return;
      }
    }
    ScrollToSelection();
  }

  void HTMLSelectElement::OnChildrenChanged() noexcept
  {
    ReassignSlots();
    SetRecalcListItems();
    UpdateValidity();
    _lastOnChangeSelection.clear();

    HTMLFormControlElement::OnChildrenChanged();
  }

  void HTMLSelectElement::OptionElementChildrenChanged() noexcept
  {
    SetOptionsChangedOnRenderer();
    InvalidateStyleForSubtree();
    UpdateValidity();
    CloneSelectedOptionIntoSelectButton();
  }

  HTMLOptionElement *HTMLSelectElement::NamedItem(dom::DOMStringView name) const noexcept
  {
    if (name.empty())
    {
      return nullptr;
    }
    for (auto &item : ListItems())
    {
      if (auto *option = DynamicDowncast<HTMLOptionElement>(item.get()))
      {
        if (option->GetAttribute(u8"id") == name || option->GetAttribute(u8"name") == name)
        {
          return option;
        }
      }
    }
    return nullptr;
  }

  HTMLOptionElement *HTMLSelectElement::Item(uint32 index) const noexcept
  {
    int32 listIndex = OptionToListIndex(static_cast<int32>(index));
    if (listIndex < 0)
    {
      return nullptr;
    }
    return &Downcast<HTMLOptionElement>(*ListItems()[static_cast<size_t>(listIndex)]);
  }

  dom::ExceptionOr<void> HTMLSelectElement::SetItem(uint32 index, HTMLOptionElement *option) noexcept
  {
    // Setting a new or existing indexed property of the options collection.
    CEReactionsScope ceReactions;

    // 1. If value is null, then remove an option from collection given index and return.
    if (!option)
    {
      Remove(static_cast<int32>(index));
      return {};
    }

    // 2. Let length be the number of nodes represented by collection.
    uint32 length = Length();

    // 3. Let delta be index − length.
    int64 delta = static_cast<int64>(index) - static_cast<int64>(length);

    // 4. If delta is greater than 0, then append new option elements to the select element on which
    // collection
    //    is rooted given delta.
    if (delta > 0)
    {
      // An index past the limit the length setter has does nothing, as in other browsers.
      if (index >= MaxSelectItems)
      {
        return {};
      }
      auto result = AppendNewOptionElements(static_cast<uint32>(delta));
      if (result.HasException())
      {
        return result;
      }
    }

    // 5. If delta is greater than or equal to 0, then append value to the select element on which collection
    //    is rooted. Otherwise, replace the indexth element in collection by value.
    if (delta >= 0)
    {
      auto result = AppendChild(*option);
      if (result.HasException())
      {
        return result.ReleaseException();
      }
      if (option->Selected())
      {
        OptionSelectionStateChanged(*option, true);
      }
    }
    else
    {
      Ref<HTMLOptionElement> item = ShareRef(*Item(index));
      auto result = item->ParentNode()->ReplaceChild(*option, *item);
      if (result.HasException())
      {
        return result.ReleaseException();
      }
    }

    return {};
  }

  dom::ExceptionOr<void> HTMLSelectElement::AppendNewOptionElements(uint32 count) noexcept
  {
    // 1. Let fragment be the result of creating a document fragment given select's node document.
    auto fragment = CreateRef<dom::DocumentFragment>(NodeDocument());

    // 2. Append count new option elements to fragment.
    for (uint32 i = 0; i < count; ++i)
    {
      auto result = fragment->AppendChild(
        *dom::ElementFactory::CreateHTMLElement<HTMLOptionElement>(NodeDocument(), u8"option"));
      if (result.HasException())
      {
        return result.ReleaseException();
      }
    }

    // 3. Append fragment to select.
    auto result = AppendChild(*fragment);
    if (result.HasException())
    {
      return result.ReleaseException();
    }
    return {};
  }

  dom::ExceptionOr<void> HTMLSelectElement::Length(uint32 newLength) noexcept
  {
    // The options collection's length setter, which this acts like.
    CEReactionsScope ceReactions;

    // 1. Let current be the number of nodes represented by the collection.
    uint32 current = Length();

    // 2. If the given value is greater than current:
    if (newLength > current)
    {
      // 2a. If the given value is greater than 100,000, then return.
      if (newLength > MaxSelectItems)
      {
        return {};
      }

      // 2b. Let n be value − current.
      // 2c. Append new option elements to the select element on which this is rooted given n.
      return AppendNewOptionElements(newLength - current);
    }

    // 3. If the given value is less than current:
    // 3a. Let n be current − value.
    // 3b. Remove the last n nodes in the collection from their parent nodes.
    {
      // Removing children fires mutation events, which might mutate the DOM further, so we first copy out a
      // list of elements that we intend to remove then attempt to remove them one at a time.
      List<Ref<HTMLOptionElement>> itemsToRemove;
      uint32 optionIndex = 0;
      for (auto &item : ListItems())
      {
        auto *option = DynamicDowncast<HTMLOptionElement>(item.get());
        if (option && optionIndex++ >= newLength)
        {
          krys_debug_assert(item->ParentNode());
          itemsToRemove.push_back(ShareRef(*option));
        }
      }

      for (auto &item : itemsToRemove)
      {
        (void)item->Remove();
      }
    }
    return {};
  }

  // Returns the first valid item `skip` items from `listIndex` in `direction` if there is one. Otherwise, it
  // returns the valid item closest to that boundary which is past `listIndex` if there is one. Otherwise, it
  // returns `listIndex`. Valid means that it is enabled and an option element.
  int32 HTMLSelectElement::NextValidIndex(int32 listIndex, SkipDirection direction, int32 skip) const noexcept
  {
    auto step = static_cast<int32>(direction);
    auto &listItems = ListItems();
    int32 lastGoodIndex = listIndex;
    auto size = static_cast<int32>(listItems.size());
    for (listIndex += step; listIndex >= 0 && listIndex < size; listIndex += step)
    {
      --skip;
      auto &listItem = *listItems[static_cast<size_t>(listIndex)];
      if (!listItem.IsDisabledFormControl() && Is<HTMLOptionElement>(listItem))
      {
        lastGoodIndex = listIndex;
        if (skip <= 0)
        {
          break;
        }
      }
    }
    return lastGoodIndex;
  }

  int32 HTMLSelectElement::NextSelectableListIndex(int32 startIndex) const noexcept
  {
    return NextValidIndex(startIndex, SkipDirection::Forwards, 1);
  }

  int32 HTMLSelectElement::PreviousSelectableListIndex(int32 startIndex) const noexcept
  {
    if (startIndex == -1)
    {
      startIndex = static_cast<int32>(ListItems().size());
    }
    return NextValidIndex(startIndex, SkipDirection::Backwards, 1);
  }

  int32 HTMLSelectElement::FirstSelectableListIndex() const noexcept
  {
    auto &items = ListItems();
    int32 index = NextValidIndex(static_cast<int32>(items.size()), SkipDirection::Backwards, INT_MAX);
    if (static_cast<size_t>(index) == items.size())
    {
      return -1;
    }
    return index;
  }

  int32 HTMLSelectElement::LastSelectableListIndex() const noexcept
  {
    return NextValidIndex(-1, SkipDirection::Forwards, INT_MAX);
  }

  // Returns the index of the next valid item one page away from `startIndex` in `direction`.
  int32 HTMLSelectElement::NextSelectableListIndexPageAway(int32 startIndex,
                                                           SkipDirection direction) const noexcept
  {
    auto &items = ListItems();

    // Can't use _size because the renderer forces a minimum size.
    int32 pageSize = 0;
    if (auto *renderListBox = DynamicDowncast<RenderListBox>(Renderer()))
    {
      // -1 so we still show context.
      pageSize = static_cast<int32>(renderListBox->Size()) - 1;
    }

    // One page away, but not outside valid bounds. If there is a valid option item one page away, the index
    // is chosen. If there is no exact one page away valid option, returns startIndex or the most far index.
    int32 edgeIndex = direction == SkipDirection::Forwards ? 0 : static_cast<int32>(items.size()) - 1;
    int32 skipAmount =
      pageSize + (direction == SkipDirection::Forwards ? startIndex : edgeIndex - startIndex);
    return NextValidIndex(edgeIndex, direction, skipAmount);
  }

  void HTMLSelectElement::SelectAll() noexcept
  {
    krys_debug_assert(!UsesMenuList());
    if (!Renderer() || !_multiple)
    {
      return;
    }

    // Save the selection so it can be compared to the new SelectAll selection when dispatching change events.
    SaveLastSelection();

    _activeSelectionState = true;
    SetActiveSelectionAnchorIndex(NextSelectableListIndex(-1));
    SetActiveSelectionEndIndex(PreviousSelectableListIndex(-1));
    if (_activeSelectionAnchorIndex < 0)
    {
      return;
    }

    UpdateListBoxSelection(false);
    ListBoxOnChange();
    UpdateValidity();
  }

  void HTMLSelectElement::SaveLastSelection() noexcept
  {
    if (UsesMenuList())
    {
      _lastOnChangeIndex = SelectedIndex();
      return;
    }

    _lastOnChangeSelection.clear();
    for (auto &element : ListItems())
    {
      auto *option = DynamicDowncast<HTMLOptionElement>(element.get());
      _lastOnChangeSelection.push_back(option && option->Selected());
    }
  }

  void HTMLSelectElement::SetActiveSelectionAnchorIndex(int32 index) noexcept
  {
    _activeSelectionAnchorIndex = index;

    // Cache the selection state so we can restore the old selection as the new selection pivots around this
    // anchor index.
    _cachedStateForActiveSelection.clear();
    for (auto &element : ListItems())
    {
      auto *option = DynamicDowncast<HTMLOptionElement>(element.get());
      _cachedStateForActiveSelection.push_back(option && option->Selected());
    }
  }

  void HTMLSelectElement::SetActiveSelectionEndIndex(int32 index) noexcept
  {
    _activeSelectionEndIndex = index;
  }

  void HTMLSelectElement::UpdateListBoxSelection(bool deselectOtherOptions) noexcept
  {
    krys_debug_assert(Renderer());
    krys_debug_assert(ListItems().empty() || _activeSelectionAnchorIndex >= 0);

    auto start = static_cast<uint32>(std::min(_activeSelectionAnchorIndex, _activeSelectionEndIndex));
    auto end = static_cast<uint32>(std::max(_activeSelectionAnchorIndex, _activeSelectionEndIndex));

    auto &items = ListItems();
    for (uint32 i = 0; i < items.size(); ++i)
    {
      auto *element = DynamicDowncast<HTMLOptionElement>(items[i].get());
      if (!element || element->IsDisabledFormControl())
      {
        continue;
      }

      if (i >= start && i <= end)
      {
        element->SetSelectedState(_activeSelectionState);
      }
      else if (deselectOtherOptions || i >= _cachedStateForActiveSelection.size())
      {
        element->SetSelectedState(false);
      }
      else
      {
        element->SetSelectedState(_cachedStateForActiveSelection[i]);
      }
    }

    InvalidateSelectedItems();
    ScrollToSelection();
    UpdateValidity();
  }

  void HTMLSelectElement::ListBoxOnChange() noexcept
  {
    krys_debug_assert(!UsesMenuList() || _multiple);

    auto &items = ListItems();

    // If the cached selection list is empty, or the size has changed, then fire
    // DispatchFormControlChangeEvent, and return early.
    if (_lastOnChangeSelection.empty() || _lastOnChangeSelection.size() != items.size())
    {
      DispatchFormControlChangeEvent();
      return;
    }

    // Update _lastOnChangeSelection and fire DispatchFormControlChangeEvent.
    bool fireOnChange = false;
    for (size_t i = 0; i < items.size(); ++i)
    {
      auto *option = DynamicDowncast<HTMLOptionElement>(items[i].get());
      bool selected = option && option->Selected();
      if (selected != _lastOnChangeSelection[i])
      {
        fireOnChange = true;
      }
      _lastOnChangeSelection[i] = selected;
    }

    if (fireOnChange)
    {
      DispatchFormControlInputEvent();
      DispatchFormControlChangeEvent();
    }
  }

  void HTMLSelectElement::DispatchChangeEventForMenuList() noexcept
  {
    krys_debug_assert(UsesMenuList());

    int32 selected = SelectedIndex();
    if (_lastOnChangeIndex != selected && _isProcessingUserDrivenChange)
    {
      _lastOnChangeIndex = selected;
      _isProcessingUserDrivenChange = false;
      DispatchFormControlInputEvent();
      DispatchFormControlChangeEvent();
    }
  }

  void HTMLSelectElement::ScrollToSelection() noexcept
  {
    if (UsesMenuList())
    {
      return;
    }

    if (auto *renderer = DynamicDowncast<RenderListBox>(Renderer()))
    {
      renderer->SelectionChanged();
    }
  }

  void HTMLSelectElement::SetOptionsChangedOnRenderer() noexcept
  {
    if (auto *renderMenuList = DynamicDowncast<RenderMenuList>(Renderer()))
    {
      renderMenuList->SetOptionsChanged(true);
    }
    else if (auto *renderListBox = DynamicDowncast<RenderListBox>(Renderer()))
    {
      renderListBox->SetOptionsChanged(true);
    }
  }

  void HTMLSelectElement::ShowPopup() noexcept
  {
    if (auto *renderMenuList = DynamicDowncast<RenderMenuList>(Renderer()))
    {
      // ShowPopup() may run script and cause the renderer to get destroyed.
      renderMenuList->ShowPopup();
    }
  }

  const List<WeakPtr<HTMLElement>> &HTMLSelectElement::ListItems() const noexcept
  {
    if (_shouldRecalcListItems)
    {
      RecalcListItems();
    }
    else
    {
#if KRYS_DEBUG
      auto items = _listItems;
      RecalcListItems(false);
      krys_debug_assert(
        std::ranges::equal(items, _listItems, [](auto &a, auto &b) { return a.get() == b.get(); }));
#endif
    }

    return _listItems;
  }

  void HTMLSelectElement::InvalidateSelectedItems() noexcept
  {
    if (auto *selectedOptions = _selectedOptions.get())
    {
      static_cast<dom::LiveHTMLCollection *>(selectedOptions)->InvalidateCache();
    }
  }

  void HTMLSelectElement::ResetSelectedness(bool hasChanged) noexcept
  {
    // 1. If the result of running the selectedness setting algorithm given select is true, then set
    // hasChanged
    //    to true.
    // (Running it while finding the list of options again, and comparing the selected option.)
    RefPtr<HTMLOptionElement> previouslySelected = ShareRefPtr(FirstSelectedOption());
    SetRecalcListItems();
    UpdateListItemSelectedStates();
    UpdateValidity();
    if (FirstSelectedOption() != previouslySelected.get())
    {
      hasChanged = true;
    }

    // 2. If hasChanged is false, then return.
    if (!hasChanged)
    {
      return;
    }

    // 3. Update a select's descendant selectedcontent elements given select.
    FormControlAlgorithms::UpdateDescendantSelectedContents(*this);
  }

  HTMLOptionElement *HTMLSelectElement::FirstSelectedOption() const noexcept
  {
    int32 index = SelectedIndex();
    return index >= 0 ? Item(static_cast<uint32>(index)) : nullptr;
  }

  void HTMLSelectElement::SetRecalcListItems() noexcept
  {
    _shouldRecalcListItems = true;
    // Manual selection anchor is reset when manipulating the select programmatically.
    _activeSelectionAnchorIndex = -1;
    SetOptionsChangedOnRenderer();
    InvalidateStyleForSubtree();
    InvalidateSelectedItems();

    auto &document = NodeDocument();
    if (this == document.FocusedElement())
    {
      if (auto *page = document.GetPage())
      {
        page->GetChromeClient().FocusedSelectElementDidChangeOptions(*this);
      }
    }
  }

  void HTMLSelectElement::RecalcListItems(bool updateSelectedStates,
                                          AllowStyleInvalidation allowStyleInvalidation) const noexcept
  {
    _listItems.clear();

    _shouldRecalcListItems = false;

    // https://html.spec.whatwg.org/#selectedness-setting-algorithm
    // (Steps 4 to 6, while finding the list of options; an option selected for being the first enabled one
    // is deselected by a later selected one.)
    RefPtr<HTMLOptionElement> foundSelected;
    auto handleOptionElement = [&](HTMLOptionElement &option)
    {
      _listItems.push_back(CreateWeakPtr(&option));
      if (updateSelectedStates && !_multiple)
      {
        if (option.SelectedWithoutUpdate())
        {
          if (foundSelected)
          {
            foundSelected->SetSelectedState(false, allowStyleInvalidation);
          }
          foundSelected = ShareRef(option);
        }
        else if (_size <= 1 && !foundSelected && !option.IsDisabledFormControl())
        {
          foundSelected = ShareRef(option);
          foundSelected->SetSelectedState(true, allowStyleInvalidation);
        }
      }
    };

    auto &select = const_cast<HTMLSelectElement &>(*this);
    for (auto *node = dom::TreeQueries::Next(select, &select); node;)
    {
      auto *descendant = DynamicDowncast<HTMLElement>(*node);
      if (!descendant)
      {
        node = dom::TreeQueries::Next(*node, &select);
        continue;
      }
      if (auto *option = DynamicDowncast<HTMLOptionElement>(*descendant))
      {
        handleOptionElement(*option);
        node = dom::TreeQueries::NextSkippingChildren(*node, &select);
        continue;
      }
      if (Is<HTMLOptGroupElement>(*descendant))
      {
        _listItems.push_back(CreateWeakPtr(descendant));
        for (auto *optGroupNode = dom::TreeQueries::Next(*descendant, descendant); optGroupNode;)
        {
          auto *optGroupDescendant = DynamicDowncast<HTMLElement>(*optGroupNode);
          if (!optGroupDescendant)
          {
            optGroupNode = dom::TreeQueries::Next(*optGroupNode, descendant);
            continue;
          }
          if (auto *option = DynamicDowncast<HTMLOptionElement>(*optGroupDescendant))
          {
            handleOptionElement(*option);
            optGroupNode = dom::TreeQueries::NextSkippingChildren(*optGroupNode, descendant);
            continue;
          }
          if (Is<HTMLOptGroupElement>(*optGroupDescendant) || Is<HTMLDataListElement>(*optGroupDescendant)
              || Is<HTMLSelectElement>(*optGroupDescendant) || Is<HTMLHRElement>(*optGroupDescendant))
          {
            optGroupNode = dom::TreeQueries::NextSkippingChildren(*optGroupNode, descendant);
            continue;
          }
          optGroupNode = dom::TreeQueries::Next(*optGroupNode, descendant);
        }
        node = dom::TreeQueries::NextSkippingChildren(*node, &select);
        continue;
      }
      if (Is<HTMLHRElement>(*descendant))
      {
        _listItems.push_back(CreateWeakPtr(descendant));
        node = dom::TreeQueries::NextSkippingChildren(*node, &select);
        continue;
      }
      if (Is<HTMLDataListElement>(*descendant) || Is<HTMLSelectElement>(*descendant))
      {
        node = dom::TreeQueries::NextSkippingChildren(*node, &select);
        continue;
      }
      node = dom::TreeQueries::Next(*node, &select);
    }
  }

  int32 HTMLSelectElement::SelectedIndex() const noexcept
  {
    int32 index = 0;

    // Return the number of the first option selected.
    for (auto &element : ListItems())
    {
      if (auto *option = DynamicDowncast<HTMLOptionElement>(element.get()))
      {
        if (option->Selected())
        {
          return index;
        }
        ++index;
      }
    }

    return -1;
  }

  // https://html.spec.whatwg.org/#set-the-selected-index
  void HTMLSelectElement::SelectedIndex(int32 index) noexcept
  {
    // 1. Let firstMatchingOption be null.
    // 2. For each option of select's list of options:
    // 2a. Set option's selectedness to false.
    // 2b. If firstMatchingOption is null and option's index is equal to value, then set firstMatchingOption
    // to
    //     option.
    // 3. If firstMatchingOption is not null, then set firstMatchingOption's selectedness to true and set
    //    firstMatchingOption's dirtiness to true.
    SelectOption(index, {SelectOptionFlag::DeselectOtherOptions, SelectOptionFlag::MakeOptionDirty});

    // 4. Update a select's descendant selectedcontent elements given select.
    FormControlAlgorithms::UpdateDescendantSelectedContents(*this);
  }

  void HTMLSelectElement::OptionSelectionStateChanged(HTMLOptionElement &option,
                                                      bool optionIsSelected) noexcept
  {
    krys_debug_assert(option.OwnerSelectElement() == this);
    if (optionIsSelected)
    {
      SelectOption(option.Index());
    }
    else if (!UsesMenuList())
    {
      SelectOption(-1);
    }
    else
    {
      SelectOption(NextSelectableListIndex(-1));
    }
  }

  void HTMLSelectElement::SelectOption(int32 optionIndex, SelectOptionFlags flags) noexcept
  {
    bool shouldDeselect = !_multiple || flags.Contains(SelectOptionFlag::DeselectOtherOptions);

    auto &items = ListItems();
    int32 listIndex = OptionToListIndex(optionIndex);

    RefPtr<HTMLElement> element;
    if (listIndex >= 0)
    {
      element = ShareRefPtr(items[static_cast<size_t>(listIndex)].get());
    }

    if (shouldDeselect)
    {
      DeselectItemsWithoutValidation(element.get());
    }

    if (auto *option = DynamicDowncast<HTMLOptionElement>(element.get()))
    {
      if (_activeSelectionAnchorIndex < 0 || shouldDeselect)
      {
        SetActiveSelectionAnchorIndex(listIndex);
      }
      if (_activeSelectionEndIndex < 0 || shouldDeselect)
      {
        SetActiveSelectionEndIndex(listIndex);
      }
      option->SetSelectedState(true);
      if (flags.ContainsAny({SelectOptionFlag::MakeOptionDirty, SelectOptionFlag::UserDriven}))
      {
        option->MakeDirty();
      }
    }

    // Picking an option sends select update notifications, which update the selectedcontent elements.
    if (flags.Contains(SelectOptionFlag::UserDriven))
    {
      FormControlAlgorithms::UpdateDescendantSelectedContents(*this);
    }

    InvalidateSelectedItems();
    UpdateValidity();

    // For the menu list case, this is what makes the selected element appear.
    if (auto *renderer = Renderer())
    {
      renderer->UpdateFromElement();
    }

    ScrollToSelection();

    if (UsesMenuList())
    {
      _isProcessingUserDrivenChange = flags.Contains(SelectOptionFlag::UserDriven);
      if (flags.Contains(SelectOptionFlag::DispatchChangeEvent))
      {
        DispatchChangeEventForMenuList();
      }
      if (auto *renderMenuList = DynamicDowncast<RenderMenuList>(Renderer()))
      {
        renderMenuList->DidSetSelectedIndex(listIndex);
      }
      else if (auto *renderListBox = DynamicDowncast<RenderListBox>(Renderer()))
      {
        renderListBox->SelectionChanged();
      }
    }
  }

  int32 HTMLSelectElement::OptionToListIndex(int32 optionIndex) const noexcept
  {
    auto &items = ListItems();
    auto listSize = static_cast<int32>(items.size());
    if (optionIndex < 0 || optionIndex >= listSize)
    {
      return -1;
    }

    int32 optionIndex2 = -1;
    for (int32 listIndex = 0; listIndex < listSize; ++listIndex)
    {
      if (Is<HTMLOptionElement>(*items[static_cast<size_t>(listIndex)]))
      {
        ++optionIndex2;
        if (optionIndex2 == optionIndex)
        {
          return listIndex;
        }
      }
    }

    return -1;
  }

  int32 HTMLSelectElement::ListToOptionIndex(int32 listIndex) const noexcept
  {
    auto &items = ListItems();
    if (listIndex < 0 || listIndex >= static_cast<int32>(items.size())
        || !Is<HTMLOptionElement>(*items[static_cast<size_t>(listIndex)]))
    {
      return -1;
    }

    // Actual index of option not counting optgroup entries that may be in list.
    int32 optionIndex = 0;
    for (int32 i = 0; i < listIndex; ++i)
    {
      if (Is<HTMLOptionElement>(*items[static_cast<size_t>(i)]))
      {
        ++optionIndex;
      }
    }

    return optionIndex;
  }

  void HTMLSelectElement::DispatchFocusEvent(dom::EventTarget *relatedTarget,
                                             const dom::FocusOptions &options) noexcept
  {
    // Save the selection so it can be compared to the new selection when dispatching change events during
    // blur event dispatch.
    if (UsesMenuList())
    {
      SaveLastSelection();
    }
    HTMLFormControlElement::DispatchFocusEvent(relatedTarget, options);
  }

  void HTMLSelectElement::DispatchBlurEvent(dom::EventTarget *relatedTarget) noexcept
  {
    // We only need to fire change events here for menu lists, because we fire change events for list boxes
    // whenever the selection change is actually made. This matches other browsers' behavior.
    if (UsesMenuList())
    {
      DispatchChangeEventForMenuList();
    }
    HTMLFormControlElement::DispatchBlurEvent(relatedTarget);
  }

  void HTMLSelectElement::DeselectItemsWithoutValidation(HTMLElement *excludeElement) noexcept
  {
    for (auto &element : ListItems())
    {
      if (element.get() == excludeElement)
      {
        continue;
      }
      if (auto *option = DynamicDowncast<HTMLOptionElement>(element.get()))
      {
        option->SetSelectedState(false);
      }
    }
    InvalidateSelectedItems();
  }

  void HTMLSelectElement::ParseMultipleAttribute(bool present) noexcept
  {
    bool oldUsesMenuList = UsesMenuList();
    bool oldMultiple = _multiple;
    int32 oldSelectedIndex = SelectedIndex();
    _multiple = present;
    UpdateValidity();
    if (oldUsesMenuList != UsesMenuList())
    {
      InvalidateStyleAndRenderersForSubtree();
    }
    // https://html.spec.whatwg.org/#reset-a-select's-selectedness
    if (oldMultiple != _multiple)
    {
      // 1. If the multiple attribute was removed, then set the selectedness of all but the first option
      // element
      //    with its selectedness set to true in select's list of options to false.
      if (oldSelectedIndex >= 0)
      {
        SelectOption(oldSelectedIndex, {SelectOptionFlag::DeselectOtherOptions});
      }
      // 2. Reset a select's selectedness given select and true.
      ResetSelectedness(true);
    }
  }

  bool HTMLSelectElement::AppendFormData(DOMFormData &formData) noexcept
  {
    auto name = Name();
    if (name.empty())
    {
      return false;
    }

    bool successful = false;
    for (auto &element : ListItems())
    {
      auto *option = DynamicDowncast<HTMLOptionElement>(element.get());
      if (option && option->Selected() && !option->IsDisabledFormControl())
      {
        formData.Append(name, option->Value());
        successful = true;
      }
    }

    // It's possible that this is a menu list with multiple options and nothing will be submitted
    // (!successful). We won't send an unselected non-disabled option as fallback. This behavior matches other
    // browsers.
    return successful;
  }

  // https://html.spec.whatwg.org/#the-select-element:concept-form-reset-control
  void HTMLSelectElement::Reset() noexcept
  {
    // 1. Set selectElement's user validity to false.
    // (SetInteractedWithSinceLastFormSubmitEvent below.)
    RefPtr<HTMLOptionElement> firstOption;
    RefPtr<HTMLOptionElement> selectedOption;

    // 2. For each optionElement of selectElement's list of options:
    for (auto &element : ListItems())
    {
      auto *option = DynamicDowncast<HTMLOptionElement>(element.get());
      if (!option)
      {
        continue;
      }

      // 2b. Set optionElement's dirtiness to false.
      option->ResetDirtiness();

      // 2a. If optionElement has a selected attribute, then set optionElement's selectedness to true;
      // otherwise
      //     set it to false.
      // 3. Run the selectedness setting algorithm given selectElement.
      // (Both at once, keeping the last selected option and the first enabled one.)
      if (option->HasAttribute(u8"selected"))
      {
        if (selectedOption && !_multiple)
        {
          selectedOption->SetSelectedState(false);
        }
        option->SetSelectedState(true);
        selectedOption = ShareRef(*option);
      }
      else
      {
        option->SetSelectedState(false);
      }

      if (!firstOption && !option->IsDisabledFormControl())
      {
        firstOption = ShareRef(*option);
      }
    }

    if (!selectedOption && firstOption && !_multiple && _size <= 1)
    {
      firstOption->SetSelectedState(true);
    }

    // 4. Update a select's descendant selectedcontent elements given selectElement.
    FormControlAlgorithms::UpdateDescendantSelectedContents(*this);
    SetInteractedWithSinceLastFormSubmitEvent(false);
    InvalidateSelectedItems();
    SetOptionsChangedOnRenderer();
    InvalidateStyleForSubtree();
    UpdateValidity();
  }

  bool HTMLSelectElement::PlatformHandleKeydownEvent(dom::KeyboardEvent &event) noexcept
  {
    if (!RenderTheme::Singleton().PopsMenuByArrowKeys())
    {
      return false;
    }

    if (!NodeDocument().GetSettings().SpatialNavigationEnabled)
    {
      if (event.KeyIdentifier() == u8"Down" || event.KeyIdentifier() == u8"Up")
      {
        Focus();
        NodeDocument().UpdateStyleIfNeeded();
        // Calling Focus() may cause us to lose our renderer. Return true so that our caller doesn't process
        // the event further, but don't set the event as handled.
        if (!UsesMenuList() || !Renderer())
        {
          return true;
        }

        // Save the selection so it can be compared to the new selection when dispatching change events
        // during SelectOption, which gets called from the menu list when the user makes a selection from the
        // menu.
        SaveLastSelection();
        ShowPopup();
        event.SetDefaultHandled();
      }
      return true;
    }

    return false;
  }

  void HTMLSelectElement::MenuListDefaultEventHandler(dom::Event &event) noexcept
  {
    if (event.Type() == EventNames::KeyDown)
    {
      auto *keyboardEvent = DynamicDowncast<dom::KeyboardEvent>(event);
      if (!keyboardEvent)
      {
        return;
      }

      if (PlatformHandleKeydownEvent(*keyboardEvent))
      {
        return;
      }

      // When using spatial navigation, we want to be able to navigate away from the select element when the
      // user hits any of the arrow keys, instead of changing the selection.
      if (NodeDocument().GetSettings().SpatialNavigationEnabled)
      {
        if (!_activeSelectionState)
        {
          return;
        }
      }

      auto keyIdentifier = keyboardEvent->KeyIdentifier();
      bool handled = true;
      auto &listItems = ListItems();
      int32 listIndex = OptionToListIndex(SelectedIndex());

      // When using caret browsing, we want to be able to move the focus out of the select element when user
      // hits a left or right arrow key.
      if (NodeDocument().GetSettings().CaretBrowsingEnabled)
      {
        if (keyIdentifier == u8"Left" || keyIdentifier == u8"Right")
        {
          return;
        }
      }

      if (keyIdentifier == u8"Down" || keyIdentifier == u8"Right")
      {
        listIndex = NextValidIndex(listIndex, SkipDirection::Forwards, 1);
      }
      else if (keyIdentifier == u8"Up" || keyIdentifier == u8"Left")
      {
        listIndex = NextValidIndex(listIndex, SkipDirection::Backwards, 1);
      }
      else if (keyIdentifier == u8"PageDown")
      {
        listIndex = NextValidIndex(listIndex, SkipDirection::Forwards, 3);
      }
      else if (keyIdentifier == u8"PageUp")
      {
        listIndex = NextValidIndex(listIndex, SkipDirection::Backwards, 3);
      }
      else if (keyIdentifier == u8"Home")
      {
        listIndex = NextValidIndex(-1, SkipDirection::Forwards, 1);
      }
      else if (keyIdentifier == u8"End")
      {
        listIndex = NextValidIndex(static_cast<int32>(listItems.size()), SkipDirection::Backwards, 1);
      }
      else
      {
        handled = false;
      }

      if (handled && static_cast<size_t>(listIndex) < listItems.size())
      {
        SelectOption(ListToOptionIndex(listIndex),
                     {SelectOptionFlag::DeselectOtherOptions, SelectOptionFlag::DispatchChangeEvent,
                      SelectOptionFlag::UserDriven});
      }

      if (handled)
      {
        keyboardEvent->SetDefaultHandled();
      }
    }

    // Use key press event here since sending simulated mouse events on key down blocks the proper sending of
    // the key press event.
    if (event.Type() == EventNames::KeyPress)
    {
      auto *keyboardEvent = DynamicDowncast<dom::KeyboardEvent>(event);
      if (!keyboardEvent)
      {
        return;
      }

      int32 keyCode = keyboardEvent->KeyCode();
      bool handled = false;

      if (keyCode == ' ' && NodeDocument().GetSettings().SpatialNavigationEnabled)
      {
        // Use space to toggle arrow key handling for selection change or spatial navigation.
        _activeSelectionState = !_activeSelectionState;
        keyboardEvent->SetDefaultHandled();
        return;
      }

      if (RenderTheme::Singleton().PopsMenuBySpaceOrReturn())
      {
        if (keyCode == ' ' || keyCode == '\r')
        {
          Focus();
          NodeDocument().UpdateStyleIfNeeded();

          // Calling Focus() may remove the renderer or change the renderer type.
          if (!UsesMenuList() || !Renderer())
          {
            return;
          }

          // Save the selection so it can be compared to the new selection when dispatching change events
          // during SelectOption, which gets called from the menu list when the user makes a selection from
          // the menu.
          SaveLastSelection();
          ShowPopup();
          handled = true;
        }
      }
      else if (RenderTheme::Singleton().PopsMenuByArrowKeys())
      {
        if (keyCode == ' ')
        {
          Focus();
          NodeDocument().UpdateStyleIfNeeded();

          // Calling Focus() may remove the renderer or change the renderer type.
          if (!UsesMenuList() || !Renderer())
          {
            return;
          }

          // Save the selection so it can be compared to the new selection when dispatching change events
          // during SelectOption, which gets called from the menu list when the user makes a selection from
          // the menu.
          SaveLastSelection();
          ShowPopup();
          handled = true;
        }
        else if (keyCode == '\r')
        {
          if (RefPtr<HTMLFormElement> form = ShareRefPtr(Form()))
          {
            form->SubmitImplicitly(*keyboardEvent, false);
          }
          DispatchChangeEventForMenuList();
          handled = true;
        }
      }

      if (handled)
      {
        keyboardEvent->SetDefaultHandled();
      }
    }

    if (auto *mouseEvent = DynamicDowncast<dom::MouseEvent>(event);
        mouseEvent && event.Type() == EventNames::MouseDown && mouseEvent->Button() == 0)
    {
      Focus();
      NodeDocument().UpdateStyleIfNeeded();

      if (UsesMenuList() && Renderer())
      {
        // Save the selection so it can be compared to the new selection when we call onChange during
        // SelectOption, which gets called from the menu list when the user makes a selection from the menu.
        SaveLastSelection();
        ShowPopup();
      }
      event.SetDefaultHandled();
    }

    if (event.Type() == EventNames::Blur && !Focused())
    {
      auto &menuList = Downcast<RenderMenuList>(*Renderer());
      if (menuList.PopupIsVisible())
      {
        menuList.HidePopup();
      }
    }
  }

  void HTMLSelectElement::UpdateSelectedState(int32 listIndex, bool multi, bool shift) noexcept
  {
    auto &items = ListItems();
    auto listSize = static_cast<int32>(items.size());
    if (listIndex < 0 || listIndex >= listSize)
    {
      return;
    }

    Ref<HTMLElement> clickedElement = ShareRef(*items[static_cast<size_t>(listIndex)]);
    if (Is<HTMLOptGroupElement>(*clickedElement))
    {
      return;
    }

    // Save the selection so it can be compared to the new selection when dispatching change events during
    // mouseup, or after autoscroll finishes.
    SaveLastSelection();

    _activeSelectionState = true;

    bool shiftSelect = _multiple && shift;
    bool multiSelect = _multiple && multi && !shift;

    if (auto *option = DynamicDowncast<HTMLOptionElement>(*clickedElement))
    {
      // Keep track of whether an active selection (like during drag selection), should select or deselect.
      if (option->Selected() && multiSelect)
      {
        _activeSelectionState = false;
      }
      if (!_activeSelectionState)
      {
        option->SetSelectedState(false);
      }
    }

    // If we're not in any special multiple selection mode, then deselect all other items, excluding the
    // clicked option. If no option was clicked, then this will deselect all items in the list.
    if (!shiftSelect && !multiSelect)
    {
      DeselectItemsWithoutValidation(clickedElement.get());
    }

    // If the anchor hasn't been set, and we're doing a single selection or a shift selection, then initialize
    // the anchor to the first selected index.
    if (_activeSelectionAnchorIndex < 0 && !multiSelect)
    {
      SetActiveSelectionAnchorIndex(SelectedIndex());
    }

    // Set the selection state of the clicked option.
    if (auto *option = DynamicDowncast<HTMLOptionElement>(*clickedElement);
        option && !option->IsDisabledFormControl())
    {
      option->SetSelectedState(true);
      option->MakeDirty();
    }

    // If there was no SelectedIndex() for the previous initialization, or if we're doing a single selection,
    // or a multiple selection (using cmd or ctrl), then initialize the anchor index to the listIndex that
    // just got clicked.
    if (_activeSelectionAnchorIndex < 0 || !shiftSelect)
    {
      SetActiveSelectionAnchorIndex(listIndex);
    }

    InvalidateSelectedItems();
    SetActiveSelectionEndIndex(listIndex);
    UpdateListBoxSelection(!multiSelect);

    // Picking an option sends select update notifications, which update the selectedcontent elements.
    FormControlAlgorithms::UpdateDescendantSelectedContents(*this);
  }

  void HTMLSelectElement::ListBoxDefaultEventHandler(dom::Event &event) noexcept
  {
    KRYS_MAYBE_UNUSED auto &listItems = ListItems();

    auto *mouseEvent = DynamicDowncast<dom::MouseEvent>(event);
    if (event.Type() == EventNames::MouseDown && mouseEvent && mouseEvent->Button() == 0)
    {
      Focus();
      NodeDocument().UpdateStyleIfNeeded();

      // Calling Focus() may remove or change our renderer, in which case we don't want to handle the event
      // further.
      auto *renderListBox = DynamicDowncast<RenderListBox>(Renderer());
      if (!renderListBox)
      {
        return;
      }

      // Convert to coords relative to the list box if needed.
      auto absoluteLocation = mouseEvent->AbsoluteLocation();
      IntPoint localOffset = RoundedIntPoint(renderListBox->AbsoluteToLocal(
        FloatPoint(static_cast<float>(absoluteLocation.X()), static_cast<float>(absoluteLocation.Y())),
        {MapCoordinatesMode::UseTransforms}));
      int32 listIndex = renderListBox->ListIndexAtOffset(LayoutSize(localOffset.X(), localOffset.Y()));
      if (listIndex >= 0)
      {
        if (!IsDisabledFormControl())
        {
          UpdateSelectedState(listIndex, mouseEvent->CtrlKey(), mouseEvent->ShiftKey());
        }
        if (auto *frame = NodeDocument().Frame())
        {
          frame->GetEventHandler().SetMouseDownMayStartAutoscroll();
        }

        mouseEvent->SetDefaultHandled();
      }
    }
    else if (event.Type() == EventNames::MouseMove && mouseEvent)
    {
      auto &renderListBox = Downcast<RenderListBox>(*Renderer());
      if (renderListBox.CanBeScrolledAndHasScrollableArea())
      {
        return;
      }

      if (mouseEvent->Button() != 0 || !mouseEvent->ButtonDown())
      {
        return;
      }

      auto absoluteLocation = mouseEvent->AbsoluteLocation();
      IntPoint localOffset = RoundedIntPoint(renderListBox.AbsoluteToLocal(
        FloatPoint(static_cast<float>(absoluteLocation.X()), static_cast<float>(absoluteLocation.Y())),
        {MapCoordinatesMode::UseTransforms}));
      int32 listIndex = renderListBox.ListIndexAtOffset(LayoutSize(localOffset.X(), localOffset.Y()));
      if (listIndex >= 0)
      {
        if (!IsDisabledFormControl())
        {
          if (_multiple)
          {
            // Only extend selection if there is something selected.
            if (_activeSelectionAnchorIndex < 0)
            {
              return;
            }

            SetActiveSelectionEndIndex(listIndex);
            UpdateListBoxSelection(false);
          }
          else
          {
            SetActiveSelectionAnchorIndex(listIndex);
            SetActiveSelectionEndIndex(listIndex);
            UpdateListBoxSelection(true);
          }
        }
        mouseEvent->SetDefaultHandled();
      }
    }
    else if (event.Type() == EventNames::MouseUp && mouseEvent && mouseEvent->Button() == 0
             && NodeDocument().Frame()
             && NodeDocument().Frame()->GetEventHandler().AutoscrollRenderer() != Renderer())
    {
      // This click or drag event was not over any of the options.
      if (_lastOnChangeSelection.empty())
      {
        return;
      }
      // This makes sure we fire DispatchFormControlChangeEvent for a single click. For drag selection,
      // onChange will fire when the autoscroll timer stops.
      ListBoxOnChange();
    }
    else if (event.Type() == EventNames::KeyDown)
    {
      auto *keyboardEvent = DynamicDowncast<dom::KeyboardEvent>(event);
      if (!keyboardEvent)
      {
        return;
      }

      auto *renderer = Renderer();
      bool isHorizontalWritingMode = renderer ? renderer->GetWritingMode().IsHorizontal() : true;
      bool isBlockFlipped = renderer ? renderer->GetWritingMode().IsBlockFlipped() : false;

      dom::DOMStringView nextKeyIdentifier = isHorizontalWritingMode ? u8"Down" : u8"Right";
      dom::DOMStringView previousKeyIdentifier = isHorizontalWritingMode ? u8"Up" : u8"Left";
      if (isBlockFlipped)
      {
        std::swap(nextKeyIdentifier, previousKeyIdentifier);
      }

      auto keyIdentifier = keyboardEvent->KeyIdentifier();

      bool handled = false;
      int32 endIndex = 0;
      if (_activeSelectionEndIndex < 0)
      {
        // Initialize the end index
        if (keyIdentifier == nextKeyIdentifier || keyIdentifier == u8"PageDown")
        {
          int32 startIndex = LastSelectedListIndex();
          handled = true;
          if (keyIdentifier == nextKeyIdentifier)
          {
            endIndex = NextSelectableListIndex(startIndex);
          }
          else
          {
            endIndex = NextSelectableListIndexPageAway(startIndex, SkipDirection::Forwards);
          }
        }
        else if (keyIdentifier == previousKeyIdentifier || keyIdentifier == u8"PageUp")
        {
          int32 startIndex = OptionToListIndex(SelectedIndex());
          handled = true;
          if (keyIdentifier == previousKeyIdentifier)
          {
            endIndex = PreviousSelectableListIndex(startIndex);
          }
          else
          {
            endIndex = NextSelectableListIndexPageAway(startIndex, SkipDirection::Backwards);
          }
        }
      }
      else
      {
        // Set the end index based on the current end index.
        if (keyIdentifier == nextKeyIdentifier)
        {
          endIndex = NextSelectableListIndex(_activeSelectionEndIndex);
          handled = true;
        }
        else if (keyIdentifier == previousKeyIdentifier)
        {
          endIndex = PreviousSelectableListIndex(_activeSelectionEndIndex);
          handled = true;
        }
        else if (keyIdentifier == u8"PageDown")
        {
          endIndex = NextSelectableListIndexPageAway(_activeSelectionEndIndex, SkipDirection::Forwards);
          handled = true;
        }
        else if (keyIdentifier == u8"PageUp")
        {
          endIndex = NextSelectableListIndexPageAway(_activeSelectionEndIndex, SkipDirection::Backwards);
          handled = true;
        }
      }
      if (keyIdentifier == u8"Home")
      {
        endIndex = FirstSelectableListIndex();
        handled = true;
      }
      else if (keyIdentifier == u8"End")
      {
        endIndex = LastSelectableListIndex();
        handled = true;
      }

      if (NodeDocument().GetSettings().SpatialNavigationEnabled)
      {
        // Check if the selection moves to the boundary.
        if (keyIdentifier == u8"Left" || keyIdentifier == u8"Right"
            || ((keyIdentifier == u8"Down" || keyIdentifier == u8"Up")
                && endIndex == _activeSelectionEndIndex))
        {
          return;
        }
      }

      if (endIndex >= 0 && handled)
      {
        // Save the selection so it can be compared to the new selection when dispatching change events
        // immediately after making the new selection.
        SaveLastSelection();

        krys_debug_assert(listItems.empty() || static_cast<size_t>(endIndex) < listItems.size());
        SetActiveSelectionEndIndex(endIndex);

        _allowsNonContiguousSelection =
          _multiple && (NodeDocument().GetSettings().SpatialNavigationEnabled || keyboardEvent->CtrlKey());
        bool selectNewItem = keyboardEvent->ShiftKey() || !_allowsNonContiguousSelection;

        if (selectNewItem)
        {
          _activeSelectionState = true;
        }
        // If the anchor is uninitialized, or if we're going to deselect all other options, then set the
        // anchor index equal to the end index.
        bool deselectOthers = !_multiple || (!keyboardEvent->ShiftKey() && selectNewItem);
        if (_activeSelectionAnchorIndex < 0 || deselectOthers)
        {
          if (deselectOthers)
          {
            DeselectItemsWithoutValidation();
          }
          SetActiveSelectionAnchorIndex(_activeSelectionEndIndex);
        }

        auto *frame = NodeDocument().Frame();
        (void)Downcast<RenderListBox>(*renderer).ScrollToRevealElementAtListIndex(
          endIndex, frame ? frame->GetEventHandler().KeyboardScrollIsAnimated() : ScrollIsAnimated::No);
        if (selectNewItem)
        {
          UpdateListBoxSelection(deselectOthers);
          ListBoxOnChange();
        }
        else
        {
          ScrollToSelection();
        }

        keyboardEvent->SetDefaultHandled();
      }
    }
    else if (event.Type() == EventNames::KeyPress)
    {
      auto *keyboardEvent = DynamicDowncast<dom::KeyboardEvent>(event);
      if (!keyboardEvent)
      {
        return;
      }

      int32 keyCode = keyboardEvent->KeyCode();
      if (keyCode == '\r')
      {
        if (RefPtr<HTMLFormElement> form = ShareRefPtr(Form()))
        {
          form->SubmitImplicitly(*keyboardEvent, false);
        }
        keyboardEvent->SetDefaultHandled();
      }
      else if (_multiple && keyCode == ' ' && _allowsNonContiguousSelection)
      {
        // Use space to toggle selection change.
        _activeSelectionState = !_activeSelectionState;
        krys_debug_assert(_activeSelectionEndIndex >= 0);
        krys_debug_assert(_activeSelectionEndIndex < static_cast<int32>(listItems.size()));
        krys_debug_assert(Is<HTMLOptionElement>(*listItems[static_cast<size_t>(_activeSelectionEndIndex)]));
        UpdateSelectedState(_activeSelectionEndIndex, true, false);
        ListBoxOnChange();
        keyboardEvent->SetDefaultHandled();
      }
    }
  }

  void HTMLSelectElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    if (!Renderer())
    {
      return;
    }

    if (IsDisabledFormControl())
    {
      HTMLFormControlElement::DefaultEventHandler(event);
      return;
    }

    if (_contentsUseBaseAppearance)
    {
      BaseAppearanceDefaultEventHandler(event);
    }
    else if (UsesMenuList())
    {
      MenuListDefaultEventHandler(event);
    }
    else
    {
      ListBoxDefaultEventHandler(event);
    }
    if (event.DefaultHandled())
    {
      return;
    }

    if (event.Type() == EventNames::KeyPress)
    {
      if (auto *keyboardEvent = DynamicDowncast<dom::KeyboardEvent>(event))
      {
        if (!keyboardEvent->CtrlKey() && !keyboardEvent->AltKey() && !keyboardEvent->MetaKey()
            && u_isprint(static_cast<UChar32>(keyboardEvent->CharCode())))
        {
          TypeAheadFind(*keyboardEvent);
          event.SetDefaultHandled();
          return;
        }
      }
    }
    HTMLFormControlElement::DefaultEventHandler(event);
  }

  int32 HTMLSelectElement::LastSelectedListIndex() const noexcept
  {
    auto &items = ListItems();
    for (size_t i = items.size(); i;)
    {
      auto *element = DynamicDowncast<HTMLOptionElement>(items[--i].get());
      if (element && element->Selected())
      {
        return static_cast<int32>(i);
      }
    }
    return -1;
  }

#pragma region TypeAheadDataSource

  int32 HTMLSelectElement::IndexOfSelectedOption() const noexcept
  {
    return OptionToListIndex(SelectedIndex());
  }

  int32 HTMLSelectElement::OptionCount() const noexcept
  {
    return static_cast<int32>(ListItems().size());
  }

  dom::DOMString HTMLSelectElement::OptionAtIndex(int32 index) const noexcept
  {
    auto *option = DynamicDowncast<HTMLOptionElement>(ListItems()[static_cast<size_t>(index)].get());
    if (!option || option->IsDisabledFormControl())
    {
      return {};
    }
    return option->TextIndentedToRespectGroupLabel();
  }

#pragma endregion

  void HTMLSelectElement::TypeAheadFind(dom::KeyboardEvent &event) noexcept
  {
    int32 index = _typeAhead.HandleEvent(
      event, {TypeAhead::MatchMode::MatchPrefix, TypeAhead::MatchMode::CycleFirstChar});
    if (index < 0)
    {
      return;
    }
    SelectOption(ListToOptionIndex(index),
                 {SelectOptionFlag::DeselectOtherOptions, SelectOptionFlag::DispatchChangeEvent,
                  SelectOptionFlag::UserDriven});
    if (!UsesMenuList())
    {
      ListBoxOnChange();
    }
  }

  void HTMLSelectElement::AccessKeySetSelectedIndex(int32 index) noexcept
  {
    // First bring into focus the list box.
    if (!Focused())
    {
      (void)AccessKeyAction(false);
    }

    // If this index is already selected, unselect. otherwise update the selected index.
    auto &items = ListItems();
    int32 listIndex = OptionToListIndex(index);
    if (listIndex >= 0)
    {
      if (auto *option = DynamicDowncast<HTMLOptionElement>(items[static_cast<size_t>(listIndex)].get()))
      {
        if (option->Selected())
        {
          option->SetSelectedState(false);
        }
        else
        {
          SelectOption(index, {SelectOptionFlag::DispatchChangeEvent, SelectOptionFlag::UserDriven});
        }
      }
    }

    if (UsesMenuList())
    {
      DispatchChangeEventForMenuList();
    }
    else
    {
      ListBoxOnChange();
    }

    ScrollToSelection();
  }

  uint32 HTMLSelectElement::Length() const noexcept
  {
    uint32 options = 0;
    for (auto &item : ListItems())
    {
      if (Is<HTMLOptionElement>(*item))
      {
        ++options;
      }
    }
    return options;
  }

  dom::ExceptionOr<void> HTMLSelectElement::ShowPicker() noexcept
  {
    if (!NodeDocument().Frame())
    {
      return {};
    }

    if (!IsMutable())
    {
      return dom::Exception(dom::ExceptionCode::InvalidStateError,
                            u8"Select showPicker() cannot be used on immutable controls.");
    }

    auto *window = NodeDocument().Window();
    if (!window || !window->ConsumeTransientActivation())
    {
      return dom::Exception(dom::ExceptionCode::NotAllowedError,
                            u8"Select showPicker() requires a user gesture.");
    }

    if (_contentsUseBaseAppearance)
    {
      OpenPicker(true);
    }
    else
    {
      ShowPopup();
    }
    return {};
  }

#pragma region HTMLSelectElement

  // https://html.spec.whatwg.org/#dom-fe-autocomplete
  dom::DOMString HTMLSelectElement::Autocomplete() const noexcept
  {
    // Return the element's IDL-exposed autofill value.
    return AutofillData::Create(*this).IDLExposedValue;
  }

  void HTMLSelectElement::Autocomplete(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"autocomplete", krys::move(value));
  }

  bool HTMLSelectElement::Disabled() const noexcept
  {
    return Reflect<bool>(*this, u8"disabled");
  }

  void HTMLSelectElement::Disabled(bool value) noexcept
  {
    Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  void HTMLSelectElement::Multiple(bool value) noexcept
  {
    Reflect<bool>(*this, u8"multiple", krys::move(value));
  }

  dom::DOMString HTMLSelectElement::Name() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLSelectElement::Name(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  bool HTMLSelectElement::Required() const noexcept
  {
    return Reflect<bool>(*this, u8"required");
  }

  void HTMLSelectElement::Required(bool value) noexcept
  {
    Reflect<bool>(*this, u8"required", krys::move(value));
  }

  void HTMLSelectElement::Size(uint32 value) noexcept
  {
    Reflect<uint32>(*this, u8"size", krys::move(value));
  }

#pragma endregion
}
