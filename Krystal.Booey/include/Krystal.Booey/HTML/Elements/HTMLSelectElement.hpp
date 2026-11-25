#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Forms/TypeAhead.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::html
{
  class HTMLOptionsCollection;
}

namespace krys::boo
{
  class HTMLCollection;
  class HTMLOptGroupElement;
  class KeyboardEvent;

  /// @see https://html.spec.whatwg.org/multipage/form-elements.html#htmlselectelement
  class HTMLSelectElement : public HTMLFormControlElement, private html::TypeAheadDataSource
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSelectElement);

  public:
    using OptionOrOptGroupElement = Variant<RefPtr<HTMLOptionElement>, RefPtr<HTMLOptGroupElement>>;
    using HTMLElementOrInt = Variant<RefPtr<HTMLElement>, int32>;

    enum class ExcludeOptGroup : bool
    {
      No,
      Yes,
    };

  private:
    WeakPtr<HTMLSlotElement> _selectButtonSlot;
    WeakPtr<boo::Element> _selectFallbackButtonText;
    WeakPtr<HTMLElement> _selectPopover;
    WeakPtr<HTMLSlotElement> _selectPopoverSlot;
    WeakPtr<HTMLSlotElement> _selectListBoxSlot;
    WeakPtr<boo::Element> _selectButton;
    bool _slotsAreForMenuList {true};
    bool _contentsUseBaseAppearance {false};

    enum class SelectOptionFlag : uint8
    {
      DeselectOtherOptions,
      DispatchChangeEvent,
      MakeOptionDirty,
      UserDriven,
    };

    using SelectOptionFlags = EnumSet<SelectOptionFlag>;

    enum class SkipDirection : int8
    {
      Backwards = -1,
      Forwards = 1,
    };

    /// @brief The options, optgroups and hrs of the select, in order.
    mutable List<WeakPtr<HTMLElement>> _listItems;
    // [SameObject] collections, held weakly as they hold the select.
    WeakPtr<HTMLCollection> _options;
    WeakPtr<HTMLCollection> _selectedOptions;
    List<bool> _lastOnChangeSelection;
    List<bool> _cachedStateForActiveSelection;
    html::TypeAhead _typeAhead;
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
    explicit HTMLSelectElement(Document &document) noexcept;

    KRYS_NODISCARD static Ref<HTMLSelectElement> Create(Document &document) noexcept;

    /// @brief The select whose list of options a child of @p startNode would be in, if any.
    KRYS_NODISCARD static HTMLSelectElement *FindOwnerSelect(ContainerNode *startNode,
                                                             ExcludeOptGroup excludeOptGroup) noexcept;

    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept override
    {
      return false;
    }

    KRYS_NODISCARD bool CanStartSelection() const noexcept final
    {
      return false;
    }

