#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/TypeAhead.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class HTMLCollection;
  class KeyboardEvent;
}

namespace krys::boo::html
{
  class HTMLOptGroupElement;
  class HTMLOptionsCollection;

  /// @see https://html.spec.whatwg.org/#htmlselectelement
  class HTMLSelectElement : public HTMLFormControlElement, private TypeAheadDataSource
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSelectElement);

  public:
    using OptionOrOptGroupElement = Variant<RefPtr<HTMLOptionElement>, RefPtr<HTMLOptGroupElement>>;
    using HTMLElementOrInt = Variant<RefPtr<HTMLElement>, int32>;

    enum class ExcludeOptGroup : bool
    {
      No,
      Yes
    };

  private:
    enum class SelectOptionFlag : uint8
    {
      DeselectOtherOptions,
      DispatchChangeEvent,
      UserDriven,
    };
    using SelectOptionFlags = EnumSet<SelectOptionFlag>;

    enum class SkipDirection : int8
    {
      Backwards = -1,
      Forwards = 1
    };

    /// @brief The options, optgroups and hrs of the select, in order.
    mutable List<WeakPtr<HTMLElement>> _listItems;
    List<bool> _lastOnChangeSelection;
    List<bool> _cachedStateForActiveSelection;
    TypeAhead _typeAhead;
    uint32 _size {0};
    int32 _lastOnChangeIndex {-1};
    int32 _activeSelectionAnchorIndex {-1};
    int32 _activeSelectionEndIndex {-1};
    bool _isProcessingUserDrivenChange {false};
    bool _multiple {false};
    bool _activeSelectionState {false};
    bool _allowsNonContiguousSelection {false};
    mutable bool _shouldRecalcListItems {false};

  public:
    explicit HTMLSelectElement(dom::Document &document) noexcept;

    /// @brief The select whose list of options a child of @p startNode would be in, if any.
    KRYS_NODISCARD static HTMLSelectElement *FindOwnerSelect(dom::ContainerNode *startNode,
                                                             ExcludeOptGroup excludeOptGroup) noexcept;

    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept override
    {
      return false;
    }

    KRYS_NODISCARD bool CanStartSelection() const noexcept final
    {
      return false;
    }

