#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Internals/HTMLCollection/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLDataListElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLHRElement.hpp"
#include "Krystal.Booey/HTML/HTMLOptGroupElement.hpp"
#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/HTMLOptionsCollection.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderListBox.hpp"
#include "Krystal.Booey/Rendering/RenderMenuList.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include <climits>
#include <unicode/uchar.h>

namespace krys::boo::html
{
  namespace
  {
    // Upper limit agreed upon with representatives of Opera and Mozilla.
    constexpr uint32 MaxSelectItems = 10'000u;

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

  FormControlState HTMLSelectElement::SaveFormControlState() const noexcept
  {
    FormControlState state;
    auto &items = ListItems();
    state.reserve(items.size());
    for (auto &element : items)
    {
      auto *option = DynamicDowncast<HTMLOptionElement>(element.get());
      if (!option || !option->Selected())
      {
        continue;
      }
      state.push_back(option->Value());
      if (!Multiple())
      {
        break;
      }
    }
    return state;
  }

  Maybe<size_t> HTMLSelectElement::SearchOptionsForValue(dom::DOMStringView value, size_t listIndexStart,
                                                         size_t listIndexEnd) const noexcept
  {
    auto &items = ListItems();
    auto loopEndIndex = std::min(items.size(), listIndexEnd);
    for (auto i = listIndexStart; i < loopEndIndex; ++i)
    {
      auto *option = DynamicDowncast<HTMLOptionElement>(items[i].get());
      if (!option)
      {
        continue;
      }
      if (option->Value() == value)
      {
        return i;
      }
    }
    return null;
  }

  void HTMLSelectElement::RestoreFormControlState(const FormControlState &state) noexcept
  {
    RecalcListItems();

    auto &items = ListItems();
    auto itemsSize = items.size();
    if (!itemsSize)
    {
      return;
    }

    for (auto &element : items)
    {
      if (RefPtr<HTMLOptionElement> option = ShareRefPtr(DynamicDowncast<HTMLOptionElement>(element.get())))
      {
        option->SetSelectedState(false);
      }
    }

    if (!Multiple())
    {
      if (auto foundIndex = SearchOptionsForValue(state[0], 0, itemsSize))
      {
        Downcast<HTMLOptionElement>(*items[*foundIndex]).SetSelectedState(true);
      }
    }
    else
    {
      size_t startIndex = 0;
      for (auto &value : state)
      {
        auto foundIndex = SearchOptionsForValue(value, startIndex, itemsSize);
        if (!foundIndex)
        {
          foundIndex = SearchOptionsForValue(value, 0, startIndex);
        }
        if (!foundIndex)
        {
          continue;
        }
        Downcast<HTMLOptionElement>(*items[*foundIndex]).SetSelectedState(true);
        startIndex = *foundIndex + 1;
      }
    }

    InvalidateSelectedItems();
    SetOptionsChangedOnRenderer();
    UpdateValidity();
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

    int32 listIndex = OptionToListIndex(0);
    if (listIndex)
    {
      return false;
    }
    auto &option = Downcast<HTMLOptionElement>(*ListItems()[static_cast<size_t>(listIndex)]);
    return option.Value().empty();
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
    CEReactionsScope ceReactions;
    RefPtr<HTMLElement> beforeElement;
    Ref<dom::ContainerNode> parent = ShareRef<dom::ContainerNode>(*this);
    if (before)
    {
      beforeElement = std::visit(
        [this]<typename T>(const T &value) -> RefPtr<HTMLElement>
        {
          if constexpr (SameType<T, int32>)
          {
            return ShareRefPtr<HTMLElement>(Item(static_cast<uint32>(value)));
          }
          else
          {
            return value;
          }
        },
        *before);
      if (std::holds_alternative<int32>(*before) && beforeElement && beforeElement->ParentNode())
      {
        parent = ShareRef(*beforeElement->ParentNode());
      }
    }

    Ref<HTMLElement> toInsert = std::visit([](const auto &htmlElement) -> Ref<HTMLElement>
                                           { return ShareRef<HTMLElement>(*htmlElement); }, element);

    auto result = parent->InsertBefore(*toInsert, beforeElement.get());
    if (result.HasException())
    {
      return result.ReleaseException();
    }
    return {};
  }

