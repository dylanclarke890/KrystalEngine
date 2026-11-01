# HTML review checklist: 4.10.18 Form control infrastructure

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 60 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html form-control-infrastructure` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

#### [4.10.18 Form control infrastructure](https://html.spec.whatwg.org/#form-control-infrastructure)

Status: reviewed (2026-11-01)
> Headings only.

##### [4.10.18.1 A form control's value](https://html.spec.whatwg.org/#a-form-control's-value)

Status: reviewed (2026-11-01)
> Each control's value, checkedness and dirty flags (`HTMLInputElement`, `HTMLTextAreaElement`, `HTMLSelectElement`); user validity is `SetInteractedWithSinceLastFormSubmitEvent`.
<!-- definitions: concept-fe-value concept-fe-checked concept-fe-dirty concept-fe-optional-value user-validity concept-fe-values concept-fe-api-value -->

##### [4.10.18.2 Mutability](https://html.spec.whatwg.org/#mutability)

Status: reviewed (2026-11-01)
> `HTMLFormControlElement::IsMutable` and the types' `SupportsReadOnly`.
<!-- definitions: concept-fe-mutable -->

##### [4.10.18.3 Association of controls and forms](https://html.spec.whatwg.org/#association-of-controls-and-forms)

Status: reviewed (2026-11-01)
> `FormAssociatedElement` and `FormListedElement::ResetFormOwner`, with `FindAssociatedForm` and the form attribute's ID target observer; the parser's form element pointer is `SetFormSetByParser`.
<!-- definitions: form-owner parser-inserted-flag attr-fae-form -->

- [x] [associated](https://html.spec.whatwg.org/#concept-form-association) — When a form-associated element is to be associated with a form, its form owner must be set to that form.

- [x] [reset the form owner](https://html.spec.whatwg.org/#reset-the-form-owner) — To reset the form owner of a form-associated element element:
  - 1. Unset element's parser inserted flag.
  - 2. If all of the following are true: element's form owner is not null;; element is not listed or its form content attribute is not present; and; element's form owner is its nearest form element ancestor after the change to the ancestor chain, then return.
  - 3. Set element's form owner to null.
  - 4. If element is listed, has a form content attribute, and is connected:
    - 4a. If the first element in element's tree, in tree order, to have an ID that is identical to element's form content attribute's value, is a form element, then associate the element with that form element.
  - 5. Otherwise, if element has an ancestor form element, then associate element with the nearest such ancestor form element.

- [x] [HTMLObjectElement,HTMLInputElement,HTMLButtonElement,HTMLSelectElement,HTMLTextAreaElement,HTMLOutputElement,HTMLFieldSetElement.form](https://html.spec.whatwg.org/#dom-fae-form) — Listed form-associated elements except for form-associated custom elements have a form IDL attribute, which, on getting, must return the element's form owner, or null if there isn't one.

- [x] [ElementInternals.form](https://html.spec.whatwg.org/#dom-elementinternals-form) — Form-associated custom elements don't have a form IDL attribute. Instead, their ElementInternals object has a form IDL attribute. On getting, it must throw a "NotSupportedError" DOMException if the target element is not a form-associated custom element. Otherwise, it must return the element's form owner, or null if there isn't one.

#### [4.10.19 Attributes common to form controls](https://html.spec.whatwg.org/#attributes-common-to-form-controls)

Status: reviewed (2026-11-01)
> Headings only.

##### [4.10.19.1 Naming form controls: the name attribute](https://html.spec.whatwg.org/#naming-form-controls:-the-name-attribute)

Status: reviewed (2026-11-01)
> Each control's `Name`; `_charset_` is in `HTMLInputElement::AppendFormData`.
<!-- definitions: attr-fe-name attr-fe-name-charset -->

##### [4.10.19.2 Submitting element directionality: the dirname attribute](https://html.spec.whatwg.org/#submitting-element-directionality:-the-dirname-attribute)

Status: reviewed (2026-11-01)
> `DirectionForFormData`.
<!-- definitions: attr-fe-dirname -->

##### [4.10.19.3 Limiting user input length: the maxlength attribute](https://html.spec.whatwg.org/#limiting-user-input-length:-the-maxlength-attribute)

Status: reviewed (2026-11-01)
> `ParseLengthAttribute`, `MaxLength`.
<!-- definitions: attr-fe-maxlength -->

- [x] [maximum allowed value length](https://html.spec.whatwg.org/#maximum-allowed-value-length) — If an element has its form control maxlength attribute specified, the attribute's value must be a valid non-negative integer. If the attribute is specified and applying the rules for parsing non-negative integers to its value results in a number, then that number is the element's maximum allowed value length. If the attribute is omitted or parsing its value results in an error, then there is no maximum allowed value length.

##### [4.10.19.4 Setting minimum input length requirements: the minlength attribute](https://html.spec.whatwg.org/#setting-minimum-input-length-requirements:-the-minlength-attribute)

Status: reviewed (2026-11-01)
> `ParseLengthAttribute`, `MinLength`.
<!-- definitions: attr-fe-minlength -->

- [x] [minimum allowed value length](https://html.spec.whatwg.org/#minimum-allowed-value-length) — If an element has its form control minlength attribute specified, the attribute's value must be a valid non-negative integer. If the attribute is specified and applying the rules for parsing non-negative integers to its value results in a number, then that number is the element's minimum allowed value length. If the attribute is omitted or parsing its value results in an error, then there is no minimum allowed value length.

##### [4.10.19.5 Enabling and disabling form controls: the disabled attribute](https://html.spec.whatwg.org/#enabling-and-disabling-form-controls:-the-disabled-attribute)

Status: reviewed (2026-11-01)
> `HTMLFormControlElement::IsDisabledFormControl` and the fieldset's `SetDisabledByAncestorFieldset`.
<!-- definitions: attr-fe-disabled -->

- [x] [disabled](https://html.spec.whatwg.org/#concept-fe-disabled) — A form control is disabled if any of the following are true:

##### [4.10.19.6 Form submission attributes](https://html.spec.whatwg.org/#form-submission-attributes)

Status: reviewed (2026-11-01)
> `FormSubmission::Attributes`; the IDL attributes are on `HTMLFormElement`, `HTMLInputElement` and `HTMLButtonElement`.
> Fixed: `method`, `enctype`, `encoding`, `formMethod` and `formEnctype` weren't limited to known values, `encoding` reflected a non-existent `encoding` attribute, and a GET or POST submission's encoding went by the form's method rather than the submitter's.
<!-- definitions: attributes-for-form-submission attr-fs-action attr-fs-formaction concept-fs-action attr-fs-method attr-fs-formmethod attr-fs-enctype attr-fs-formenctype attr-fs-enctype-urlencoded attr-fs-enctype-formdata attr-fs-enctype-text attr-fs-target attr-fs-formtarget attr-fs-novalidate attr-fs-formnovalidate dom-fs-enctype dom-fs-encoding dom-fs-formenctype dom-fs-formmethod -->

- [x] [method](https://html.spec.whatwg.org/#concept-fs-method) — The method of an element is one of those states. If the element is a submit button and has a formmethod attribute, then the element's method is that attribute's state; otherwise, it is the form owner's method attribute's state.

- [x] [enctype](https://html.spec.whatwg.org/#concept-fs-enctype) — The enctype of an element is one of those three states. If the element is a submit button and has a formenctype attribute, then the element's enctype is that attribute's state; otherwise, it is the form owner's enctype attribute's state.

- [x] [no-validate state](https://html.spec.whatwg.org/#concept-fs-novalidate) — The no-validate state of an element is true if the element is a submit button and the element's formnovalidate attribute is present, or if the element's form owner's novalidate attribute is present, and false otherwise.

- [x] [HTMLFormElement.action](https://html.spec.whatwg.org/#dom-fs-action) — The action getter steps are:
  - 1. Let attribute be this's action attribute.
  - 2. If attribute is null or attribute's value is the empty string, then return this's node document's URL.
  - 3. Let urlString be the result of encoding-parsing-and-serializing a URL given attribute's value, relative to this's node document.
  - 4. If urlString is not failure, then return urlString.
  - 5. Return attribute's value, converted to a scalar value string.

- [x] [HTMLButtonElement,HTMLInputElement.formAction](https://html.spec.whatwg.org/#dom-fs-formaction) — The formAction getter steps are:
  - 1. Let attribute be this's formaction attribute.
  - 2. If attribute is null or attribute's value is the empty string, then return this's node document's URL.
  - 3. Let urlString be the result of encoding-parsing-and-serializing a URL given attribute's value, relative to this's node document.
  - 4. If urlString is not failure, then return urlString.
  - 5. Return attribute's value, converted to a scalar value string.

- [x] [HTMLFormElement.method](https://html.spec.whatwg.org/#dom-fs-method) — The method and enctype IDL attributes must reflect the respective content attributes of the same name, limited to only known values. The encoding IDL attribute must reflect the enctype content attribute, limited to only known values. The formEnctype IDL attribute must reflect the formenctype content attribute, limited to only known values. The formMethod IDL attribute must reflect the formmethod content attribute, limited to only known values.

##### [4.10.19.7 Autofill](https://html.spec.whatwg.org/#autofill)

Status: reviewed (2026-11-01)
> Headings only.

###### [4.10.19.7.1 Autofilling form controls: the autocomplete attribute](https://html.spec.whatwg.org/#autofilling-form-controls:-the-autocomplete-attribute)

Status: reviewed (2026-11-01)
> Parsed by `AutofillData::Create`; there's no autofill (see `KNOWN-ISSUES.md`), so a control's data and inappropriate fields aren't used.
<!-- definitions: attr-fe-autocomplete autofill-expectation-mantle autofill-anchor-mantle attr-fe-autocomplete-section attr-fe-autocomplete-shipping attr-fe-autocomplete-billing attr-fe-autocomplete-home attr-fe-autocomplete-work attr-fe-autocomplete-mobile attr-fe-autocomplete-fax attr-fe-autocomplete-pager attr-fe-autocomplete-webauthn attr-fe-autocomplete-off attr-fe-autocomplete-on autofill-field control-group-text control-group-multiline control-group-password control-group-url control-group-username control-group-tel control-group-numeric control-group-month control-group-date -->

- [x] [Autofill detail tokens](https://html.spec.whatwg.org/#autofill-detail-tokens) — Autofill detail tokens are the following, in the order given below:
  - 1. Optionally, a token whose first eight characters are an ASCII case-insensitive match for the string "section-", meaning that the field belongs to the named group.
  - 2. Optionally, a token that is an ASCII case-insensitive match for one of the following strings: "shipping", meaning the field is part of the shipping address or contact information; "billing", meaning the field is part of the billing address or contact information
  - 3. Either of the following two options: A token that is an ASCII case-insensitive match for one of the following autofill field names, excluding those that are inappropriate for the control: "name"; "honorific-prefix"; "given-name"; "additional-name"; "family-name"; "honorific-suffix"; "nickname"; "username"; "new-password"; "current-password"; "one-time-code"; "organization-title"; "organization";…
    - 3a. Optionally, a token that is an ASCII case-insensitive match for one of the following strings: "home", meaning the field is for contacting someone at their residence; "work", meaning the field is for contacting someone at their workplace; "mobile", meaning the field is for contacting someone regardless of location; "fax", meaning the field describes a fax machine's contact details; "pager", meanin…
    - 3b. A token that is an ASCII case-insensitive match for one of the following autofill field names, excluding those that are inappropriate for the control: "tel"; "tel-country-code"; "tel-national"; "tel-area-code"; "tel-local"; "tel-local-prefix"; "tel-local-suffix"; "tel-extension"; "email"; "impp" (See the table below for descriptions of these values.)
  - 4. Optionally, a token that is an ASCII case-insensitive match for the string "webauthn", meaning the user agent should show public key credentials available via conditional mediation when the user interacts with the form control. webauthn is only valid for input and textarea elements.

- [x] [inappropriate for a control](https://html.spec.whatwg.org/#inappropriate-for-the-control) — Some fields are only appropriate for certain form controls. An autofill field name is inappropriate for a control if the control does not belong to the group listed for that autofill field in the fifth column of the first row describing that autofill field in the table below. What controls fall into each group is described below the table.

###### [4.10.19.7.2 Processing model](https://html.spec.whatwg.org/#autofill-processing-model)

Status: reviewed (2026-11-01)
> `AutofillData::Create` (`HTML/Autofill.cpp`).
> Fixed: `autocomplete` returned the attribute's value.
<!-- definitions: autofill-hint-set autofill-scope autofill-field-name non-autofill-credential-type concept-fe-autofill -->

- [x] [These values are defined as the result of running the following algorithm](https://html.spec.whatwg.org/#idl-exposed-autofill-value) — These values are defined as the result of running the following algorithm:
  - 1. If the element has no autocomplete attribute, then jump to the step labeled default.
  - 2. Let tokens be the result of splitting the attribute's value on ASCII whitespace.
  - 3. If tokens is empty, then jump to the step labeled default.
  - 4. Let index be the index of the last token in tokens.
  - 5. Let field be the indexth token in tokens.
  - 6. Let category be the result of determining a field's category given field.
  - 7. If category is null, then jump to the step labeled default.
  - 8. If category is Off or Automatic, and either tokens's size is greater than 1 or the element's autocomplete attribute is wearing the autofill anchor mantle, then jump to the step labeled default.
  - 9. If category is Off, set the element's autofill field name to the string "off", set its autofill hint set to empty, and set its IDL-exposed autofill value to the string "off". Then, return.
  - 10. If category is Automatic, set the element's autofill field name to the string "on", set its autofill hint set to empty, and set its IDL-exposed autofill value to the string "on". Then, return.
  - 11. Let scope tokens be an empty list.
  - 12. Let hint tokens be an empty set.
  - 13. Let credential type be null.
  - 14. Let IDL value have the same value as field.
  - 15. If category is Credential and the indexth token in tokens is an ASCII case-insensitive match for "webauthn", then run the substeps that follow:
    - 15a. Set credential type to "webauthn".
    - 15b. If the indexth token in tokens is the first entry, then skip to the step labeled done.
    - 15c. Decrement index by one.
    - 15d. Set category to the result of determining a field's category given the indexth token in tokens.
    - 15e. If category is not Normal and category is not Contact, then jump to the step labeled default.
    - 15f. Set IDL value to the concatenation of the indexth token in tokens, a U+0020 SPACE character, and the previous value of IDL value.
  - 16. If the indexth token in tokens is the first entry, then skip to the step labeled done.
  - 17. Decrement index by one.
  - 18. If category is Contact and the indexth token in tokens is an ASCII case-insensitive match for one of the strings in the following list: "home"; "work"; "mobile"; "fax"; "pager" then:
    - 18a. Let contact be the matching string from the list above.
    - 18b. Insert contact at the start of scope tokens.
    - 18c. Add contact to hint tokens.
    - 18d. Let IDL value be the concatenation of contact, a U+0020 SPACE character, and the previous value of IDL value.
    - 18e. If the indexth entry in tokens is the first entry, then skip to the step labeled done.
    - 18f. Decrement index by one.
  - 19. If the indexth token in tokens is an ASCII case-insensitive match for one of the strings in the following list: "shipping"; "billing" then:
    - 19a. Let mode be the matching string from the list above.
    - 19b. Insert mode at the start of scope tokens.
    - 19c. Add mode to hint tokens.
    - 19d. Let IDL value be the concatenation of mode, a U+0020 SPACE character, and the previous value of IDL value.
    - 19e. If the indexth entry in tokens is the first entry, then skip to the step labeled done.
    - 19f. Decrement index by one.
  - 20. If the indexth entry in tokens is not the first entry, then jump to the step labeled default.
  - 21. If the first eight characters of the indexth token in tokens are not an ASCII case-insensitive match for the string "section-", then jump to the step labeled default.
  - 22. Let section be the indexth token in tokens, converted to ASCII lowercase.
  - 23. Insert section at the start of scope tokens.
  - 24. Let IDL value be the concatenation of section, a U+0020 SPACE character, and the previous value of IDL value.
  - 25. Done: Set the element's autofill hint set to hint tokens.
  - 26. Set the element's non-autofill credential type to credential type.
  - 27. Set the element's autofill scope to scope tokens.
  - 28. Set the element's autofill field name to field.
  - 29. Set the element's IDL-exposed autofill value to IDL value.
  - 30. Return.
  - 31. Default: Set the element's IDL-exposed autofill value to the empty string, and its autofill hint set and autofill scope to empty.
  - 32. If the element's autocomplete attribute is wearing the autofill anchor mantle, then set the element's autofill field name to the empty string and return.
  - 33. Let form be the element's form owner, if any, or null otherwise.
  - 34. If form is not null and form's autocomplete attribute is in the Off state, then set the element's autofill field name to "off". Otherwise, set the element's autofill field name to "on".

- [x] [determine a field's category](https://html.spec.whatwg.org/#determine-a-field's-category) — To determine a field's category, given field:
  - 1. If the field is not an ASCII case-insensitive match for one of the tokens given in the first column of the following table, then return null.
  - 2. Return the value of the cell in the second column of that row.

- [x] [control's data](https://html.spec.whatwg.org/#control's-data) — For the purposes of autofill, a control's data depends on the kind of control:
  - • An input element with its type attribute in the Email state and with the multiple attribute specified: The element's values.
  - • Any other input element / A textarea element: The element's value.
  - • A select element with its multiple attribute specified: The option elements in the select element's list of options that have their selectedness set to true.
  - • Any other select element: The option element in the select element's list of options that has its selectedness set to true.

- [x] [HTMLInputElement,HTMLSelectElement,HTMLTextAreaElement.autocomplete](https://html.spec.whatwg.org/#dom-fe-autocomplete) — The autocomplete IDL attribute, on getting, must return the element's IDL-exposed autofill value.

#### [4.10.20 APIs for the text control selections](https://html.spec.whatwg.org/#textFieldSelection)

Status: reviewed (2026-11-01)
> `HTMLTextFormControlElement` and the input's `ForBindings` members. Offsets are UTF-8 (see `DIFFERENCES.md`), `select()` focuses, and "none" is "forward" (the platform has no selection without a direction).
<!-- definitions: concept-textarea/input-selection concept-textarea/input-cursor selection-direction dom-selectionmode-select dom-selectionmode-start dom-selectionmode-end dom-selectionmode-preserve -->

- [x] [Whenever the relevant value changes for an element to which these APIs apply, r…](https://html.spec.whatwg.org/#concept-textarea/input-relevant-value) — Whenever the relevant value changes for an element to which these APIs apply, run these steps:
  - 1. If the element has a selection:
    - 1a. If the start of the selection is now past the end of the relevant value, set it to the end of the relevant value.
    - 1b. If the end of the selection is now past the end of the relevant value, set it to the end of the relevant value.
    - 1c. If the user agent does not support empty selection, and both the start and end of the selection are now pointing to the end of the relevant value, then instead set the element's text entry cursor position to the end of the relevant value, removing any selection.
  - 2. Otherwise, the element must have a text entry cursor position position. If it is now past the end of the relevant value, set it to the end of the relevant value.

- [x] [set the selection direction](https://html.spec.whatwg.org/#set-the-selection-direction) — To set the selection direction of an element to a given direction, update the element's selection direction to the given direction, unless the direction is "none" and the platform does not support that direction; in that case, update the element's selection direction to "forward".

- [x] [HTMLInputElement,HTMLTextAreaElement.select()](https://html.spec.whatwg.org/#dom-textarea/input-select) — The select() method, when invoked, must run the following steps:
  - 1. If this element is an input element, and either select() does not apply to this element or the corresponding control has no selectable text, return.
  - 2. Set the selection range with 0 and infinity.

- [x] [HTMLInputElement,HTMLTextAreaElement.selectionStart](https://html.spec.whatwg.org/#dom-textarea/input-selectionstart) — The selectionStart attribute's getter must run the following steps:
  - 1. If this element is an input element, and selectionStart does not apply to this element, return null.
  - 2. If there is no selection, return the code unit offset within the relevant value to the character that immediately follows the text entry cursor.
  - 3. Return the code unit offset within the relevant value to the character that immediately follows the start of the selection.

- [x] [The selectionStart attribute's setter must run the following steps](https://html.spec.whatwg.org/#dom-textarea/input-selectionstart) — The selectionStart attribute's setter must run the following steps:
  - 1. If this element is an input element, and selectionStart does not apply to this element, throw an "InvalidStateError" DOMException.
  - 2. Let end be the value of this element's selectionEnd attribute.
  - 3. If end is less than the given value, set end to the given value.
  - 4. Set the selection range with the given value, end, and the value of this element's selectionDirection attribute.

- [x] [HTMLInputElement,HTMLTextAreaElement.selectionEnd](https://html.spec.whatwg.org/#dom-textarea/input-selectionend) — The selectionEnd attribute's getter must run the following steps:
  - 1. If this element is an input element, and selectionEnd does not apply to this element, return null.
  - 2. If there is no selection, return the code unit offset within the relevant value to the character that immediately follows the text entry cursor.
  - 3. Return the code unit offset within the relevant value to the character that immediately follows the end of the selection.

- [x] [The selectionEnd attribute's setter must run the following steps](https://html.spec.whatwg.org/#dom-textarea/input-selectionend) — The selectionEnd attribute's setter must run the following steps:
  - 1. If this element is an input element, and selectionEnd does not apply to this element, throw an "InvalidStateError" DOMException.
  - 2. Set the selection range with the value of this element's selectionStart attribute, the given value, and the value of this element's selectionDirection attribute.

- [x] [HTMLInputElement,HTMLTextAreaElement.selectionDirection](https://html.spec.whatwg.org/#dom-textarea/input-selectiondirection) — The selectionDirection attribute's getter must run the following steps:
  - 1. If this element is an input element, and selectionDirection does not apply to this element, return null.
  - 2. Return this element's selection direction.

- [x] [The selectionDirection attribute's setter must run the following steps](https://html.spec.whatwg.org/#dom-textarea/input-selectiondirection) — The selectionDirection attribute's setter must run the following steps:
  - 1. If this element is an input element, and selectionDirection does not apply to this element, throw an "InvalidStateError" DOMException.
  - 2. Set the selection range with the value of this element's selectionStart attribute, the value of this element's selectionEnd attribute, and the given value.

- [x] [HTMLInputElement,HTMLTextAreaElement.setSelectionRange(start, end, direction)](https://html.spec.whatwg.org/#dom-textarea/input-setselectionrange) — The setSelectionRange(start, end, direction) method, when invoked, must run the following steps:
  - 1. If this element is an input element, and setSelectionRange() does not apply to this element, throw an "InvalidStateError" DOMException.
  - 2. Set the selection range with start, end, and direction.

- [x] [set the selection range](https://html.spec.whatwg.org/#set-the-selection-range) — To set the selection range with an integer or null start, an integer or null or the special value infinity end, and optionally a string direction, run the following steps:
  - 1. If start is null, let start be 0.
  - 2. If end is null, let end be 0.
  - 3. Set the selection of the text control to the sequence of code units within the relevant value starting with the code unit at the startth position (in logical order) and ending with the code unit at the (end-1)th position. Arguments greater than the length of the relevant value of the text control (including the special value infinity) must be treated as pointing at the end of the text control. If…
  - 4. If direction is not identical to either "backward" or "forward", or if the direction argument was not given, set direction to "none".
  - 5. Set the selection direction of the text control to direction.
  - 6. If the previous steps caused the selection of the text control to be modified (in either extent or direction), then queue an element task on the user interaction task source given the element to fire an event named select at the element, with the bubbles attribute initialized to true.

- [x] [HTMLInputElement,HTMLTextAreaElement.setRangeText(replacement, start, end, selectMode)](https://html.spec.whatwg.org/#dom-textarea/input-setrangetext) — The setRangeText(replacement, start, end, selectMode) method, when invoked, must run the following steps:
  - 1. If this element is an input element, and setRangeText() does not apply to this element, throw an "InvalidStateError" DOMException.
  - 2. Set this element's dirty value flag to true.
  - 3. If the method has only one argument, then let start and end have the values of the selectionStart attribute and the selectionEnd attribute respectively. Otherwise, let start, end have the values of the second and third arguments respectively.
  - 4. If start is greater than end, then throw an "IndexSizeError" DOMException.
  - 5. If start is greater than the length of the relevant value of the text control, then set it to the length of the relevant value of the text control.
  - 6. If end is greater than the length of the relevant value of the text control, then set it to the length of the relevant value of the text control.
  - 7. Let selection start be the current value of the selectionStart attribute.
  - 8. Let selection end be the current value of the selectionEnd attribute.
  - 9. If start is less than end, delete the sequence of code units within the element's relevant value starting with the code unit at the startth position and ending with the code unit at the (end-1)th position.
  - 10. Insert the value of the first argument into the text of the relevant value of the text control, immediately before the startth code unit.
  - 11. Let new length be the length of the value of the first argument.
  - 12. Let new end be the sum of start and new length.
  - 13. Run the appropriate set of substeps from the following list: If the fourth argument's value is "select" Let selection start be start. Let selection end be new end. If the fourth argument's value is "start" Let selection start and selection end be start. If the fourth argument's value is "end" Let selection start and selection end be new end. If the fourth argument's value is "preserve"If the meth…
    - 13a. Let old length be end minus start.
    - 13b. Let delta be new length minus old length.
    - 13c. If selection start is greater than end, then increment it by delta. (If delta is negative, i.e. the new text is shorter than the old text, then this will decrease the value of selection start.) Otherwise: if selection start is greater than start, then set it to start. (This snaps the start of the selection to the start of the new text if it was in the middle of the text that it replaced.)
    - 13d. If selection end is greater than end, then increment it by delta in the same way. Otherwise: if selection end is greater than start, then set it to new end. (This snaps the end of the selection to the end of the new text if it was in the middle of the text that it replaced.)
  - 14. Set the selection range with selection start and selection end.

#### [4.10.21 Constraints](https://html.spec.whatwg.org/#constraints)

Status: reviewed (2026-11-01)
> Headings only.

##### [4.10.21.1 Definitions](https://html.spec.whatwg.org/#definitions)

Status: reviewed (2026-11-01)
> `ValidatedFormListedElement` (`WillValidate`, `IsValidFormControlElement`) and each control's validity states.
<!-- definitions: barred-from-constraint-validation custom-validity-error-message validity-states suffering-from-being-missing suffering-from-a-type-mismatch suffering-from-a-pattern-mismatch suffering-from-being-too-long suffering-from-being-too-short suffering-from-an-underflow suffering-from-an-overflow suffering-from-a-step-mismatch suffering-from-bad-input suffering-from-a-custom-error -->

- [x] [candidate for constraint validation](https://html.spec.whatwg.org/#candidate-for-constraint-validation) — A submittable element is a candidate for constraint validation except when a condition has barred the element from constraint validation. (For example, an element is barred from constraint validation if it has a datalist element ancestor.)

- [x] [satisfies its constraints](https://html.spec.whatwg.org/#concept-fv-valid) — An element satisfies its constraints if it is not suffering from any of the above validity states.

##### [4.10.21.2 Constraint validation](https://html.spec.whatwg.org/#constraint-validation)

Status: reviewed (2026-11-01)
> `HTMLFormElement::CheckInvalidControlsAndCollectUnhandled` and `ValidateInteractively`.

- [x] [statically validate the constraints](https://html.spec.whatwg.org/#statically-validate-the-constraints) — When the user agent is required to statically validate the constraints of form element form, it must run the following steps, which return either a positive result (all the controls in the form are valid) or a negative result (there are invalid controls) along with a (possibly empty) list of elemen…
  - 1. Let controls be a list of all the submittable elements whose form owner is form, in tree order.
  - 2. Let invalid controls be an initially empty list of elements.
  - 3. For each element field in controls, in tree order:
    - 3a. If field is not a candidate for constraint validation, then move on to the next element.
    - 3b. Otherwise, if field satisfies its constraints, then move on to the next element.
    - 3c. Otherwise, add field to invalid controls.
  - 4. If invalid controls is empty, then return a positive result.
  - 5. Let unhandled invalid controls be an initially empty list of elements.
  - 6. For each element field in invalid controls, if any, in tree order:
    - 6a. Let notCanceled be the result of firing an event named invalid at field, with the cancelable attribute initialized to true.
    - 6b. If notCanceled is true, then add field to unhandled invalid controls.
  - 7. Return a negative result with the list of elements in the unhandled invalid controls list.

- [x] [interactively validate the constraints](https://html.spec.whatwg.org/#interactively-validate-the-constraints) — If a user agent is to interactively validate the constraints of form element form, then the user agent must run the following steps:
  - 1. Statically validate the constraints of form, and let unhandled invalid controls be the list of elements returned if the result was negative.
  - 2. If the result was positive, then return that result.
  - 3. Report the problems with the constraints of at least one of the elements given in unhandled invalid controls to the user. User agents may focus one of those elements in the process, by running the focusing steps for that element, and may change the scrolling position of the document, or perform some other action that brings the element to the user's attention. For elements that are form-associate…
  - 4. Return a negative result.

##### [4.10.21.3 The constraint validation API](https://html.spec.whatwg.org/#the-constraint-validation-api)

Status: reviewed (2026-11-01)
> `ValidatedFormListedElement`, `ValidityState` and `ElementInternals`.
> Fixed: `setCustomValidity()` didn't normalize newlines.
<!-- definitions: dom-validitystate-valuemissing dom-validitystate-typemismatch dom-validitystate-patternmismatch dom-validitystate-toolong dom-validitystate-tooshort dom-validitystate-rangeunderflow dom-validitystate-rangeoverflow dom-validitystate-stepmismatch dom-validitystate-badinput dom-validitystate-customerror dom-validitystate-valid -->

- [x] [HTMLObjectElement,HTMLInputElement,HTMLButtonElement,HTMLSelectElement,HTMLTextAreaElement,HTMLOutputElement,HTMLFieldSetElement.willValidate](https://html.spec.whatwg.org/#dom-cva-willvalidate) — The willValidate attribute's getter must return true, if this element is a candidate for constraint validation, and false otherwise (i.e., false if any conditions are barring it from constraint validation).

- [x] [ElementInternals.willValidate](https://html.spec.whatwg.org/#dom-elementinternals-willvalidate) — The willValidate attribute of ElementInternals interface, on getting, must throw a "NotSupportedError" DOMException if the target element is not a form-associated custom element. Otherwise, it must return true if the target element is a candidate for constraint validation, and false otherwise.

- [x] [HTMLObjectElement,HTMLInputElement,HTMLButtonElement,HTMLSelectElement,HTMLTextAreaElement,HTMLOutputElement,HTMLFieldSetElement.setCustomValidity(error)](https://html.spec.whatwg.org/#dom-cva-setcustomvalidity) — The setCustomValidity(error) method steps are:
  - 1. Set error to the result of normalizing newlines given error.
  - 2. Set the custom validity error message to error.

- [x] [HTMLObjectElement,HTMLInputElement,HTMLButtonElement,HTMLSelectElement,HTMLTextAreaElement,HTMLOutputElement,HTMLFieldSetElement.validity](https://html.spec.whatwg.org/#dom-cva-validity) — The validity attribute's getter must return a ValidityState object that represents the validity states of this element. This object is live.

- [x] [ElementInternals.validity](https://html.spec.whatwg.org/#dom-elementinternals-validity) — The validity attribute of ElementInternals interface, on getting, must throw a "NotSupportedError" DOMException if the target element is not a form-associated custom element. Otherwise, it must return a ValidityState object that represents the validity states of the target element. This object is live.

- [x] [check validity steps](https://html.spec.whatwg.org/#check-validity-steps) — The check validity steps for an element element are:
  - 1. If element is a candidate for constraint validation and does not satisfy its constraints:
    - 1a. Fire an event named invalid at element, with the cancelable attribute initialized to true (though canceling has no effect).
    - 1b. Return false.
  - 2. Return true.

- [x] [HTMLObjectElement,HTMLInputElement,HTMLButtonElement,HTMLSelectElement,HTMLTextAreaElement,HTMLOutputElement,HTMLFieldSetElement.checkValidity()](https://html.spec.whatwg.org/#dom-cva-checkvalidity) — The checkValidity() method, when invoked, must run the check validity steps on this element.

- [x] [ElementInternals.checkValidity()](https://html.spec.whatwg.org/#dom-elementinternals-checkvalidity) — The checkValidity() method of the ElementInternals interface must run these steps:
  - 1. Let element be this ElementInternals's target element.
  - 2. If element is not a form-associated custom element, then throw a "NotSupportedError" DOMException.
  - 3. Run the check validity steps on element.

- [x] [report validity steps](https://html.spec.whatwg.org/#report-validity-steps) — The report validity steps for an element element are:
  - 1. If element is a candidate for constraint validation and does not satisfy its constraints:
    - 1a. Let report be the result of firing an event named invalid at element, with the cancelable attribute initialized to true.
    - 1b. If report is true, then report the problems with the constraints of this element to the user. When reporting the problem with the constraints to the user, the user agent may run the focusing steps for element, and may change the scrolling position of the document, or perform some other action that brings element to the user's attention. User agents may report more than one constraint violation, i…
    - 1c. Return false.
  - 2. Return true.

- [x] [HTMLObjectElement,HTMLInputElement,HTMLButtonElement,HTMLSelectElement,HTMLTextAreaElement,HTMLOutputElement,HTMLFieldSetElement.reportValidity()](https://html.spec.whatwg.org/#dom-cva-reportvalidity) — The reportValidity() method, when invoked, must run the report validity steps on this element.

- [x] [ElementInternals.reportValidity()](https://html.spec.whatwg.org/#dom-elementinternals-reportvalidity) — The reportValidity() method of the ElementInternals interface must run these steps:
  - 1. Let element be this ElementInternals's target element.
  - 2. If element is not a form-associated custom element, then throw a "NotSupportedError" DOMException.
  - 3. Run the report validity steps on element.

- [x] [HTMLObjectElement,HTMLInputElement,HTMLButtonElement,HTMLSelectElement,HTMLTextAreaElement,HTMLOutputElement,HTMLFieldSetElement.validationMessage](https://html.spec.whatwg.org/#dom-cva-validationmessage) — The validationMessage attribute's getter must run these steps:
  - 1. If this element is not a candidate for constraint validation or if this element satisfies its constraints, then return the empty string.
  - 2. Return a suitably localized message that the user agent would show the user if this were the only form control with a validity constraint problem. If the user agent would not actually show a textual message in such a situation (e.g., it would show a graphical cue instead), then return a suitably localized message that expresses (one or more of) the validity constraint(s) that the control does not…

##### [4.10.21.4 Security](https://html.spec.whatwg.org/#security-forms)

Status: n/a (no algorithms; advice)

#### [4.10.22 Form submission](https://html.spec.whatwg.org/#form-submission-2)

Status: reviewed (2026-11-01)
> Headings only.

##### [4.10.22.1 Introduction](https://html.spec.whatwg.org/#introduction-5)

Status: n/a (introduction)

##### [4.10.22.2 Implicit submission](https://html.spec.whatwg.org/#implicit-submission)

Status: reviewed (2026-11-01)
> `HTMLFormElement::SubmitImplicitly` and `DefaultButton`; each type's `CanTriggerImplicitSubmission`.

- [x] [default button](https://html.spec.whatwg.org/#default-button) — A form element's default button is the first submit button in tree order whose form owner is that form element.

- [x] [If the form has no submit button, then the implicit submission mechanism must p…](https://html.spec.whatwg.org/#default-button) — If the form has no submit button, then the implicit submission mechanism must perform the following steps:
  - 1. If the form has more than one field that blocks implicit submission, then return.
  - 2. Submit the form element from the form element itself with userInvolvement set to "activation".

- [x] [field that blocks implicit submission](https://html.spec.whatwg.org/#field-that-blocks-implicit-submission) — For the purpose of the previous paragraph, an element is a field that blocks implicit submission of a form element if it is an input element whose form owner is that form element and whose type attribute is in one of the following states: Text, Search, Telephone, URL, Email, Password, Date, Month, Week, Time, Local Date and Time, Number

##### [4.10.22.3 Form submission algorithm](https://html.spec.whatwg.org/#form-submission-algorithm)

Status: reviewed (2026-11-01)
> `HTMLFormElement::SubmitIfPossible` and `Submit`, `FormSubmission::Create`, and `FrameLoader::SubmitForm`; the dialog method is `SubmitDialog`. The rel attribute's noopener and noreferrer are FIXMEs.
<!-- definitions: constructing-entry-list firing-submission-events submit-submitted-from-method submit-user-involvement planned-navigation plan-to-navigate submit-mutate-action submit-body submit-get-action submit-mailto-headers submit-mailto-body -->

- [x] [submit](https://html.spec.whatwg.org/#concept-form-submit) — To submit a form element form from an element submitter (typically a button), given an optional boolean submitted from submit() method (default false) and an optional user navigation involvement userInvolvement (default "none"):
  - 1. If form cannot navigate, then return.
  - 2. If form's constructing entry list is true, then return.
  - 3. Let form document be form's node document.
  - 4. If form document's active sandboxing flag set has its sandboxed forms browsing context flag set, then return.
  - 5. If submitted from submit() method is false:
    - 5a. If form's firing submission events is true, then return.
    - 5b. Set form's firing submission events to true.
    - 5c. For each element field in the list of submittable elements whose form owner is form, set field's user validity to true.
    - 5d. If the submitter element's no-validate state is false, then interactively validate the constraints of form and examine the result. If the result is negative (i.e., the constraint validation concluded that there were invalid fields and probably informed the user of this):
      - 5d.i. Set form's firing submission events to false.
      - 5d.ii. Return.
    - 5e. Let submitterButton be null if submitter is form. Otherwise, let submitterButton be submitter.
    - 5f. Let shouldContinue be the result of firing an event named submit at form using SubmitEvent, with the submitter attribute initialized to submitterButton, the bubbles attribute initialized to true, and the cancelable attribute initialized to true.
    - 5g. Set form's firing submission events to false.
    - 5h. If shouldContinue is false, then return.
    - 5i. If form cannot navigate, then return.
  - 6. Let encoding be the result of picking an encoding for the form.
  - 7. Let entry list be the result of constructing the entry list with form, submitter, and encoding.
  - 8. Assert: entry list is not null.
  - 9. If form cannot navigate, then return.
  - 10. Let method be the submitter element's method.
  - 11. If method is dialog:
    - 11a. If form does not have an ancestor dialog element, then return.
    - 11b. Let subject be form's nearest ancestor dialog element.
    - 11c. Let result be null.
    - 11d. If submitter is an input element whose type attribute is in the Image Button state:
      - 11d.i. Let (x, y) be the selected coordinate.
      - 11d.ii. Set result to the concatenation of x, ",", and y.
    - 11e. Otherwise, if submitter is a submit button, then set result to submitter's optional value.
    - 11f. Close the dialog subject with result and null.
    - 11g. Return.
  - 12. Let action be the submitter element's action.
  - 13. If action is the empty string, let action be the URL of the form document.
  - 14. Let parsed action be the result of encoding-parsing a URL given action, relative to submitter's node document.
  - 15. If parsed action is failure, then return.
  - 16. Let scheme be the scheme of parsed action.
  - 17. Let enctype be the submitter element's enctype.
  - 18. Let formTarget be null.
  - 19. If the submitter element is a submit button and it has a formtarget attribute, then set formTarget to the formtarget attribute value.
  - 20. Let target be the result of getting an element's target given submitter's form owner and formTarget.
  - 21. Let noopener be the result of getting an element's noopener with form, parsed action, and target.
  - 22. Let targetNavigable be the first return value of applying the rules for choosing a navigable given target, form's node navigable, and noopener.
  - 23. If targetNavigable is null, then return.
  - 24. Let historyHandling be "auto".
  - 25. If form document equals targetNavigable's active document, and form document has not yet completely loaded, then set historyHandling to "replace".
  - 26. Select the appropriate row in the table below based on scheme as given by the first cell of each row. Then, select the appropriate cell on that row based on method as given in the first cell of each column. Then, jump to the steps named in that cell and defined below the table. If scheme is not one of those listed in this table, then the behavior is not defined by this specification. User agents…
    - 26a. Let referrerPolicy be the empty string.
    - 26b. If the form element's link types include the noreferrer keyword, then set referrerPolicy to "no-referrer".
    - 26c. If the form has a non-null planned navigation, remove it from its task queue.
    - 26d. Queue an element task on the DOM manipulation task source given the form element and the following steps:
      - 26d.i. Set the form's planned navigation to null.
      - 26d.ii. Navigate targetNavigable to url using the form element's node document, with historyHandling set to historyHandling, userInvolvement set to userInvolvement, sourceElement set to submitter, referrerPolicy set to referrerPolicy, documentResource set to postResource, and formDataEntryList set to entry list.
    - 26e. Set the form's planned navigation to the just-queued task.

##### [4.10.22.4 Constructing the entry list](https://html.spec.whatwg.org/#constructing-form-data-set)

Status: reviewed (2026-11-01)
> `HTMLFormElement::ConstructEntryList` and each control's `AppendFormData`; entries are `DOMFormData`'s.
> Fixed: controls in a datalist were submitted.
<!-- definitions: entry-list form-entry form-entry-name form-entry-value -->

- [x] [create an entry (entry list)](https://html.spec.whatwg.org/#create-an-entry) — To create an entry given a string name, a string or Blob object value, and optionally a scalar value string filename:
  - 1. Set name to the result of converting name into a scalar value string.
  - 2. If value is a string, then set value to the result of converting value into a scalar value string.
  - 3. Otherwise:
    - 3a. If value is not a File object, then set value to a new File object, representing the same bytes, whose name attribute value is "blob".
    - 3b. If filename is given, then set value to a new File object, representing the same bytes, whose name attribute is filename.
  - 4. Return an entry whose name is name and whose value is value.

- [x] [construct the entry list](https://html.spec.whatwg.org/#constructing-the-form-data-set) — To construct the entry list given a form, an optional submitter (default null), and an optional encoding (default UTF-8):
  - 1. If form's constructing entry list is true, then return null.
  - 2. Set form's constructing entry list to true.
  - 3. Let controls be a list of all the submittable elements whose form owner is form, in tree order.
  - 4. Let entry list be a new empty entry list.
  - 5. For each element field in controls, in tree order:
    - 5a. If any of the following are true: field has a datalist element ancestor;; field is disabled;; field is a button but it is not submitter;; field is an input element whose type attribute is in the Checkbox state and whose checkedness is false; or; field is an input element whose type attribute is in the Radio Button state and whose checkedness is false, then continue.
    - 5b. If the field element is an input element whose type attribute is in the Image Button state:
      - 5b.i. If the field element is not submitter, then continue.
      - 5b.ii. If the field element has a name attribute specified and its value is not the empty string, let name be that value followed by U+002E (.). Otherwise, let name be the empty string.
      - 5b.iii. Let namex be the concatenation of name and U+0078 (x).
      - 5b.iv. Let namey be the concatenation of name and U+0079 (y).
      - 5b.v. Let (x, y) be the selected coordinate.
      - 5b.vi. Create an entry with namex and x, and append it to entry list.
      - 5b.vii. Create an entry with namey and y, and append it to entry list.
      - 5b.viii. Continue.
    - 5c. If the field is a form-associated custom element, then perform the entry construction algorithm given field and entry list, then continue.
    - 5d. If either the field element does not have a name attribute specified, or its name attribute's value is the empty string, then continue.
    - 5e. Let name be the value of the field element's name attribute.
    - 5f. If the field element is a select element, then for each option element in the select element's list of options whose selectedness is true and that is not disabled, create an entry with name and the value of the option element, and append it to entry list.
    - 5g. Otherwise, if the field element is an input element whose type attribute is in the Checkbox state or the Radio Button state:
      - 5g.i. If the field element has a value attribute specified, then let value be the value of that attribute; otherwise, let value be the string "on".
      - 5g.ii. Create an entry with name and value, and append it to entry list.
    - 5h. Otherwise, if the field element is an input element whose type attribute is in the File Upload state:
      - 5h.i. If there are no selected files, then create an entry with name and a new File object with an empty name, application/octet-stream as type, and an empty body, and append it to entry list.
      - 5h.ii. Otherwise, for each file in selected files, create an entry with name and a File object representing the file, and append it to entry list.
    - 5i. Otherwise, if the field element is an input element whose type attribute is in the Hidden state and name is an ASCII case-insensitive match for "_charset_":
      - 5i.i. Let charset be the name of encoding.
      - 5i.ii. Create an entry with name and charset, and append it to entry list.
    - 5j. Otherwise, create an entry with name and the value of the field element, and append it to entry list.
    - 5k. If the element has a dirname attribute, that attribute's value is not the empty string, and the element is an auto-directionality form-associated element:
      - 5k.i. Let dirname be the value of the element's dirname attribute.
      - 5k.ii. Let dir be the string "ltr" if the directionality of the element is 'ltr', and "rtl" otherwise (i.e., when the directionality of the element is 'rtl').
      - 5k.iii. Create an entry with dirname and dir, and append it to entry list.
  - 6. Let form data be a new FormData object associated with entry list.
  - 7. Fire an event named formdata at form using FormDataEvent, with the formData attribute initialized to form data and the bubbles attribute initialized to true.
  - 8. Set form's constructing entry list to false.
  - 9. Return a clone of entry list.

##### [4.10.22.5 Selecting a form submission encoding](https://html.spec.whatwg.org/#selecting-a-form-submission-encoding)

Status: reviewed (2026-11-01)
> Always UTF-8 (FIXME in `FormSubmission.cpp`, see `KNOWN-ISSUES.md`).

- [x] [pick an encoding for a form](https://html.spec.whatwg.org/#picking-an-encoding-for-the-form) — If the user agent is to pick an encoding for a form, it must run the following steps:
  - 1. Let encoding be the document's character encoding.
  - 2. If the form element has an accept-charset attribute, set encoding to the return value of running these substeps:
    - 2a. Let input be the value of the form element's accept-charset attribute.
    - 2b. Let candidate encoding labels be the result of splitting input on ASCII whitespace.
    - 2c. Let candidate encodings be an empty list of character encodings.
    - 2d. For each token in candidate encoding labels in turn (in the order in which they were found in input), get an encoding for the token and, if this does not result in failure, append the encoding to candidate encodings.
    - 2e. If candidate encodings is empty, return UTF-8.
    - 2f. Return the first encoding in candidate encodings.
  - 3. Return the result of getting an output encoding from encoding.

##### [4.10.22.6 Converting an entry list to a list of name-value pairs](https://html.spec.whatwg.org/#converting-an-entry-list-to-a-list-of-name-value-pairs)

Status: reviewed (2026-11-01)
> `FormData` (`Platform/Network/FormData.cpp`).

- [x] [convert to a list of name-value pairs](https://html.spec.whatwg.org/#convert-to-a-list-of-name-value-pairs) — To convert to a list of name-value pairs an entry list entry list, run these steps:
  - 1. Let list be an empty list of name-value pairs.
  - 2. For each entry of entry list:
    - 2a. Let name be entry's name, with every occurrence of U+000D (CR) not followed by U+000A (LF), and every occurrence of U+000A (LF) not preceded by U+000D (CR), replaced by a string consisting of U+000D (CR) and U+000A (LF).
    - 2b. If entry's value is a File object, then let value be entry's value's name. Otherwise, let value be entry's value.
    - 2c. Replace every occurrence of U+000D (CR) not followed by U+000A (LF), and every occurrence of U+000A (LF) not preceded by U+000D (CR), in value, by a string consisting of U+000D (CR) and U+000A (LF).
    - 2d. Append to list a new name-value pair whose name is name and whose value is value.
  - 3. Return list.

##### [4.10.22.7 URL-encoded form data](https://html.spec.whatwg.org/#url-encoded-form-data)

Status: reviewed (2026-11-01)
> `FormDataBuilder::EncodeStringAsFormData`.

##### [4.10.22.8 Multipart form data](https://html.spec.whatwg.org/#multipart-form-data)

Status: reviewed (2026-11-01)
> `FormData::CreateMultiPart` and `FormDataBuilder`.
<!-- definitions: multipart/form-data-boundary-string -->

- [x] [multipart/form-data encoding algorithm](https://html.spec.whatwg.org/#multipart/form-data-encoding-algorithm) — The multipart/form-data encoding algorithm, given an entry list entry list and an encoding encoding, is as follows:
  - 1. For each entry of entry list:
    - 1a. Replace every occurrence of U+000D (CR) not followed by U+000A (LF), and every occurrence of U+000A (LF) not preceded by U+000D (CR), in entry's name, by a string consisting of a U+000D (CR) and U+000A (LF).
    - 1b. If entry's value is not a File object, then replace every occurrence of U+000D (CR) not followed by U+000A (LF), and every occurrence of U+000A (LF) not preceded by U+000D (CR), in entry's value, by a string consisting of a U+000D (CR) and U+000A (LF).
  - 2. Return the byte sequence resulting from encoding the entry list using the rules described by RFC 7578, Returning Values from Forms: multipart/form-data, given the following conditions: [RFC7578] Each entry in entry list is a field, the name of the entry is the field name and the value of the entry is the field value.; The order of parts must be the same as the order of fields in entry list. Multi…

##### [4.10.22.9 Plain text form data](https://html.spec.whatwg.org/#plain-text-form-data)

Status: reviewed (2026-11-01)
> `FormData`'s text/plain encoding.

- [x] [text/plain encoding algorithm](https://html.spec.whatwg.org/#text/plain-encoding-algorithm) — The text/plain encoding algorithm, given a list of name-value pairs pairs, is as follows:
  - 1. Let result be the empty string.
  - 2. For each pair in pairs:
    - 2a. Append pair's name to result.
    - 2b. Append a single U+003D EQUALS SIGN character (=) to result.
    - 2c. Append pair's value to result.
    - 2d. Append a U+000D CARRIAGE RETURN (CR) U+000A LINE FEED (LF) character pair to result.
  - 3. Return result.

##### [4.10.22.10 The SubmitEvent interface](https://html.spec.whatwg.org/#the-submitevent-interface)

Status: reviewed (2026-11-01)
> `SubmitEvent`.

- [x] [SubmitEvent.submitter](https://html.spec.whatwg.org/#dom-submitevent-submitter) — The submitter attribute must return the value it was initialized to.

##### [4.10.22.11 The FormDataEvent interface](https://html.spec.whatwg.org/#the-formdataevent-interface)

Status: reviewed (2026-11-01)
> `dom::FormDataEvent`.

- [x] [FormDataEvent.formData](https://html.spec.whatwg.org/#dom-formdataevent-formdata) — The formData attribute must return the value it was initialized to. It represents a FormData object associated to the entry list that is constructed when the form is submitted.

#### [4.10.23 Resetting a form](https://html.spec.whatwg.org/#resetting-a-form)

Status: reviewed (2026-11-01)
> `HTMLFormElement::Reset` and each control's `Reset`.
<!-- definitions: concept-form-reset-control -->

- [x] [reset](https://html.spec.whatwg.org/#concept-form-reset) — When a form element form is reset, run these steps:
  - 1. Let reset be the result of firing an event named reset at form, with the bubbles and cancelable attributes initialized to true.
  - 2. If reset is true, then invoke the reset algorithm of each resettable element whose form owner is form.
