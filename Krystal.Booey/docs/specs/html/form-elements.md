# HTML review checklist: 4.10.6 The button element

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 98 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html form-elements` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

#### [4.10.6 The button element](https://html.spec.whatwg.org/#the-button-element)

Status: reviewed (2026-11-01)
> `HTMLButtonElement`; the activation behaviour is `ActivationBehavior`, the command steps `HandleCommand` with `Element::IsValidCommandType` and `HandleCommandInternal`. A button whose parent is a select is still a submit button in the Auto state (no customizable selects, see `KNOWN-ISSUES.md`).
> Fixed: `type` returned the attribute rather than the button's state, and the activation behaviour didn't check that the document is fully active or that the target is still connected after the command event.
<!-- definitions: attr-button-type attr-button-commandfor attr-button-command is-valid-command-steps command-steps -->

- [x] [Auto](https://html.spec.whatwg.org/#attr-button-type-auto-state) — The attribute's missing value default and invalid value default are both the Auto state.

- [x] [Unknown](https://html.spec.whatwg.org/#attr-button-command-unknown-state) — The attribute's missing value default and invalid value default are both the Unknown state.

- [x] [custom command keyword](https://html.spec.whatwg.org/#attr-button-command-custom) — A custom command keyword is a string that starts with "--".

- [x] [determine if a command is valid for a target](https://html.spec.whatwg.org/#determine-if-command-is-valid) — To determine if a command is valid for a target given a command attribute command and an element target:
  - 1. If command is in the Unknown state, then return false.
  - 2. If command is in the Custom state, then return true.
  - 3. If target is not an HTML element, then return false.
  - 4. If command is in any of the following states: Toggle Popover; Show Popover; Hide Popover then return true.
  - 5. If this standard does not define is valid command steps for target's local name, then return false.
  - 6. Otherwise, return the result of running target's corresponding is valid command steps given command.

- [x] [A button element element's activation behavior given event](https://html.spec.whatwg.org/#determine-if-command-is-valid) — A button element element's activation behavior given event is:
  - 1. If element is disabled, then return.
  - 2. If element's node document is not fully active, then return.
  - 3. If element has a form owner:
    - 3a. If element is a submit button, then submit element's form owner from element with userInvolvement set to event's user navigation involvement, and return.
    - 3b. If element's type attribute is in the Reset Button state, then reset element's form owner, and return.
    - 3c. If element's type attribute is in the Auto state, then return.
  - 4. Let target be the result of running element's get the commandfor-associated element.
  - 5. If target is not null:
    - 5a. Let command be element's command attribute.
    - 5b. If the result of determining if a command is valid for a target given command and target is false, then return.
    - 5c. Let continue be the result of firing an event named command at target, using CommandEvent, with its command attribute initialized to command, its source attribute initialized to element, and its cancelable attribute initialized to true.
    - 5d. If continue is false, then return.
    - 5e. If target is not connected, then return.
    - 5f. If command is in the Custom state, then return.
    - 5g. If command is in the Hide Popover state:
      - 5g.i. Let validity be the result of running check popover validity given target, true, and null. If this throws an exception, catch it, and let validity be false.
      - 5g.ii. If validity is true, then run the hide popover algorithm given target, true, true, false, and element.
    - 5h. Otherwise, if command is in the Toggle Popover state:
      - 5h.i. Let validity be the result of running check popover validity given target, false, and null. If this throws an exception, catch it, and let validity be false.
      - 5h.ii. If validity is true, then run the show popover algorithm given target, false, and element.
      - 5h.iii. Otherwise:
        - 5h.iii.A. Let validity be the result of running check popover validity given target, true, and null. If this throws an exception, catch it, and let validity be false.
        - 5h.iii.B. If validity is true, then run the hide popover algorithm given target, true, true, false, and element.
    - 5i. Otherwise, if command is in the Show Popover state:
      - 5i.i. Let validity be the result of running check popover validity given target, false, and null. If this throws an exception, catch it, and let validity be false.
      - 5i.ii. If validity is true, then run the show popover algorithm given target, false, and element.
    - 5j. Otherwise, if this standard defines command steps for target's local name, then run the corresponding command steps given target, element, and command.
  - 6. Otherwise, run the popover target attribute activation behavior given element and event's target.

- [x] [HTMLButtonElement.command](https://html.spec.whatwg.org/#dom-button-command) — The command getter steps are:
  - 1. Let command be this's command attribute.
  - 2. If command is in the Custom state, then return command's value.
  - 3. If command is in the Unknown state, then return the empty string.
  - 4. Return the keyword corresponding to the value of command.

- [x] [value (button)](https://html.spec.whatwg.org/#attr-button-value) — The value attribute gives the element's value for the purposes of form submission. The element's value is the value of the element's value attribute, if there is one; otherwise the empty string. The element's optional value is the value of the element's value attribute, if there is one; otherwise null.

- [x] [HTMLButtonElement.type](https://html.spec.whatwg.org/#dom-button-type) — The type getter steps are:
  - 1. If this is a submit button, then return "submit".
  - 2. Let state be this's type attribute.
  - 3. Assert: state is not in the Submit Button state.
  - 4. If state is in the Auto state, then return "button".
  - 5. Return the keyword value corresponding to state.

#### [4.10.7 The select element](https://html.spec.whatwg.org/#the-select-element)

Status: reviewed (2026-11-01)
> `HTMLSelectElement`; the list of options and the selectedness setting algorithm are `RecalcListItems`, the reset algorithm `Reset`, and reset a select's selectedness `ResetSelectedness`. Opening the picker and focusing another option are for base appearance (no customizable selects).
> Fixed: `options` and `selectedOptions` were new collections each time; a disabled first option was selected; `selectedIndex`, `value` and picking didn't make the option dirty; adding or removing `multiple` with nothing selected reset the select; the placeholder label option wasn't found after an `<hr>`.
<!-- definitions: attr-select-multiple attr-select-size attr-select-required concept-select-toggle select-selected-index -->

- [x] [placeholder label option](https://html.spec.whatwg.org/#placeholder-label-option) — If a select element has a required attribute specified, and has a display size of 1; and if the value of the first option element in the select element's list of options (if any) is the empty string, and that option element does not have an ancestor optgroup in between itself and the select element, then that option is the select element's placeholder label option.

- [x] [The reset algorithm for a select element selectElement](https://html.spec.whatwg.org/#concept-form-reset-control) — The reset algorithm for a select element selectElement is:
  - 1. Set selectElement's user validity to false.
  - 2. For each optionElement of selectElement's list of options:
    - 2a. If optionElement has a selected attribute, then set optionElement's selectedness to true; otherwise set it to false.
    - 2b. Set optionElement's dirtiness to false.
  - 3. Run the selectedness setting algorithm given selectElement.
  - 4. Update a select's descendant selectedcontent elements given selectElement.

- [x] [pick an option](https://html.spec.whatwg.org/#concept-select-pick) — To pick an option given a select element select, an option element option, and an Event or null event:
  - 1. If select has the multiple attribute or select is disabled, then return.
  - 2. If event is not null and event's canceled flag is set, then return.
  - 3. If event is a MouseEvent and event's button attribute is not 0, then return.
  - 4. Set option's selectedness to true.
  - 5. Set option's dirtiness to true.
  - 6. Send select update notifications given select.
  - 7. If select is being rendered as a drop-down box with base appearance, then run the hide popover algorithm given select's select popover.

- [x] [open the picker](https://html.spec.whatwg.org/#select-open-picker) — If the select is being rendered as a drop-down box with base appearance, then the user agent should allow the user to open the picker given a corresponding select element select and a corresponding mousedown or keydown event event:
  - 1. If event's canceled flag is set, then return.
  - 2. If event is a MouseEvent and event's button attribute is not 0, then return.
  - 3. Run the show popover algorithm given select's select popover, false, and select.

- [x] [focus another option](https://html.spec.whatwg.org/#focus-another-option) — If the select is being rendered with base appearance, then the user agent should allow the user to focus another option given the new option element to focus option and a keydown event event:
  - 1. If event's canceled flag is set, then return.
  - 2. Run the focusing steps on option.

- [x] [display size](https://html.spec.whatwg.org/#concept-select-size) — The display size of a select element is the result of applying the rules for parsing non-negative integers to the value of the element's size attribute, if it has one and parsing it is successful. If applying those rules to the attribute's value is not successful, or if the size attribute is absent, then the element's display size is 4 if the element's multiple content attribute is present, and 1 otherwise.

- [x] [list of options](https://html.spec.whatwg.org/#concept-select-option-list) — To get the list of options given a select element select:
  - 1. Let options be « ».
  - 2. Let node be the first child of select in tree order.
  - 3. While node is not null:
    - 3a. If node is an option element, then append node to options.
    - 3b. If any of the following conditions are true: node is a select element;; node is an hr element;; node is an option element;; node is a datalist element;; node is an optgroup element and node has an ancestor optgroup in between itself and select, then set node to the next descendant of select in tree order, excluding node's descendants, if any such node exists; otherwise null. Otherwise, set node t…
  - 4. Return options.

- [x] [ask for a reset](https://html.spec.whatwg.org/#ask-for-a-reset) — To ask for a reset given an option element option:
  - 1. Let select be option's cached nearest ancestor select element.
  - 2. If select is null, then return.
  - 3. Run the selectedness setting algorithm given select.
  - 4. Update a select's descendant selectedcontent elements given select.

- [x] [selectedness setting algorithm](https://html.spec.whatwg.org/#selectedness-setting-algorithm) — To run the selectedness setting algorithm given a select element select, run these steps. They return a boolean.
  - 1. If select has the multiple attribute, then return false.
  - 2. Let hasChanged be false.
  - 3. Let firstEnabledOption be null.
  - 4. Let lastSelectedOption be null.
  - 5. For each option of select's list of options:
    - 5a. If option's selectedness is true:
      - 5a.i. If lastSelectedOption is not null:
        - 5a.i.A. Set lastSelectedOption's selectedness to false.
        - 5a.i.B. Set hasChanged to true.
      - 5a.ii. Set lastSelectedOption to option.
    - 5b. If firstEnabledOption is null and option is not disabled, then set firstEnabledOption to option.
  - 6. If lastSelectedOption is null, firstEnabledOption is not null, and select's display size is 1:
    - 6a. Set firstEnabledOption's selectedness to true.
    - 6b. Set hasChanged to true.
  - 7. Return hasChanged.

- [x] [reset a select's selectedness](https://html.spec.whatwg.org/#reset-a-select's-selectedness) — To reset a select's selectedness, given a select element select and a boolean hasChanged:
  - 1. If the result of running the selectedness setting algorithm given select is true, then set hasChanged to true.
  - 2. If hasChanged is false, then return.
  - 3. Update a select's descendant selectedcontent elements given select.

- [x] [Whenever a select element select's multiple attribute is added or removed, run…](https://html.spec.whatwg.org/#reset-a-select's-selectedness) — Whenever a select element select's multiple attribute is added or removed, run these steps:
  - 1. If the multiple attribute was removed, then set the selectedness of all but the first option element with its selectedness set to true in select's list of options to false.
  - 2. Reset a select's selectedness given select and true.

- [x] [send select update notifications](https://html.spec.whatwg.org/#send-select-update-notifications) — To send select update notifications for a select element element, queue an element task on the user interaction task source given element to run these steps:
  - 1. Set element's user validity to true.
  - 2. Update a select's descendant selectedcontent elements given element.
  - 3. Clone selected option into select button given element.
  - 4. Fire an event named input at element, with the bubbles and composed attributes initialized to true.
  - 5. Fire an event named change at element, with the bubbles attribute initialized to true.

- [x] [HTMLSelectElement.type](https://html.spec.whatwg.org/#dom-select-type) — The type IDL attribute, on getting, must return the string "select-one" if the multiple attribute is absent, and the string "select-multiple" if the multiple attribute is present.

- [x] [HTMLSelectElement.options](https://html.spec.whatwg.org/#dom-select-options) — The options IDL attribute must return an HTMLOptionsCollection rooted at the select node, whose filter matches the elements in the list of options.

- [x] [HTMLSelectElement.length](https://html.spec.whatwg.org/#dom-select-length) — The length IDL attribute must return the number of nodes represented by the options collection. On setting, it must act like the attribute of the same name on the options collection.

- [x] [HTMLSelectElement.item(index)](https://html.spec.whatwg.org/#dom-select-item) — The item(index) method must return the value returned by the method of the same name on the options collection, when invoked with the same argument.

- [x] [HTMLSelectElement.namedItem(name)](https://html.spec.whatwg.org/#dom-select-nameditem) — The namedItem(name) method must return the value returned by the method of the same name on the options collection, when invoked with the same argument.

- [x] [HTMLSelectElement.add(element, before)](https://html.spec.whatwg.org/#dom-select-add) — Similarly, the add(element, before) method must act like its namesake method on that same options collection.

- [x] [HTMLSelectElement.remove()](https://html.spec.whatwg.org/#dom-select-remove) — The remove() method must act like its namesake method on that same options collection when it has arguments, and like its namesake method on the ChildNode interface implemented by the HTMLSelectElement ancestor interface Element when it has no arguments.

- [x] [HTMLSelectElement.selectedOptions](https://html.spec.whatwg.org/#dom-select-selectedoptions) — The selectedOptions IDL attribute must return an HTMLCollection rooted at the select node, whose filter matches the elements in the list of options that have their selectedness set to true.

- [x] [set the selected index](https://html.spec.whatwg.org/#set-the-selected-index) — To set the selected index of a select element select to a value value:
  - 1. Let firstMatchingOption be null.
  - 2. For each option of select's list of options:
    - 2a. Set option's selectedness to false.
    - 2b. If firstMatchingOption is null and option's index is equal to value, then set firstMatchingOption to option.
  - 3. If firstMatchingOption is not null, then set firstMatchingOption's selectedness to true and set firstMatchingOption's dirtiness to true.
  - 4. Update a select's descendant selectedcontent elements given select.

- [x] [HTMLSelectElement.selectedIndex](https://html.spec.whatwg.org/#dom-select-selectedindex) — The selectedIndex getter steps are to return this's selected index.

- [x] [The selectedIndex setter steps](https://html.spec.whatwg.org/#dom-select-selectedindex) — The selectedIndex setter steps are to set the selected index of this to the given value.

- [x] [HTMLSelectElement.value](https://html.spec.whatwg.org/#dom-select-value) — The value getter steps are to return the value of the first option element in this's list of options in tree order that has its selectedness set to true, if any. If there isn't one, then return the empty string.

- [x] [The value setter steps](https://html.spec.whatwg.org/#dom-select-value) — The value setter steps are:
  - 1. Let firstMatchingOption be null.
  - 2. For each option of this's list of options:
    - 2a. Set option's selectedness to false.
    - 2b. If firstMatchingOption is null and option's value is equal to the given value, then set firstMatchingOption to option.
  - 3. If firstMatchingOption is not null, then set firstMatchingOption's selectedness to true and set firstMatchingOption's dirtiness to true.
  - 4. Update a select's descendant selectedcontent elements given this.

#### [4.10.8 The datalist element](https://html.spec.whatwg.org/#the-datalist-element)

Status: reviewed (2026-11-01)
> `HTMLDataListElement`; its descendants are barred from constraint validation in `ValidatedFormListedElement`.
> Added: `options`.

- [x] [HTMLDataListElement.options](https://html.spec.whatwg.org/#dom-datalist-options) — The options IDL attribute must return an HTMLCollection rooted at the datalist node, whose filter matches option elements.

#### [4.10.9 The optgroup element](https://html.spec.whatwg.org/#the-optgroup-element)

Status: reviewed (2026-11-01)
> `HTMLOptGroupElement`. Its label is only its `label` attribute: a legend child shows only with base appearance (`KNOWN-ISSUES.md`).
> Fixed: changing `disabled` didn't reset its select's selectedness straight away.
<!-- definitions: attr-optgroup-label -->

- [x] [element's group of option elements](https://html.spec.whatwg.org/#optgroup-option-group) — The element's group of option elements consists of the option elements that are descendants of the optgroup element.

- [x] [Whenever an optgroup element's disabled attribute is added or removed, then](https://html.spec.whatwg.org/#attr-optgroup-disabled) — Whenever an optgroup element's disabled attribute is added or removed, then:
  - 1. Let select be the optgroup element's nearest ancestor select.
  - 2. If select is null, then return.
  - 3. Reset a select's selectedness given select and false.

- [x] [get an optgroup element's label](https://html.spec.whatwg.org/#concept-optgroup-label) — To get an optgroup element's label, given an optgroup optgroup:
  - 1. If optgroup's first element child is a legend element, then return the result of get HTML-aware text content given that legend element and true.
  - 2. If optgroup has a label attribute, then return the value of that attribute.
  - 3. Return the empty string.

#### [4.10.10 The option element](https://html.spec.whatwg.org/#the-option-element)

Status: reviewed (2026-11-01)
> `HTMLOptionElement`; the nearest ancestor select is `HTMLSelectElement::FindOwnerSelect`, the label `OptionLabel`, and HTML-aware text content `CollectOptionInnerText`. Clone selected option into select button is for base appearance (no customizable selects).
> Fixed: changing `disabled` didn't reset its select's selectedness, an empty `label` attribute showed an empty label, and `Option()` made the option dirty.
<!-- definitions: attr-option-label concept-option-value attr-option-selected concept-option-dirtiness cached-nearest-ancestor-select-element -->

- [x] [disabled (option)](https://html.spec.whatwg.org/#attr-option-disabled) — The disabled attribute is a boolean attribute. An option element option is disabled if the following steps return true:
  - 1. If option's disabled attribute is present, then return true.
  - 2. For each ancestor of option's ancestors in reverse tree order:
    - 2a. If ancestor is a select, hr, datalist, or option element, then return false.
    - 2b. If ancestor is an optgroup element, then return true if ancestor's disabled attribute is present; otherwise false.
  - 3. Return false.

- [x] [Whenever an option element's disabled attribute is added or removed, then](https://html.spec.whatwg.org/#concept-option-disabled) — Whenever an option element's disabled attribute is added or removed, then:
  - 1. Let select be the option element's cached nearest ancestor select element.
  - 2. If select is null, then return.
  - 3. Reset a select's selectedness given select and false.

- [x] [get an option element's label](https://html.spec.whatwg.org/#concept-option-label) — To get an option element's label, given an option element option and a boolean includeAltText:
  - 1. If option has a label attribute whose value is not the empty string, then return that value.
  - 2. Return the result of get HTML-aware text content given option and includeAltText.

- [x] [value (option)](https://html.spec.whatwg.org/#attr-option-value) — The value attribute provides a value for the element. The value of an option element is the value of the value content attribute, if there is one, or, if there is not, the result of get HTML-aware text content given this and false.

- [x] [selectedness](https://html.spec.whatwg.org/#concept-option-selectedness) — The selectedness of an option element is a boolean state, initially false. Except where otherwise specified, when the element is created, its selectedness must be set to true if the element has a selected attribute.

- [x] [Whenever an option element option's selected attribute is added or removed](https://html.spec.whatwg.org/#concept-option-selectedness) — Whenever an option element option's selected attribute is added or removed:
  - 1. Let hasChanged be false.
  - 2. If option's dirtiness is false:
    - 2a. If the selected attribute was added and option's selectedness is false, then set option's selectedness to true and set hasChanged to true.
    - 2b. If the selected attribute was removed and option's selectedness is true, then set option's selectedness to false and set hasChanged to true.
  - 3. Let select be option's cached nearest ancestor select element.
  - 4. If select is null, then return.
  - 5. Reset a select's selectedness given select and hasChanged.

- [x] [index](https://html.spec.whatwg.org/#concept-option-index) — An option element's index is the number of option elements that are in the same list of options but that come before it in tree order. If the option element is not in a list of options, then the option element's index is zero.

- [x] [update an option's nearest ancestor select](https://html.spec.whatwg.org/#update-an-option's-nearest-ancestor-select) — To update an option's nearest ancestor select, given an option option, run these steps. They return a list of select elements whose descendant selectedcontent elements need updating.
  - 1. Let oldSelect be option's cached nearest ancestor select element.
  - 2. Let newSelect be option's nearest ancestor select.
  - 3. Set option's cached nearest ancestor select element to newSelect.
  - 4. If oldSelect is newSelect, then return « ».
  - 5. Let result be « ».
  - 6. For each select of « oldSelect, newSelect »:
    - 6a. If select is null, then continue.
    - 6b. Let hasChanged be the result of running the selectedness setting algorithm given select.
    - 6c. If option's selectedness is true or hasChanged is true, then append select to result.
  - 7. Return result.

- [x] [update descendant selectedcontent elements for an option](https://html.spec.whatwg.org/#update-descendant-selectedcontent-elements-for-an-option) — To update descendant selectedcontent elements for an option, given an option element option:
  - 1. Let select be option's cached nearest ancestor select element.
  - 2. If select is null or option's selectedness is false, then return.
  - 3. Update a select's descendant selectedcontent elements given select.

- [x] [The option HTML element removing steps, given removedNode, isSubtreeRoot, and o…](https://html.spec.whatwg.org/#the-option-element) — The option HTML element removing steps, given removedNode, isSubtreeRoot, and oldAncestor, are:
  - 1. Let selects be the result of update an option's nearest ancestor select given removedNode.
  - 2. For each select of selects: queue a microtask to update a select's descendant selectedcontent elements given select.

- [x] [The option HTML element moving steps, given movedNode, isSubtreeRoot, and oldAn…](https://html.spec.whatwg.org/#the-option-element) — The option HTML element moving steps, given movedNode, isSubtreeRoot, and oldAncestor, are:
  - 1. Let selects be the result of update an option's nearest ancestor select given movedNode.
  - 2. For each select of selects: queue a microtask to update a select's descendant selectedcontent elements given select.

- [x] [nearest ancestor select](https://html.spec.whatwg.org/#option-element-nearest-ancestor-select) — To get the nearest ancestor select given an Element element, run these steps. They return a select element or null.
  - 1. Let ancestorOptgroup be null.
  - 2. For each ancestor of element's ancestors, in reverse tree order:
    - 2a. If ancestor is a datalist, hr, or option element, then return null.
    - 2b. If ancestor is an optgroup element:
      - 2b.i. If ancestorOptgroup is not null, then return null.
      - 2b.ii. Set ancestorOptgroup to ancestor.
    - 2c. If ancestor is a select, then return ancestor.
  - 3. Return null.

- [x] [clone selected option into select button](https://html.spec.whatwg.org/#clone-selected-option-into-select-button) — To clone selected option into select button, given a select element select:
  - 1. Let option be the first element of select's option list whose selectedness is set to true, if such an element exists; otherwise null.
  - 2. Let text be the empty string.
  - 3. If option is not null, then set text to option's label.
  - 4. Set select's select fallback button text to text.

- [x] [HTMLOptionElement.label](https://html.spec.whatwg.org/#dom-option-label) — The label getter steps are:
  - 1. Let attribute be this's label attribute.
  - 2. If attribute is null, then return the result of getting an option element's label given this and false.
  - 3. Return attribute's value.

- [x] [HTMLOptionElement.value](https://html.spec.whatwg.org/#dom-option-value) — The value getter steps are to return this's value.

- [x] [HTMLOptionElement.selected](https://html.spec.whatwg.org/#dom-option-selected) — The selected getter steps are to return true if this's selectedness is true, and false otherwise.

- [x] [The selected setter steps](https://html.spec.whatwg.org/#dom-option-selected) — The selected setter steps are:
  - 1. Set this's selectedness to the given value.
  - 2. Set this's dirtiness to true.
  - 3. Ask for a reset given this.

- [x] [HTMLOptionElement.index](https://html.spec.whatwg.org/#dom-option-index) — The index IDL attribute must return the element's index.

- [x] [HTMLOptionElement.text](https://html.spec.whatwg.org/#dom-option-text) — The text getter steps are to return the result of get HTML-aware text content given this and false.

- [x] [The text setter steps](https://html.spec.whatwg.org/#dom-option-text) — The text setter steps are to string replace all with the given value within this.

- [x] [HTMLOptionElement.form](https://html.spec.whatwg.org/#dom-option-form) — The form getter steps are:
  - 1. Let select be this's nearest ancestor select.
  - 2. If select is null, then return null.
  - 3. Return select's form owner.

- [x] [HTMLOptionElement.Option(text, value, defaultSelected, selected)](https://html.spec.whatwg.org/#dom-option) — A legacy factory function is provided for creating HTMLOptionElement objects (in addition to the factory methods from DOM such as createElement()): Option(text, value, defaultSelected, selected). When invoked, the legacy factory function must perform the following steps:
  - 1. Let document be the current global object's associated Document.
  - 2. Let option be the result of creating an element given document, "option", and the HTML namespace.
  - 3. If text is not the empty string, then append to option a new Text node whose data is text.
  - 4. If value is given, then set an attribute value for option using "value" and value.
  - 5. If defaultSelected is true, then set an attribute value for option using "selected" and the empty string.
  - 6. If selected is true, then set option's selectedness to true; otherwise set its selectedness to false (even if defaultSelected is true).
  - 7. Return option.

- [x] [get HTML-aware text content](https://html.spec.whatwg.org/#get-html-aware-text-content) — To get HTML-aware text content, given an element element and a boolean includeAltText:
  - 1. Let text be the empty string.
  - 2. For each descendant descendant of element in tree order:
    - 2a. If descendant is a script or SVG script element, then continue skipping all descendants of descendant.
    - 2b. If descendant is a Text node, then set text to the concatenation of text and descendant's data.
    - 2c. If descendant is an img element with an alt attribute and includeAltText is true, then set text to the concatenation of text and the value of descendant's alt attribute.
    - 2d. If descendant is an img element, then continue skipping all descendants of descendant.
  - 3. Return the result of strip and collapse ASCII whitespace given text.

#### [4.10.11 The textarea element](https://html.spec.whatwg.org/#the-textarea-element)

Status: reviewed (2026-11-01)
> `HTMLTextAreaElement`; the wrapping transformation is `ValueWithHardLineBreaks`. Changing the writing direction is the editor's.
> Fixed: `wrap` took the legacy `physical` and `on` as `hard`.
<!-- definitions: concept-textarea-raw-value attr-textarea-cols-value attr-textarea-rows-value attr-textarea-wrap attr-textarea-maxlength attr-textarea-minlength attr-textarea-required attr-textarea-placeholder -->

- [x] [If the element is mutable, the user agent should allow the user to change the w…](https://html.spec.whatwg.org/#attr-textarea-readonly) — If the element is mutable, the user agent should allow the user to change the writing direction of the element, setting it either to a left-to-right writing direction or a right-to-left writing direction. If the user does so, the user agent must then run the following steps:
  - 1. Set the element's dir attribute to "ltr" if the user selected a left-to-right writing direction, and "rtl" if the user selected a right-to-left writing direction.
  - 2. Queue an element task on the user interaction task source given the textarea element to fire an event named input at the textarea element, with the bubbles and composed attributes initialized to true.

- [x] [cols (textarea)](https://html.spec.whatwg.org/#attr-textarea-cols) — The cols attribute specifies the expected maximum number of characters per line. If the cols attribute is specified, its value must be a valid non-negative integer greater than zero. If applying the rules for parsing non-negative integers to the attribute's value results in a number greater than zero, then the element's character width is that value; otherwise, it is 20.

- [x] [rows (textarea)](https://html.spec.whatwg.org/#attr-textarea-rows) — The rows attribute specifies the number of lines to show. If the rows attribute is specified, its value must be a valid non-negative integer greater than zero. If applying the rules for parsing non-negative integers to the attribute's value results in a number greater than zero, then the element's character height is that value; otherwise, it is 2.

- [x] [textarea wrapping transformation](https://html.spec.whatwg.org/#textarea-wrapping-transformation) — The element's value is defined to be the element's API value with the textarea wrapping transformation applied. The textarea wrapping transformation is the following algorithm, as applied to a string:
  - 1. If the element's wrap attribute is in the Hard state, insert U+000A LINE FEED (LF) characters into the string using an implementation-defined algorithm so that each line has no more than character width characters. For the purposes of this requirement, lines are delimited by the start of the string, the end of the string, and U+000A LINE FEED (LF) characters.

- [x] [HTMLTextAreaElement.type](https://html.spec.whatwg.org/#dom-textarea-type) — The type IDL attribute must return the value "textarea".

- [x] [HTMLTextAreaElement.defaultValue](https://html.spec.whatwg.org/#dom-textarea-defaultvalue) — The defaultValue attribute's getter must return the element's child text content.

- [x] [HTMLTextAreaElement.value](https://html.spec.whatwg.org/#dom-textarea-value) — The value IDL attribute must, on getting, return the element's API value. On setting, it must perform the following steps:
  - 1. Let oldAPIValue be this element's API value.
  - 2. Set this element's raw value to the new value.
  - 3. Set this element's dirty value flag to true.
  - 4. If the new API value is different from oldAPIValue, then move the text entry cursor position to the end of the text control, unselecting any selected text and resetting the selection direction to "none".

- [x] [HTMLTextAreaElement.textLength](https://html.spec.whatwg.org/#dom-textarea-textlength) — The textLength IDL attribute must return the length of the element's API value.

#### [4.10.12 The output element](https://html.spec.whatwg.org/#the-output-element)

Status: reviewed (2026-11-01)
> `HTMLOutputElement`.
<!-- definitions: attr-output-for concept-output-default-value-override -->

- [x] [default value](https://html.spec.whatwg.org/#concept-output-default-value) — The element's default value is determined by the following steps:
  - 1. If this element's default value override is non-null, then return it.
  - 2. Return this element's descendant text content.

- [x] [The reset algorithm for output elements](https://html.spec.whatwg.org/#concept-form-reset-control) — The reset algorithm for output elements is to run these steps:
  - 1. String replace all with this element's default value within this element.
  - 2. Set this element's default value override to null.

- [x] [HTMLOutputElement.value](https://html.spec.whatwg.org/#dom-output-value) — The value getter steps are to return this's descendant text content.

- [x] [The value setter steps](https://html.spec.whatwg.org/#dom-output-value) — The value setter steps are:
  - 1. Set this's default value override to its default value.
  - 2. String replace all with the given value within this.

- [x] [HTMLOutputElement.defaultValue](https://html.spec.whatwg.org/#dom-output-defaultvalue) — The defaultValue getter steps are to return the result of running this's default value.

- [x] [The defaultValue setter steps](https://html.spec.whatwg.org/#dom-output-defaultvalue) — The defaultValue setter steps are:
  - 1. If this's default value override is null, then string replace all with the given value within this and return.
  - 2. Set this's default value override to the given value.

- [x] [HTMLOutputElement.type](https://html.spec.whatwg.org/#dom-output-type) — The type getter steps are to return "output".

#### [4.10.13 The progress element](https://html.spec.whatwg.org/#the-progress-element)

Status: reviewed (2026-11-01)
> `HTMLProgressElement`.
<!-- definitions: attr-progress-value attr-progress-max -->

- [x] [maximum value](https://html.spec.whatwg.org/#concept-progress-maximum) — If the progress bar is a determinate progress bar and the element has a max attribute, the user agent must parse the max attribute's value according to the rules for parsing floating-point number values. If this does not result in an error, and if the parsed value is greater than zero, then the maximum value of the progress bar is that value. Otherwise, if the element has no max attribute, or if it has one but parsing it resulted in an error, or if the parsed value was less than or equal to zero, then the maximum value of the progress bar is 1.0.

- [x] [value](https://html.spec.whatwg.org/#concept-progress-value) — If the progress bar is a determinate progress bar, user agents must parse the value attribute's value according to the rules for parsing floating-point number values. If this does not result in an error and the parsed value is greater than zero, then the value of the progress bar is that parsed value. Otherwise, if parsing the value attribute's value resulted in an error or a number less than or equal to zero, then the value of the progress bar is zero.

- [x] [current value](https://html.spec.whatwg.org/#concept-progress-current-value) — If the progress bar is a determinate progress bar, then the current value is the maximum value, if value is greater than the maximum value, and value otherwise.

- [x] [HTMLProgressElement.position](https://html.spec.whatwg.org/#dom-progress-position) — If the progress bar is an indeterminate progress bar, then the position IDL attribute must return −1. Otherwise, it must return the result of dividing the current value by the maximum value.

- [x] [HTMLProgressElement.value](https://html.spec.whatwg.org/#dom-progress-value) — The value getter steps are to return 0 if this is an indeterminate progress bar; otherwise this's current value.

#### [4.10.14 The meter element](https://html.spec.whatwg.org/#the-meter-element)

Status: reviewed (2026-11-01)
> `HTMLMeterElement`.
<!-- definitions: attr-meter-min attr-meter-max attr-meter-value attr-meter-low attr-meter-high attr-meter-optimum concept-meter-minimum concept-meter-maximum concept-meter-actual concept-meter-low concept-meter-high concept-meter-optimum -->

- [x] [HTMLMeterElement.value](https://html.spec.whatwg.org/#dom-meter-value) — The value getter steps are to return this's actual value.

- [x] [HTMLMeterElement.min](https://html.spec.whatwg.org/#dom-meter-min) — The min getter steps are to return this's minimum value.

- [x] [HTMLMeterElement.max](https://html.spec.whatwg.org/#dom-meter-max) — The max getter steps are to return this's maximum value.

- [x] [HTMLMeterElement.low](https://html.spec.whatwg.org/#dom-meter-low) — The low getter steps are to return this's low boundary.

- [x] [HTMLMeterElement.high](https://html.spec.whatwg.org/#dom-meter-high) — The high getter steps are to return this's high boundary.

- [x] [HTMLMeterElement.optimum](https://html.spec.whatwg.org/#dom-meter-optimum) — The optimum getter steps are to return this's optimum value.

#### [4.10.15 The fieldset element](https://html.spec.whatwg.org/#the-fieldset-element)

Status: reviewed (2026-11-01)
> `HTMLFieldSetElement`; disabled fieldsets are `IsActuallyDisabled` and `DisabledStateChanged`.
> Fixed: `elements` was a new collection each time.

- [x] [disabled (fieldset)](https://html.spec.whatwg.org/#attr-fieldset-disabled) — The disabled attribute, when specified, causes all the form control descendants of the fieldset element, excluding those that are descendants of the fieldset element's first legend element child, if any, to be disabled.

- [x] [disabled fieldset](https://html.spec.whatwg.org/#concept-fieldset-disabled) — A fieldset element is a disabled fieldset if it matches any of the following conditions:

- [x] [HTMLFieldSetElement.type](https://html.spec.whatwg.org/#dom-fieldset-type) — The type IDL attribute must return the string "fieldset".

- [x] [HTMLFieldSetElement.elements](https://html.spec.whatwg.org/#dom-fieldset-elements) — The elements IDL attribute must return an HTMLCollection rooted at the fieldset element, whose filter matches listed elements.

#### [4.10.16 The legend element](https://html.spec.whatwg.org/#the-legend-element)

Status: reviewed (2026-11-01)
> `HTMLLegendElement::Form`.

- [x] [HTMLLegendElement.form](https://html.spec.whatwg.org/#dom-legend-form) — The form IDL attribute's behavior depends on whether the legend element is in a fieldset element or not. If the legend has a fieldset element as its parent, then the form IDL attribute must return the same value as the form IDL attribute on that fieldset element. Otherwise, it must return null.

#### [4.10.17 The selectedcontent element](https://html.spec.whatwg.org/#the-selectedcontent-element)

Status: reviewed (2026-11-01)
> `HTMLSelectedContentElement` and `FormControlAlgorithms`; the select and option algorithms update it.
> Fixed: it followed an older version of the specification, mirroring an option only when the parser popped it, and was never disabled.
<!-- definitions: selectedcontent-disabled -->

- [x] [update a select's descendant selectedcontent elements](https://html.spec.whatwg.org/#update-a-select's-descendant-selectedcontent-elements) — To update a select's descendant selectedcontent elements given a select element select:
  - 1. If select has the multiple attribute, then return.
  - 2. Let descendantSelectedcontents be a list of select's descendants, in tree order, that are selectedcontent elements whose disabled is false.
  - 3. For each selectedcontent of descendantSelectedcontents: update a selectedcontent given select and selectedcontent.

- [x] [update a selectedcontent](https://html.spec.whatwg.org/#update-a-selectedcontent) — To update a selectedcontent given a select element select and a selectedcontent element selectedcontent:
  - 1. If selectedcontent's disabled is true, then return.
  - 2. Let option be the first option in select's list of options whose selectedness is true, if any such option exists; otherwise null.
  - 3. If option is null, then clear a selectedcontent given selectedcontent.
  - 4. Otherwise, clone an option into a selectedcontent given option and selectedcontent.

- [x] [clone an option into a selectedcontent](https://html.spec.whatwg.org/#clone-an-option-into-a-selectedcontent) — To clone an option into a selectedcontent, given an option element option and a selectedcontent element selectedcontent:
  - 1. Let documentFragment be the result of creating a document fragment given option's node document.
  - 2. For each child of option's children:
    - 2a. Let childClone be the result of running clone given child with subtree set to true.
    - 2b. Append childClone to documentFragment.
  - 3. Replace all with documentFragment within selectedcontent.

- [x] [clear a selectedcontent](https://html.spec.whatwg.org/#clear-a-selectedcontent) — To clear a selectedcontent given a selectedcontent element selectedcontent:
  - 1. Replace all with null within selectedcontent.

- [x] [The selectedcontent HTML element post-connection steps, given selectedcontent,](https://html.spec.whatwg.org/#the-selectedcontent-element) — The selectedcontent HTML element post-connection steps, given selectedcontent, are:
  - 1. Let select be the result of recalculate a selectedcontent element's disabledness given selectedcontent.
  - 2. If selectedcontent's disabled is true, select is null, or select has the multiple attribute, then return.
  - 3. Update a selectedcontent given select and selectedcontent.

- [x] [The selectedcontent HTML element moving steps, given movedNode, isSubtreeRoot,…](https://html.spec.whatwg.org/#the-selectedcontent-element) — The selectedcontent HTML element moving steps, given movedNode, isSubtreeRoot, and oldAncestor, are:
  - 1. Let select be the result of recalculate a selectedcontent element's disabledness given movedNode.
  - 2. If movedNode's disabled is true, select is null, or select has the multiple attribute, then return.
  - 3. Queue a microtask to update a selectedcontent given select and movedNode.

- [x] [recalculate a selectedcontent element's disabledness](https://html.spec.whatwg.org/#recalculate-a-selectedcontent-element's-disabledness) — To recalculate a selectedcontent element's disabledness, given a selectedcontent element selectedcontent, run these steps. They return a select element or null.
  - 1. Let nearestSelectAncestor be null.
  - 2. Set selectedcontent's disabled to false.
  - 3. For each ancestor of selectedcontent's ancestors, in reverse tree order:
    - 3a. If ancestor is a select element:
      - 3a.i. If nearestSelectAncestor is null, then set nearestSelectAncestor to ancestor and continue.
      - 3a.ii. Set selectedcontent's disabled to true and break.
    - 3b. If ancestor is an option element or a selectedcontent element, then set selectedcontent's disabled to true and break.
  - 4. Return nearestSelectAncestor.