  void HTMLSelectElement::Remove(int32 optionIndex) noexcept
  {
    CEReactionsScope ceReactions;
    int32 listIndex = OptionToListIndex(optionIndex);
    if (listIndex < 0)
    {
      return;
    }

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

  void HTMLSelectElement::Value(dom::DOMStringView value) noexcept
  {
    // Find the option with Value() matching the given parameter and make it the current selection.
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

  void HTMLSelectElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                             Maybe<dom::DOMStringView> value,
                                             dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLFormControlElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
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
        SetRecalcListItems();
        UpdateValidity();
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
    // FIXME: Validation message shadow trees, once there are validation bubbles.
    if (!UsesMenuList())
    {
      return Is<HTMLOptionElement>(child) || Is<HTMLOptGroupElement>(child);
    }
    return false;
  }

  Ref<dom::HTMLCollection> HTMLSelectElement::SelectedOptions() noexcept
  {
    return CreateRef<dom::LiveHTMLCollection>(*this,
                                              [this](const dom::Element &element)
                                              {
                                                auto *option = DynamicDowncast<HTMLOptionElement>(element);
                                                return option && option->OwnerSelectElement() == this
                                                       && option->Selected();
                                              });
  }

  Ref<HTMLOptionsCollection> HTMLSelectElement::Options() noexcept
  {
    return CreateRef<HTMLOptionsCollection>(*this);
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
    CEReactionsScope ceReactions;
    if (!option)
    {
      Remove(static_cast<int32>(index));
      return {};
    }

    // If we are adding options, we should check 'index > MaxSelectItems' first to avoid integer overflow.
    if (index > Length() && index >= MaxSelectItems)
    {
      return {};
    }

    int64 diff = static_cast<int64>(index) - static_cast<int64>(Length());

    RefPtr<HTMLOptionElement> before;
    // Out of array bounds? First insert empty dummies.
    if (diff > 0)
    {
      auto result = Length(index);
      if (result.HasException())
      {
        return result;
      }
    }
    // Replace an existing entry?
    else if (diff < 0)
    {
      before = ShareRefPtr(Item(index + 1u));
      Remove(static_cast<int32>(index));
    }

    // Finally add the new element.
    auto result = Add(ShareRefPtr(option), HTMLElementOrInt {RefPtr<HTMLElement>(before)});
    if (result.HasException())
    {
      return result;
    }

    if (diff >= 0 && option->Selected())
    {
      OptionSelectionStateChanged(*option, true);
    }

    return {};
  }

  dom::ExceptionOr<void> HTMLSelectElement::Length(uint32 newLength) noexcept
  {
    CEReactionsScope ceReactions;
    // If we are adding options, we should check 'index > MaxSelectItems' first to avoid integer overflow.
    if (newLength > Length() && newLength > MaxSelectItems)
    {
      return {};
    }

    int64 diff = static_cast<int64>(Length()) - static_cast<int64>(newLength);

    // Add dummy elements.
    if (diff < 0)
    {
      do
      {
        auto result = Add(RefPtr<HTMLOptionElement>(CreateRef<HTMLOptionElement>(NodeDocument())));
        if (result.HasException())
        {
          return result;
        }
      } while (++diff);
    }
    else
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
    // Collections don't cache their items, so there is nothing to invalidate.
  }

  void HTMLSelectElement::SetRecalcListItems() noexcept
  {
    _shouldRecalcListItems = true;
    // Manual selection anchor is reset when manipulating the select programmatically.
    _activeSelectionAnchorIndex = -1;
    SetOptionsChangedOnRenderer();
    InvalidateStyleForSubtree();
    if (!IsConnected())
    {
      InvalidateSelectedItems();
    }

    // FIXME: Tell the chrome client the focused select's options changed
    // (focusedSelectElementDidChangeOptions), for its popup.
  }

  void HTMLSelectElement::RecalcListItems(bool updateSelectedStates,
                                          AllowStyleInvalidation allowStyleInvalidation) const noexcept
  {
    _listItems.clear();

    _shouldRecalcListItems = false;

    RefPtr<HTMLOptionElement> foundSelected;
    RefPtr<HTMLOptionElement> firstOption;
    auto handleOptionElement = [&](HTMLOptionElement &option)
    {
      _listItems.push_back(WeakPtr<HTMLElement>::WithRef(option));
      if (updateSelectedStates && !_multiple)
      {
        if (!firstOption)
        {
          firstOption = ShareRef(option);
        }
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
        _listItems.push_back(WeakPtr<HTMLElement>::WithRef(*descendant));
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
        _listItems.push_back(WeakPtr<HTMLElement>::WithRef(*descendant));
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

    if (!foundSelected && _size <= 1 && firstOption && !firstOption->SelectedWithoutUpdate())
    {
      firstOption->SetSelectedState(true, allowStyleInvalidation);
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

  void HTMLSelectElement::SelectedIndex(int32 index) noexcept
  {
    SelectOption(index, {SelectOptionFlag::DeselectOtherOptions});
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
    if (oldMultiple != _multiple)
    {
      if (oldSelectedIndex >= 0)
      {
        SelectedIndex(oldSelectedIndex);
      }
      else
      {
        Reset();
      }
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

  void HTMLSelectElement::Reset() noexcept
  {
    RefPtr<HTMLOptionElement> firstOption;
    RefPtr<HTMLOptionElement> selectedOption;

    for (auto &element : ListItems())
    {
      auto *option = DynamicDowncast<HTMLOptionElement>(element.get());
      if (!option)
      {
        continue;
      }

      option->ResetDirtiness();
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

        (void)Downcast<RenderListBox>(*renderer).ScrollToRevealElementAtListIndex(endIndex);
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

    if (UsesMenuList())
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

    ShowPopup();
    return {};
  }

#pragma region HTMLSelectElement

  dom::DOMString HTMLSelectElement::Autocomplete() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"autocomplete").value_or(u8"");
  }

  void HTMLSelectElement::Autocomplete(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"autocomplete", krys::move(value));
  }

  bool HTMLSelectElement::Disabled() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLSelectElement::Disabled(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  void HTMLSelectElement::Multiple(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"multiple", krys::move(value));
  }

  dom::DOMString HTMLSelectElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLSelectElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  bool HTMLSelectElement::Required() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"required");
  }

  void HTMLSelectElement::Required(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"required", krys::move(value));
  }

  void HTMLSelectElement::Size(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"size", krys::move(value));
  }

#pragma endregion
}