#pragma region HTMLSelectElement - https://html.spec.whatwg.org/#htmlselectelement

    /// @see https://html.spec.whatwg.org/#dom-select-autocomplete
    KRYS_NODISCARD dom::DOMString Autocomplete() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-autocomplete
    void Autocomplete(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-disabled
    void Disabled(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-multiple
    KRYS_NODISCARD bool Multiple() const noexcept
    {
      return _multiple;
    }

    /// @see https://html.spec.whatwg.org/#dom-select-multiple
    void Multiple(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept override;

    /// @see https://html.spec.whatwg.org/#dom-select-name
    void Name(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-required
    KRYS_NODISCARD bool Required() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-required
    void Required(bool value) noexcept;

    /// @brief The parsed `size` attribute, or 0 if it's missing or invalid.
    /// @see https://html.spec.whatwg.org/#dom-select-size
    KRYS_NODISCARD uint32 Size() const noexcept
    {
      return _size;
    }

    /// @see https://html.spec.whatwg.org/#dom-select-size
    void Size(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept
    {
      return FormControlType();
    }

    /// @see https://html.spec.whatwg.org/#dom-select-options
    KRYS_NODISCARD Ref<HTMLOptionsCollection> Options() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-length
    KRYS_NODISCARD uint32 Length() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-length
    dom::ExceptionOr<void> Length(uint32 newLength) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-item
    KRYS_NODISCARD HTMLOptionElement *Item(uint32 index) const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-nameditem
    KRYS_NODISCARD HTMLOptionElement *NamedItem(dom::DOMStringView name) const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-add
    dom::ExceptionOr<void> Add(const OptionOrOptGroupElement &element,
                               const Maybe<HTMLElementOrInt> &before = {}) noexcept;

    using Element::Remove;

    /// @see https://html.spec.whatwg.org/#dom-select-remove
    void Remove(int32 optionIndex) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-setter
    dom::ExceptionOr<void> SetItem(uint32 index, HTMLOptionElement *option) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-selectedoptions
    KRYS_NODISCARD Ref<dom::HTMLCollection> SelectedOptions() noexcept;

    /// @brief The index of the first selected option in the list of options, or -1 if none is.
    /// @see https://html.spec.whatwg.org/#dom-select-selectedindex
    KRYS_NODISCARD int32 SelectedIndex() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-selectedindex
    void SelectedIndex(int32 index) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-value
    KRYS_NODISCARD dom::DOMString Value() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-value
    void Value(dom::DOMStringView value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-showpicker
    dom::ExceptionOr<void> ShowPicker() noexcept;

#pragma endregion

    /// @brief Whether the select is drawn as a drop-down (a menu list) rather than a list box.
    KRYS_NODISCARD bool UsesMenuList() const noexcept;

    /// @brief The options, optgroups and hrs of the select, in order.
    KRYS_NODISCARD const List<WeakPtr<HTMLElement>> &ListItems() const noexcept;

    /// @brief Selects the option at @p optionIndex as the user would.
    void OptionSelectedByUser(int32 optionIndex, bool dispatchChangeEvent,
                              bool allowMultipleSelection = false) noexcept;

    void OptionElementChildrenChanged() noexcept;
    void SetRecalcListItems() noexcept;
    void InvalidateSelectedItems() noexcept;
    void UpdateListItemSelectedStates(
      AllowStyleInvalidation allowStyleInvalidation = AllowStyleInvalidation::Yes) noexcept;

    /// @brief Selects an option inserted into the list of options if it's selected.
    void OptionInserted(HTMLOptionElement &option) noexcept;

    /// @brief Selects the last selected option of an optgroup inserted into the select.
    void OptGroupInserted(HTMLOptGroupElement &optGroup) noexcept;

    void AccessKeySetSelectedIndex(int32 index) noexcept;
    void ScrollToSelection() noexcept;

    void ListBoxSelectItem(int32 listIndex, bool allowMultiplySelections, bool shift,
                           bool fireOnChangeNow = true) noexcept;

    KRYS_NODISCARD bool CanSelectAll() const noexcept;
    void SelectAll() noexcept;
    KRYS_NODISCARD int32 ListToOptionIndex(int32 listIndex) const noexcept;
    void ListBoxOnChange() noexcept;
    KRYS_NODISCARD int32 OptionToListIndex(int32 optionIndex) const noexcept;
    KRYS_NODISCARD int32 ActiveSelectionStartListIndex() const noexcept;
    KRYS_NODISCARD int32 ActiveSelectionEndListIndex() const noexcept;
    void SetActiveSelectionAnchorIndex(int32 index) noexcept;
    void SetActiveSelectionEndIndex(int32 index) noexcept;
    void UpdateListBoxSelection(bool deselectOtherOptions) noexcept;

    /// @brief Called by an option when its selectedness is set.
    void OptionSelectionStateChanged(HTMLOptionElement &option, bool optionIsSelected) noexcept;

    KRYS_NODISCARD bool AllowsNonContiguousSelection() const noexcept
    {
      return _allowsNonContiguousSelection;
    }

    KRYS_NODISCARD dom::DOMString FormControlType() const noexcept final;

    KRYS_NODISCARD bool IsEnumeratable() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsLabelable() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsInteractiveContent() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsRequiredFormControl() const noexcept final
    {
      return IsRequired();
    }

    KRYS_NODISCARD bool IsOptionalFormControl() const noexcept final
    {
      return !IsRequiredFormControl();
    }

    KRYS_NODISCARD bool ValueMissing() const noexcept final;

    bool AppendFormData(DOMFormData &formData) noexcept final;

    /// @see https://html.spec.whatwg.org/#the-select-element:concept-form-reset-control
    void Reset() noexcept final;

  protected:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept final;
    KRYS_NODISCARD bool ChildShouldCreateRenderer(const dom::Node &child) const noexcept final;
    void DidRecalcStyle(EnumSet<style::Change> styleChange) noexcept final;
    void DefaultEventHandler(dom::Event &event) noexcept final;
    void DispatchFocusEvent(dom::EventTarget *relatedTarget, const dom::FocusOptions &options) noexcept final;
    void DispatchBlurEvent(dom::EventTarget *relatedTarget) noexcept final;
    void OnChildrenChanged() noexcept final;
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value, dom::DOMStringAtom namespaceURI) noexcept final;

  private:
    /// @see https://html.spec.whatwg.org/#placeholder-label-option
    KRYS_NODISCARD bool HasPlaceholderLabelOption() const noexcept;

    void DispatchChangeEventForMenuList() noexcept;
    void RecalcListItems(bool updateSelectedStates = true, AllowStyleInvalidation allowStyleInvalidation =
                                                             AllowStyleInvalidation::Yes) const noexcept;
    void TypeAheadFind(dom::KeyboardEvent &event) noexcept;
    void SaveLastSelection() noexcept;
    void SelectOption(int32 optionIndex, SelectOptionFlags flags = {}) noexcept;
    void DeselectItemsWithoutValidation(HTMLElement *elementToExclude = nullptr) noexcept;
    void ParseMultipleAttribute(bool present) noexcept;
    KRYS_NODISCARD int32 LastSelectedListIndex() const noexcept;
    void UpdateSelectedState(int32 listIndex, bool multi, bool shift) noexcept;
    void MenuListDefaultEventHandler(dom::Event &event) noexcept;
    bool PlatformHandleKeydownEvent(dom::KeyboardEvent &event) noexcept;
    void ListBoxDefaultEventHandler(dom::Event &event) noexcept;
    void SetOptionsChangedOnRenderer() noexcept;

    /// @brief Shows the menu list's popup, if the select is drawn as a menu list.
    void ShowPopup() noexcept;

    KRYS_NODISCARD int32 NextValidIndex(int32 listIndex, SkipDirection direction, int32 skip) const noexcept;
    KRYS_NODISCARD int32 NextSelectableListIndex(int32 startIndex) const noexcept;
    KRYS_NODISCARD int32 PreviousSelectableListIndex(int32 startIndex) const noexcept;
    KRYS_NODISCARD int32 FirstSelectableListIndex() const noexcept;
    KRYS_NODISCARD int32 LastSelectableListIndex() const noexcept;
    KRYS_NODISCARD int32 NextSelectableListIndexPageAway(int32 startIndex,
                                                         SkipDirection direction) const noexcept;

#pragma region TypeAheadDataSource

    KRYS_NODISCARD int32 IndexOfSelectedOption() const noexcept final;
    KRYS_NODISCARD int32 OptionCount() const noexcept final;
    KRYS_NODISCARD dom::DOMString OptionAtIndex(int32 index) const noexcept final;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLSelectElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return Is<krys::boo::html::HTMLElement>(target)
           && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLSelectElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLSelectElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