#pragma region HTMLSelectElement - https://html.spec.whatwg.org/multipage/form-elements.html#htmlselectelement

    /// @see https://html.spec.whatwg.org/#dom-select-autocomplete
    KRYS_NODISCARD DOMString Autocomplete() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-autocomplete
    void Autocomplete(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-disabled
    void Disabled(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-multiple
    KRYS_NODISCARD bool Multiple() const noexcept
    {
      return _multiple;
    }

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-multiple
    void Multiple(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-name
    KRYS_NODISCARD DOMString Name() const noexcept override;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-required
    KRYS_NODISCARD bool Required() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-required
    void Required(bool value) noexcept;

    /// @brief The parsed `size` attribute, or 0 if it's missing or invalid.
    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-size
    KRYS_NODISCARD uint32 Size() const noexcept
    {
      return _size;
    }

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-size
    void Size(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-type
    KRYS_NODISCARD DOMString Type() const noexcept
    {
      return FormControlType();
    }

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-options
    KRYS_NODISCARD Ref<html::HTMLOptionsCollection> Options() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-length
    KRYS_NODISCARD uint32 Length() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-length
    ExceptionOr<void> Length(uint32 newLength) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-item
    KRYS_NODISCARD HTMLOptionElement *Item(uint32 index) const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-nameditem
    KRYS_NODISCARD HTMLOptionElement *NamedItem(DOMStringView name) const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-add
    ExceptionOr<void> Add(const OptionOrOptGroupElement &element,
                          const Maybe<HTMLElementOrInt> &before = {}) noexcept;

    using Element::Remove;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-remove
    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#htmloptionscollection-remove-an-option
    void Remove(int32 optionIndex) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-setter
    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#append-new-option-elements
    ExceptionOr<void> SetItem(uint32 index, HTMLOptionElement *option) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-selectedoptions
    KRYS_NODISCARD Ref<HTMLCollection> SelectedOptions() noexcept;

    /// @brief The index of the first selected option in the list of options, or -1 if none is.
    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-selectedindex
    KRYS_NODISCARD int32 SelectedIndex() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-selectedindex
    void SelectedIndex(int32 index) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-value
    KRYS_NODISCARD DOMString Value() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-select-value
    void Value(DOMStringView value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-select-showpicker
    ExceptionOr<void> ShowPicker() noexcept;

#pragma endregion

    /// @brief Whether the select is drawn as a drop-down (a menu list) rather than a list box.
    KRYS_NODISCARD bool UsesMenuList() const noexcept;

    /// @brief The options, optgroups and hrs of the select, in order.
    KRYS_NODISCARD const List<WeakPtr<HTMLElement>> &ListItems() const noexcept;

    /// @brief Selects the option at @p optionIndex as the user would.
    void OptionSelectedByUser(int32 optionIndex, bool dispatchChangeEvent,
                              bool allowMultipleSelection = false) noexcept;

    /// @brief Updates the select as an option's text changed: the renderer, its validity and the button's
    /// text.
    void OptionElementChildrenChanged() noexcept;

    /// @brief The slot of the shadow tree that takes @p child: a first child element that's a button takes
    /// the select button slot and the other children the select popover's; all of a list box's take its own
    /// slot.
    /// @see https://html.spec.whatwg.org/multipage/rendering.html#the-select-element-2
    KRYS_NODISCARD HTMLSlotElement *SlotForHostChild(const Node &child) const noexcept;

    /// @brief Whether the select renders as a drop-down box with base appearance (`appearance: base-select`):
    /// its shadow tree instead of a menu list, and a popover for its picker.
    KRYS_NODISCARD bool ContentsUseBaseAppearance() const noexcept
    {
      return _contentsUseBaseAppearance;
    }

    /// @return Whether it changed.
    bool SetContentsUseBaseAppearance(bool value) noexcept;

    /// @brief Whether the picker of a select with base appearance is showing.
    KRYS_NODISCARD bool IsPickerOpen() const noexcept;

    /// @brief The select popover, its picker when it has base appearance.
    KRYS_NODISCARD HTMLElement *SelectPopover() const noexcept
    {
      return _selectPopover.get();
    }

    /// @brief The user picked @p option from the picker of a select with base appearance.
    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#concept-select-pick
    void PickOption(HTMLOptionElement &option) noexcept;

    /// @brief Sets the text the select shows without a button of its own to its selected option's label.
    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#clone-selected-option-into-select-button
    void CloneSelectedOptionIntoSelectButton() noexcept;

    /// @brief Has the list items recomputed, as options, optgroups or hrs were added, removed or moved.
    void SetRecalcListItems() noexcept;

    /// @brief Runs the selectedness setting algorithm, then updates the selectedcontent elements if it or
    /// @p hasChanged changed the selection.
    /// @see https://html.spec.whatwg.org/#reset-a-select's-selectedness
    void ResetSelectedness(bool hasChanged) noexcept;

    /// @brief Clears the cached `selectedOptions` collection.
    void InvalidateSelectedItems() noexcept;

    /// @brief Recomputes the list items, and the options' selectedness, if they're out of date.
    void UpdateListItemSelectedStates(
      html::AllowStyleInvalidation allowStyleInvalidation = html::AllowStyleInvalidation::Yes) noexcept;

    /// @brief Selects an option inserted into the list of options if it's selected.
    void OptionInserted(HTMLOptionElement &option) noexcept;

    /// @brief Selects the last selected option of an optgroup inserted into the select.
    void OptGroupInserted(HTMLOptGroupElement &optGroup) noexcept;

    /// @brief Focuses the select and toggles the option at @p index, as an access key does.
    void AccessKeySetSelectedIndex(int32 index) noexcept;

    /// @brief Scrolls a list box to its selection.
    void ScrollToSelection() noexcept;

    /// @brief Whether Select All applies (a list box), and selects all its options.
    KRYS_NODISCARD bool CanSelectAll() const noexcept;

    void SelectAll() noexcept;

    /// @brief Converts between an index in the list items and one in the options; -1 if there's none.
    KRYS_NODISCARD int32 ListToOptionIndex(int32 listIndex) const noexcept;

    /// @brief Fires change if the list box's selection differs from the one saved when the user began.
    void ListBoxOnChange() noexcept;

    KRYS_NODISCARD int32 OptionToListIndex(int32 optionIndex) const noexcept;

    /// @brief The start and end of the range the user is selecting in a list box (from the anchor to the
    /// end), and their setters.
    KRYS_NODISCARD int32 ActiveSelectionStartListIndex() const noexcept;

    KRYS_NODISCARD int32 ActiveSelectionEndListIndex() const noexcept;

    void SetActiveSelectionAnchorIndex(int32 index) noexcept;

    void SetActiveSelectionEndIndex(int32 index) noexcept;

    /// @brief Selects the options in the user's range, restoring the others to their saved state or (with
    /// @p deselectOtherOptions) deselecting them.
    void UpdateListBoxSelection(bool deselectOtherOptions) noexcept;

    /// @brief Called by an option when its selectedness is set.
    void OptionSelectionStateChanged(HTMLOptionElement &option, bool optionIsSelected) noexcept;

    /// @brief Whether a list box's selection may have gaps (Ctrl-click and space toggle options).
    KRYS_NODISCARD bool AllowsNonContiguousSelection() const noexcept
    {
      return _allowsNonContiguousSelection;
    }

    KRYS_NODISCARD DOMString FormControlType() const noexcept final;

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

    bool AppendFormData(html::DOMFormData &formData) noexcept final;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#the-select-element:concept-form-reset-control
    void Reset() noexcept final;

  protected:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept final;

    KRYS_NODISCARD bool ChildShouldCreateRenderer(const Node &child) const noexcept final;

    void DidRecalcStyle(EnumSet<style::Change> styleChange) noexcept final;

    void DefaultEventHandler(Event &event) noexcept final;

    void DispatchFocusEvent(EventTarget *relatedTarget, const dom::FocusOptions &options) noexcept final;

    void DispatchBlurEvent(EventTarget *relatedTarget) noexcept final;

    void OnChildrenChanged() noexcept final;

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept final;

    void DisabledStateChanged() noexcept final;

    void WillChangeDisabledState(
      bool newDisabled, List<UniquePtr<style::PseudoClassChangeInvalidation>> &invalidations) noexcept final;

    void DidAddUserAgentShadowRoot(boo::ShadowRoot &root) noexcept final;

  private:
    /// @brief The default event handling of a select with base appearance: opening its picker, and moving
    /// the focus between the options.
    void BaseAppearanceDefaultEventHandler(Event &event) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#select-open-picker
    void OpenPicker(bool byKeyboard) noexcept;

    /// @brief Hides the picker of a select with base appearance, refocusing the select if an option had the
    /// focus.
    void ClosePicker() noexcept;

    /// @brief Reassigns the children to slots as the select button or the drawing mode changes.
    void ReassignSlots() noexcept;

    /// @brief Moves the focus between the selectable options for an arrow, Home, End or page key.
    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#focus-another-option
    void FocusOptionForKey(DOMStringView key) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#append-new-option-elements
    ExceptionOr<void> AppendNewOptionElements(uint32 count) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#placeholder-label-option
    KRYS_NODISCARD bool HasPlaceholderLabelOption() const noexcept;

    /// @brief The first option in the list of options whose selectedness is true, if any.
    KRYS_NODISCARD HTMLOptionElement *FirstSelectedOption() const noexcept;

    /// @brief Fires input and change if the user changed a menu list's selection.
    void DispatchChangeEventForMenuList() noexcept;

    /// @brief Recomputes the list items, running the selectedness setting algorithm with them if
    /// @p updateSelectedStates.
    void RecalcListItems(
      bool updateSelectedStates = true,
      html::AllowStyleInvalidation allowStyleInvalidation = html::AllowStyleInvalidation::Yes) const noexcept;

    /// @brief Selects the option the user is typing the start of.
    void TypeAheadFind(KeyboardEvent &event) noexcept;

    /// @brief Saves the selection, to know later whether the user changed it.
    void SaveLastSelection() noexcept;

    /// @brief Selects the option at @p optionIndex (or none, for -1), as @p flags ask.
    void SelectOption(int32 optionIndex, SelectOptionFlags flags = {}) noexcept;

    /// @brief Deselects every option but @p elementToExclude, without updating the validity.
    void DeselectItemsWithoutValidation(HTMLElement *elementToExclude = nullptr) noexcept;

    /// @brief Follows the `multiple` attribute: the drawing mode and, if it changed, the selectedness.
    void ParseMultipleAttribute(bool present) noexcept;

    /// @brief The list index of the last selected option, or -1.
    KRYS_NODISCARD int32 LastSelectedListIndex() const noexcept;

    /// @brief Selects the clicked item at @p listIndex, toggling it (@p multi) or extending the range to it
    /// (@p shift) in a list box that allows that.
    void UpdateSelectedState(int32 listIndex, bool multi, bool shift) noexcept;

    /// @brief The default event handling of a menu list.
    void MenuListDefaultEventHandler(Event &event) noexcept;

    /// @brief Opens a menu list's popup for an arrow key, where the theme does that.
    /// @return Whether the key was taken.
    bool PlatformHandleKeydownEvent(KeyboardEvent &event) noexcept;

    /// @brief The default event handling of a list box.
    void ListBoxDefaultEventHandler(Event &event) noexcept;

    /// @brief Tells the renderer the options changed.
    void SetOptionsChangedOnRenderer() noexcept;

    /// @brief Shows the menu list's popup, if the select is drawn as a menu list.
    void ShowPopup() noexcept;

    /// @brief The list index of the enabled option @p skip options from @p listIndex in @p direction, or the
    /// last one found.
    KRYS_NODISCARD int32 NextValidIndex(int32 listIndex, SkipDirection direction, int32 skip) const noexcept;

    /// @brief The enabled options next to, first, last, and a list box page away from a list index.
    KRYS_NODISCARD int32 NextSelectableListIndex(int32 startIndex) const noexcept;

    KRYS_NODISCARD int32 PreviousSelectableListIndex(int32 startIndex) const noexcept;

    KRYS_NODISCARD int32 FirstSelectableListIndex() const noexcept;

    KRYS_NODISCARD int32 LastSelectableListIndex() const noexcept;

    KRYS_NODISCARD int32 NextSelectableListIndexPageAway(int32 startIndex,
                                                         SkipDirection direction) const noexcept;

#pragma region TypeAheadDataSource

    KRYS_NODISCARD int32 IndexOfSelectedOption() const noexcept final;

    KRYS_NODISCARD int32 OptionCount() const noexcept final;

    KRYS_NODISCARD DOMString OptionAtIndex(int32 index) const noexcept final;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLSelectElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return Is<krys::boo::HTMLElement>(target)
           && Downcast<krys::boo::HTMLElement>(target).IsHTMLSelectElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLSelectElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
