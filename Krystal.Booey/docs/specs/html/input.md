# HTML review checklist: 4.10.5 The input element

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 45 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html input` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

#### [4.10.5 The input element](https://html.spec.whatwg.org/#the-input-element)

Status: reviewed (2026-11-01)
> `HTMLInputElement`; each state is an `InputType`. The legacy-pre-activation and legacy-canceled-activation behaviours are the checkbox and radio types' `WillDispatchClick`/`DidDispatchClick`; the type change steps are `UpdateType`.
> `width`/`height` are the input type's (image buttons only).
<!-- definitions: attr-input-type concept-input-apply do-not-apply value-sanitization-algorithm concept-input-value-string-number concept-input-value-number-string concept-input-value-string-date concept-input-value-date-string concept-input-checked-dirty signal-a-type-change concept-input-indeterminate dom-input-type dom-input-height -->

- [x] [value (input)](https://html.spec.whatwg.org/#attr-input-value) — The value content attribute gives the default value of the input element. When the value content attribute is added, set, or removed, if the control's dirty value flag is false, the user agent must set the value of the element to the value of the value content attribute, if there is one, or the empty string otherwise, and then run the current value sanitization algorithm, if one is defined.

- [x] [dirty checkedness flag](https://html.spec.whatwg.org/#concept-input-checked-dirty-flag) — Each input element has a boolean dirty checkedness flag. When it is true, the element is said to have a dirty checkedness. The dirty checkedness flag must be initially set to false when the element is created, and must be set to true whenever the user interacts with the control in a way that changes the checkedness.

- [x] [checked (input)](https://html.spec.whatwg.org/#attr-input-checked) — The checked content attribute is a boolean attribute that gives the default checkedness of the input element. When the checked content attribute is added, if the control does not have dirty checkedness, the user agent must set the checkedness of the element to true; when the checked content attribute is removed, if the control does not have dirty checkedness, the user agent must set the checkedness of the element to false.

- [x] [The activation behavior for input elements element, given event, are these steps](https://html.spec.whatwg.org/#input-support-picker) — The activation behavior for input elements element, given event, are these steps:
  - 1. If element is not mutable, and element's type attribute is neither in the Checkbox nor in the Radio state, then return.
  - 2. Run element's input activation behavior, if any, and do nothing otherwise.
  - 3. If element has a form owner and element's type attribute is not in the Button state, then return.
  - 4. Run the popover target attribute activation behavior given element and event's target.

- [x] [The legacy-pre-activation behavior for input elements are these steps](https://html.spec.whatwg.org/#input-activation-behavior) — The legacy-pre-activation behavior for input elements are these steps:
  - 1. If this element's type attribute is in the Checkbox state, then set this element's checkedness to its opposite value (i.e. true if it is false, false if it is true) and set this element's indeterminateness to false.
  - 2. If this element's type attribute is in the Radio Button state, then get a reference to the element in this element's radio button group that has its checkedness set to true, if any, and then set this element's checkedness to true.

- [x] [The legacy-canceled-activation behavior for input elements are these steps](https://html.spec.whatwg.org/#input-activation-behavior) — The legacy-canceled-activation behavior for input elements are these steps:
  - 1. If the element's type attribute is in the Checkbox state, then set the element's checkedness and the element's indeterminateness back to the values they had before the legacy-pre-activation behavior was run.
  - 2. If this element's type attribute is in the Radio Button state, then if the element to which a reference was obtained in the legacy-pre-activation behavior, if any, is still in what is now this element's radio button group, if it still has one, and if so, set that element's checkedness to true; or else, if there was no such element, or that element is no longer in this element's radio button group…

- [x] [When an input element's type attribute changes state, the user agent must run t…](https://html.spec.whatwg.org/#input-activation-behavior) — When an input element's type attribute changes state, the user agent must run the following steps:
  - 1. If the previous state of the element's type attribute put the value IDL attribute in the value mode, and the element's value is not the empty string, and the new state of the element's type attribute puts the value IDL attribute in either the default mode or the default/on mode, then set the element's value content attribute to the element's value.
  - 2. Otherwise, if the previous state of the element's type attribute put the value IDL attribute in any mode other than the value mode, and the new state of the element's type attribute puts the value IDL attribute in the value mode, then set the value of the element to the value of the value content attribute, if there is one, or the empty string otherwise, and then set the control's dirty value fla…
  - 3. Otherwise, if the previous state of the element's type attribute put the value IDL attribute in any mode other than the filename mode, and the new state of the element's type attribute puts the value IDL attribute in the filename mode, then set the value of the element to the empty string.
  - 4. Update the element's rendering and behavior to the new state's.
  - 5. Signal a type change for the element. (The Radio Button state uses this, in particular.)
  - 6. Invoke the value sanitization algorithm, if one is defined for the type attribute's new state.
  - 7. Let previouslySelectable be true if setRangeText() previously applied to the element, and false otherwise.
  - 8. Let nowSelectable be true if setRangeText() now applies to the element, and false otherwise.
  - 9. If previouslySelectable is false and nowSelectable is true, set the element's text entry cursor position to the beginning of the text control, and set its selection direction to "none".

- [x] [HTMLInputElement.indeterminate](https://html.spec.whatwg.org/#dom-input-indeterminate) — The indeterminate getter steps are to return this's indeterminateness.

- [x] [The indeterminate setter steps](https://html.spec.whatwg.org/#dom-input-indeterminate) — The indeterminate setter steps are to set this's indeterminateness to the given value.

- [x] [HTMLInputElement.colorSpace](https://html.spec.whatwg.org/#dom-input-colorspace) — The colorSpace IDL attribute must reflect the colorspace content attribute, limited to only known values. The type IDL attribute must reflect the respective content attribute of the same name, limited to only known values.

- [x] [HTMLInputElement.width](https://html.spec.whatwg.org/#dom-input-width) — The width and height getter steps are to return the respective component of the result of determining the dimensions of this.

- [x] [dimensions](https://html.spec.whatwg.org/#input-img-dimensions) — To determine the dimensions of an input element input:
  - 1. If input's type attribute is not in the Image Button state, then return a width of 0 and a height of 0. (In that case no image is available.)
  - 2. Let naturalDimensions be null.
  - 3. If an image is available, then set naturalDimensions to the result of applying the default sizing algorithm with that image's natural width, natural height, and natural aspect ratio, if any, using a default object size of 300 by 150.
  - 4. Return the result of determining the dimensions of input given naturalDimensions.

##### [4.10.5.1 States of the type attribute](https://html.spec.whatwg.org/#states-of-the-type-attribute)

Status: reviewed (2026-11-01)
> `InputTypeNames` and the `InputType` subclasses.

###### [4.10.5.1.1 Hidden state (type=hidden)](https://html.spec.whatwg.org/#hidden-state-%28type=hidden%29)

Status: reviewed (2026-11-01)
> `HiddenInputType`.

###### [4.10.5.1.2 Text (type=text) state and Search state (type=search)](https://html.spec.whatwg.org/#text-%28type=text%29-state-and-search-state-%28type=search%29)

Status: reviewed (2026-11-01)
> `TextInputType`, `SearchInputType`; the value sanitisation strips newlines. The user interface step (text direction) is the editor's.

- [x] [If the element is mutable, the user agent should allow the user to change the w…](https://html.spec.whatwg.org/#text-%28type=text%29-state-and-search-state-%28type=search%29) — If the element is mutable, the user agent should allow the user to change the writing direction of the element, setting it either to a left-to-right writing direction or a right-to-left writing direction. If the user does so, the user agent must then run the following steps:
  - 1. Set the element's dir attribute to "ltr" if the user selected a left-to-right writing direction, and "rtl" if the user selected a right-to-left writing direction.
  - 2. Queue an element task on the user interaction task source given the element to fire an event named input at the element, with the bubbles and composed attributes initialized to true.

###### [4.10.5.1.3 Telephone state (type=tel)](https://html.spec.whatwg.org/#telephone-state-%28type=tel%29)

Status: reviewed (2026-11-01)
> `TelephoneInputType` (header only).

###### [4.10.5.1.4 URL state (type=url)](https://html.spec.whatwg.org/#url-state-%28type=url%29)

Status: reviewed (2026-11-01)
> `URLInputType`.

###### [4.10.5.1.5 Email state (type=email)](https://html.spec.whatwg.org/#email-state-%28type=email%29)

Status: reviewed (2026-11-01)
> `EmailInputType`; addresses are checked against the specification's grammar by hand (`IsValidEmailAddress`). User-facing punycode conversion is left out (no IDN in the UI).

- [x] [Whenever the user changes the element's values, the user agent must run the fol…](https://html.spec.whatwg.org/#email-state-%28type=email%29) — Whenever the user changes the element's values, the user agent must run the following steps:
  - 1. Let latest values be a copy of the element's values.
  - 2. Strip leading and trailing ASCII whitespace from each value in latest values.
  - 3. Set the element's value to the result of concatenating all the values in latest values, separating each value from the next by a single U+002C COMMA character (,), maintaining the list's order.

- [x] [The value sanitization algorithm](https://html.spec.whatwg.org/#value-sanitization-algorithm) — The value sanitization algorithm is as follows:
  - 1. Split on commas the element's value, strip leading and trailing ASCII whitespace from each resulting token, if any, and let the element's values be the (possibly empty) resulting list of (possibly empty) tokens, maintaining the original order.
  - 2. Set the element's value to the result of concatenating the element's values, separating each value from the next by a single U+002C COMMA character (,), maintaining the list's order.

- [x] [valid email address](https://html.spec.whatwg.org/#valid-e-mail-address) — A valid email address is a string that matches the email production of the following ABNF, the character set for which is Unicode. This ABNF implements the extensions described in RFC 1123. [ABNF] [RFC5322] [RFC1034] [RFC1123]

- [x] [valid email address list](https://html.spec.whatwg.org/#valid-e-mail-address-list) — A valid email address list is a set of comma-separated tokens, where each token is itself a valid email address. To obtain the list of tokens from a valid email address list, an implementation must split the string on commas.

###### [4.10.5.1.6 Password state (type=password)](https://html.spec.whatwg.org/#password-state-%28type=password%29)

Status: reviewed (2026-11-01)
> `PasswordInputType` (header only).

###### [4.10.5.1.7 Date state (type=date)](https://html.spec.whatwg.org/#date-state-%28type=date%29)

Status: reviewed (2026-11-01)
> `DateInputType`.

###### [4.10.5.1.8 Month state (type=month)](https://html.spec.whatwg.org/#month-state-%28type=month%29)

Status: reviewed (2026-11-01)
> `MonthInputType`.

###### [4.10.5.1.9 Week state (type=week)](https://html.spec.whatwg.org/#week-state-%28type=week%29)

Status: reviewed (2026-11-01)
> `WeekInputType`.

###### [4.10.5.1.10 Time state (type=time)](https://html.spec.whatwg.org/#time-state-%28type=time%29)

Status: reviewed (2026-11-01)
> `TimeInputType`; it has a periodic domain, so a reversed range.

###### [4.10.5.1.11 Local Date and Time state (type=datetime-local)](https://html.spec.whatwg.org/#local-date-and-time-state-%28type=datetime-local%29)

Status: reviewed (2026-11-01)
> `DateTimeLocalInputType`.

###### [4.10.5.1.12 Number state (type=number)](https://html.spec.whatwg.org/#number-state-%28type=number%29)

Status: reviewed (2026-11-01)
> `NumberInputType`; numbers aren't localised (see `KNOWN-ISSUES.md`).

###### [4.10.5.1.13 Range state (type=range)](https://html.spec.whatwg.org/#range-state-%28type=range%29)

Status: reviewed (2026-11-01)
> `RangeInputType`; no tick marks (no `list`, see `KNOWN-ISSUES.md`).

- [x] [default value](https://html.spec.whatwg.org/#concept-input-value-default-range) — The default value is the minimum plus half the difference between the minimum and the maximum, unless the maximum is less than the minimum, in which case the default value is the minimum.

###### [4.10.5.1.14 Color state (type=color)](https://html.spec.whatwg.org/#color-state-%28type=color%29)

Status: reviewed (2026-11-01)
> `ColorInputType`; no `alpha` or `colorspace` support in the colour well (see `KNOWN-ISSUES.md`), though `colorSpace` reflects.
<!-- definitions: attr-input-alpha attr-input-colorspace -->

- [x] [update a color well control color](https://html.spec.whatwg.org/#update-a-color-well-control-color) — To update a color well control color, given an element element:
  - 1. Assert: element is an input element whose type attribute is in the Color state.
  - 2. Let value be the result of running these steps:
    - 2a. If element's dirty value flag is false, then return the result of getting an attribute by namespace and local name given null, "value", and element.
    - 2b. Return element's value.
  - 3. Let color be the result of parsing value.
  - 4. If color is failure, then set color to opaque black.
  - 5. Set element's value to the result of serializing a color well control color given element and color.

- [x] [serialize a color well control color](https://html.spec.whatwg.org/#serialize-a-color-well-control-color) — To serialize a color well control color, given an element element and a CSS color color:
  - 1. Assert: element is an input element whose type attribute is in the Color state.
  - 2. Let htmlCompatible be false.
  - 3. If element's alpha attribute is not specified, then set color's alpha component to be fully opaque.
  - 4. If element's colorspace attribute is in the Limited sRGB state:
    - 4a. Set color to color converted to the 'srgb' color space.
    - 4b. Round each of color's components so they are in the range 0 to 255, inclusive. Components are to be rounded towards +∞.
    - 4c. If element's alpha attribute is not specified, then set htmlCompatible to true.
    - 4d. Otherwise, set color to color converted using the 'color()' function.
  - 5. Otherwise:
    - 5a. Assert: element's colorspace attribute is in the Display P3 state.
    - 5b. Set color to color converted to the 'display-p3' color space.
  - 6. Return the result of serializing color. If htmlCompatible is true, then do so with HTML-compatible serialization requested.

###### [4.10.5.1.15 Checkbox state (type=checkbox)](https://html.spec.whatwg.org/#checkbox-state-%28type=checkbox%29)

Status: reviewed (2026-11-01)
> `CheckboxInputType`.

- [x] [The input activation behavior](https://html.spec.whatwg.org/#input-activation-behavior) — The input activation behavior is to run the following steps:
  - 1. If the element is not connected, then return.
  - 2. Fire an event named input at the element with the bubbles and composed attributes initialized to true.
  - 3. Fire an event named change at the element with the bubbles attribute initialized to true.

###### [4.10.5.1.16 Radio Button state (type=radio)](https://html.spec.whatwg.org/#radio-button-state-%28type=radio%29)

Status: reviewed (2026-11-01)
> `RadioInputType` and `dom::RadioButtonGroups`.

- [x] [radio button group](https://html.spec.whatwg.org/#radio-button-group) — The radio button group that contains an input element a also contains all the other input elements b that fulfill all of the following conditions:

- [x] [The input activation behavior](https://html.spec.whatwg.org/#input-activation-behavior) — The input activation behavior is to run the following steps:
  - 1. If the element is not connected, then return.
  - 2. Fire an event named input at the element with the bubbles and composed attributes initialized to true.
  - 3. Fire an event named change at the element with the bubbles attribute initialized to true.

###### [4.10.5.1.17 File Upload state (type=file)](https://html.spec.whatwg.org/#file-upload-state-%28type=file%29)

Status: reviewed (2026-11-01)
> `FileInputType`; no file chooser (see `KNOWN-ISSUES.md`).
<!-- definitions: concept-input-type-file-selected concept-input-file-path attr-input-accept -->

- [x] [update the file selection](https://html.spec.whatwg.org/#update-the-file-selection) — To update the file selection for an element element:
  - 1. Queue an element task on the user interaction task source given element and the following steps:
    - 1a. Update element's selected files so that it represents the user's selection.
    - 1b. Fire an event named input at the input element, with the bubbles and composed attributes initialized to true.
    - 1c. Fire an event named change at the input element, with the bubbles attribute initialized to true.

###### [4.10.5.1.18 Submit Button state (type=submit)](https://html.spec.whatwg.org/#submit-button-state-%28type=submit%29)

Status: reviewed (2026-11-01)
> `SubmitInputType`.

- [x] [The element's input activation behavior given event](https://html.spec.whatwg.org/#submit-button-state-%28type=submit%29) — The element's input activation behavior given event is as follows:
  - 1. If the element does not have a form owner, then return.
  - 2. If the element's node document is not fully active, then return.
  - 3. Submit the element's form owner from the element with userInvolvement set to event's user navigation involvement.

###### [4.10.5.1.19 Image Button state (type=image)](https://html.spec.whatwg.org/#image-button-state-%28type=image%29)

Status: reviewed (2026-11-01)
> `ImageInputType`; the image isn't loaded (see `KNOWN-ISSUES.md`).
<!-- definitions: input-img-available -->

- [x] [then unless the user agent cannot support images, or its support for images has…](https://html.spec.whatwg.org/#attr-input-src) — then unless the user agent cannot support images, or its support for images has been disabled, or the user agent only fetches images on demand, or the src attribute's value is the empty string, run these steps:
  - 1. Let url be the result of encoding-parsing a URL given the src attribute's value, relative to the element's node document.
  - 2. If url is failure, then return.
  - 3. Let request be a new request whose URL is url, client is the element's node document's relevant settings object, destination is "image", initiator type is "input", credentials mode is "include", and whose use-URL-credentials flag is set.
  - 4. Fetch request, with processResponseEndOfBody set to the following steps given response response:
    - 4a. If the download was successful and the image is available, queue an element task on the user interaction task source given the input element to fire an event named load at the input element.
    - 4b. Otherwise, if the fetching process fails without a response from the remote server, or completes but the image is not a valid or supported image, then queue an element task on the user interaction task source given the input element to fire an event named error on the input element.

- [x] [The element's input activation behavior given event](https://html.spec.whatwg.org/#attr-input-alt) — The element's input activation behavior given event is as follows:
  - 1. If the element does not have a form owner, then return.
  - 2. If the element's node document is not fully active, then return.
  - 3. If the user activated the control while explicitly selecting a coordinate, then set the element's selected coordinate to that coordinate.
  - 4. Submit the element's form owner from the element with userInvolvement set to event's user navigation involvement.

- [x] [selected coordinate](https://html.spec.whatwg.org/#concept-input-type-image-coordinate) — The element's selected coordinate consists of an x-component and a y-component. It is initially (0, 0). The coordinates represent the position relative to the edge of the element's image, with the coordinate space having the positive x direction to the right, and the positive y direction downwards.

###### [4.10.5.1.20 Reset Button state (type=reset)](https://html.spec.whatwg.org/#reset-button-state-%28type=reset%29)

Status: reviewed (2026-11-01)
> `ResetInputType`.

- [x] [The element's input activation behavior](https://html.spec.whatwg.org/#reset-button-state-%28type=reset%29) — The element's input activation behavior is as follows:
  - 1. If the element does not have a form owner, then return.
  - 2. If the element's node document is not fully active, then return.
  - 3. Reset the form owner from the element.

###### [4.10.5.1.21 Button state (type=button)](https://html.spec.whatwg.org/#button-state-%28type=button%29)

Status: reviewed (2026-11-01)
> `ButtonInputType` (header only), on `BaseButtonInputType`.

##### [4.10.5.2 Implementation notes regarding localization of form controls](https://html.spec.whatwg.org/#input-impl-notes)

Status: n/a (user interface guidance)

##### [4.10.5.3 Common input element attributes](https://html.spec.whatwg.org/#common-input-element-attributes)

Status: reviewed (2026-11-01)
> Headings only.

###### [4.10.5.3.1 The maxlength and minlength attributes](https://html.spec.whatwg.org/#the-maxlength-and-minlength-attributes)

Status: reviewed (2026-11-01)
> `TextFieldInputType`/`HTMLInputElement::TooLong`, `TooShort`; only for what the user typed.
<!-- definitions: attr-input-maxlength attr-input-minlength -->

###### [4.10.5.3.2 The size attribute](https://html.spec.whatwg.org/#the-size-attribute)

Status: reviewed (2026-11-01)
> `HTMLInputElement::Size`.
<!-- definitions: attr-input-size -->

###### [4.10.5.3.3 The readonly attribute](https://html.spec.whatwg.org/#the-readonly-attribute)

Status: reviewed (2026-11-01)
> `HTMLTextFormControlElement` and `InputType::SupportsReadOnly`.
<!-- definitions: attr-input-readonly -->

###### [4.10.5.3.4 The required attribute](https://html.spec.whatwg.org/#the-required-attribute)

Status: reviewed (2026-11-01)
> `InputType::ValueMissing`.
<!-- definitions: attr-input-required concept-input-required -->

###### [4.10.5.3.5 The multiple attribute](https://html.spec.whatwg.org/#the-multiple-attribute)

Status: reviewed (2026-11-01)
> `EmailInputType`, `FileInputType`.
<!-- definitions: attr-input-multiple -->

###### [4.10.5.3.6 The pattern attribute](https://html.spec.whatwg.org/#the-pattern-attribute)

Status: reviewed (2026-11-01)
> `BaseTextInputType::PatternMismatch`, matching whole values; an ICU regular expression rather than an ECMAScript one with the `v` flag (see `DIFFERENCES.md`).
<!-- definitions: attr-input-pattern attr-input-title -->

- [x] [compiled pattern regular expression](https://html.spec.whatwg.org/#compiled-pattern-regular-expression) — The compiled pattern regular expression of an input element, if it exists, is a JavaScript RegExp object. It is determined as follows:
  - 1. If the element does not have a pattern attribute specified, then return nothing. The element has no compiled pattern regular expression.
  - 2. Let pattern be the value of the pattern attribute of the element.
  - 3. Let regexpCompletion be RegExpCreate(pattern, "v").
  - 4. If regexpCompletion is an abrupt completion, then return nothing. The element has no compiled pattern regular expression.
  - 5. Let anchoredPattern be the string "^(?:", followed by pattern, followed by ")$".
  - 6. Return ! RegExpCreate(anchoredPattern, "v").

- [x] [matches](https://html.spec.whatwg.org/#regexp-match-a-string) — A RegExp object regexp matches a string input, if ! RegExpBuiltinExec(regexp, input) is not null.

###### [4.10.5.3.7 The min and max attributes](https://html.spec.whatwg.org/#the-min-and-max-attributes)

Status: reviewed (2026-11-01)
> `StepRange`.
<!-- definitions: has-a-periodic-domain attr-input-min attr-input-max concept-input-min-default concept-input-max-default -->

- [x] [minimum](https://html.spec.whatwg.org/#concept-input-min) — If the element has a min attribute, and the result of applying the algorithm to convert a string to a number to the value of the min attribute is a number, then that number is the element's minimum; otherwise, if the type attribute's current state defines a default minimum, then that is the minimum; otherwise, the element has no minimum.

- [x] [maximum](https://html.spec.whatwg.org/#concept-input-max) — If the element has a max attribute, and the result of applying the algorithm to convert a string to a number to the value of the max attribute is a number, then that number is the element's maximum; otherwise, if the type attribute's current state defines a default maximum, then that is the maximum; otherwise, the element has no maximum.

- [x] [has a reversed range](https://html.spec.whatwg.org/#has-a-reversed-range) — An element has a reversed range if it has a periodic domain and its maximum is less than its minimum.

- [x] [has range limitations](https://html.spec.whatwg.org/#have-range-limitations) — An element has range limitations if it has a defined minimum or a defined maximum.

###### [4.10.5.3.8 The step attribute](https://html.spec.whatwg.org/#the-step-attribute)

Status: reviewed (2026-11-01)
> `StepRange`; the step base is the minimum, else the value attribute.
<!-- definitions: attr-input-step concept-input-step-default concept-input-step-scale concept-input-step-default-base -->

- [x] [allowed value step](https://html.spec.whatwg.org/#concept-input-step) — The attribute provides the allowed value step for the element, as follows:
  - 1. If the attribute does not apply, then there is no allowed value step.
  - 2. Otherwise, if the attribute is absent, then the allowed value step is the default step multiplied by the step scale factor.
  - 3. Otherwise, if the attribute's value is an ASCII case-insensitive match for the string "any", then there is no allowed value step.
  - 4. Otherwise, if the rules for parsing floating-point number values, when they are applied to the attribute's value, return an error, zero, or a number less than zero, then the allowed value step is the default step multiplied by the step scale factor.
  - 5. Otherwise, the allowed value step is the number returned by the rules for parsing floating-point number values when they are applied to the attribute's value, multiplied by the step scale factor.

- [x] [step base](https://html.spec.whatwg.org/#concept-input-min-zero) — The step base is the value returned by the following algorithm:
  - 1. If the element has a min content attribute, and the result of applying the algorithm to convert a string to a number to the value of the min content attribute is not an error, then return that result.
  - 2. If the element has a value content attribute, and the result of applying the algorithm to convert a string to a number to the value of the value content attribute is not an error, then return that result.
  - 3. If a default step base is defined for this element given its type attribute's state, then return it.
  - 4. Return zero.

###### [4.10.5.3.9 The list attribute](https://html.spec.whatwg.org/#the-list-attribute)

Status: reviewed (2026-11-01)
> No `list` (see `KNOWN-ISSUES.md`).
<!-- definitions: attr-input-list -->

- [x] [suggestions source element](https://html.spec.whatwg.org/#concept-input-list) — The suggestions source element is the first element in the tree in tree order to have an ID equal to the value of the list attribute, if that element is a datalist element. If there is no list attribute, or if there is no element with that ID, or if the first element with that ID is not a datalist element, then there is no suggestions source element.

###### [4.10.5.3.10 The placeholder attribute](https://html.spec.whatwg.org/#the-placeholder-attribute)

Status: reviewed (2026-11-01)
> `HTMLTextFormControlElement` placeholder.
<!-- definitions: attr-input-placeholder -->

##### [4.10.5.4 Common input element APIs](https://html.spec.whatwg.org/#common-input-element-apis)

Status: reviewed (2026-11-01)
> `HTMLInputElement`'s `value`, `checked`, `files`, `valueAsDate`, `valueAsNumber`, `stepUp`/`stepDown` and `showPicker`. No `list` (see `KNOWN-ISSUES.md`).
<!-- definitions: dom-input-value dom-input-value-default dom-input-value-default-on dom-input-value-filename dom-input-valueasdate dom-input-valueasnumber dom-input-stepup dom-select-showpicker -->

- [x] [On setting](https://html.spec.whatwg.org/#dom-input-value-value) — On setting:
  - 1. Let oldValue be the element's value.
  - 2. Set the element's value to the new value.
  - 3. Set the element's dirty value flag to true.
  - 4. Invoke the value sanitization algorithm, if the element's type attribute's current state defines one.
  - 5. If the element's value (after applying the value sanitization algorithm) is different from oldValue, and the element has a text entry cursor position, move the text entry cursor position to the end of the text control, unselecting any selected text and resetting the selection direction to "none".

- [x] [HTMLInputElement.checked](https://html.spec.whatwg.org/#dom-input-checked) — The checked IDL attribute allows scripts to manipulate the checkedness of an input element. On getting, it must return the current checkedness of the element; and on setting, it must set the element's checkedness to the new value and set the element's dirty checkedness flag to true.

- [x] [On setting, it must run these steps](https://html.spec.whatwg.org/#dom-input-files) — On setting, it must run these steps:
  - 1. If the IDL attribute does not apply or the given value is null, then return.
  - 2. Replace the element's selected files with the given value.

- [x] [HTMLInputElement.stepDown(n)](https://html.spec.whatwg.org/#dom-input-stepdown) — The stepDown(n) and stepUp(n) methods, when invoked, must run the following algorithm:
  - 1. If the stepDown() and stepUp() methods do not apply, as defined for the input element's type attribute's current state, then throw an "InvalidStateError" DOMException.
  - 2. If the element has no allowed value step, then throw an "InvalidStateError" DOMException.
  - 3. If the element has a minimum and a maximum and the minimum is greater than the maximum, then return.
  - 4. If the element has a minimum and a maximum and there is no value greater than or equal to the element's minimum and less than or equal to the element's maximum that, when subtracted from the step base, is an integral multiple of the allowed value step, then return.
  - 5. If applying the algorithm to convert a string to a number to the string given by the element's value does not result in an error, then let value be the result of that algorithm. Otherwise, let value be zero.
  - 6. Let valueBeforeStepping be value.
  - 7. If value subtracted from the step base is not an integral multiple of the allowed value step, then set value to the nearest value that, when subtracted from the step base, is an integral multiple of the allowed value step, and that is less than value if the method invoked was the stepDown() method, and more than value otherwise. Otherwise (value subtracted from the step base is an integral multip…
    - 7a. Let n be the argument.
    - 7b. Let delta be the allowed value step multiplied by n.
    - 7c. If the method invoked was the stepDown() method, negate delta.
    - 7d. Let value be the result of adding delta to value.
  - 8. If the element has a minimum, and value is less than that minimum, then set value to the smallest value that, when subtracted from the step base, is an integral multiple of the allowed value step, and that is more than or equal to that minimum.
  - 9. If the element has a maximum, and value is greater than that maximum, then set value to the largest value that, when subtracted from the step base, is an integral multiple of the allowed value step, and that is less than or equal to that maximum.
  - 10. If either the method invoked was the stepDown() method and value is greater than valueBeforeStepping, or the method invoked was the stepUp() method and value is less than valueBeforeStepping, then return.
  - 11. Let value as string be the result of running the algorithm to convert a number to a string, as defined for the input element's type attribute's current state, on value.
  - 12. Set the value of the element to value as string.

- [x] [HTMLInputElement.list](https://html.spec.whatwg.org/#dom-input-list) — The list IDL attribute must return the current suggestions source element, if any, or null otherwise.

- [x] [HTMLInputElement.showPicker()](https://html.spec.whatwg.org/#dom-input-showpicker) — The HTMLInputElement showPicker() and HTMLSelectElement showPicker() method steps are:
  - 1. If this is not mutable, then throw an "InvalidStateError" DOMException.
  - 2. If this's relevant settings object's origin is not same origin with this's relevant settings object's top-level origin, and this is a select element, or this's type attribute is not in the File Upload state or Color state, then throw a "SecurityError" DOMException.
  - 3. If this's relevant global object does not have transient activation, then throw a "NotAllowedError" DOMException.
  - 4. If this is a select element, and this is not being rendered, then throw a "NotSupportedError" DOMException.
  - 5. Show the picker, if applicable, for this.

- [x] [show the picker, if applicable](https://html.spec.whatwg.org/#show-the-picker,-if-applicable) — To show the picker, if applicable for an input or select element element:
  - 1. If element's relevant global object does not have transient activation, then return.
  - 2. If element is not mutable, then return.
  - 3. Consume user activation given element's relevant global object.
  - 4. If element does not support a picker, then return.
  - 5. If element is an input element and element's type attribute is in the File Upload state, then run these steps in parallel:
    - 5a. Optionally, wait until any prior execution of this algorithm has terminated.
    - 5b. Let dismissed be the result of WebDriver BiDi file dialog opened with element.
    - 5c. If dismissed is false:
      - 5c.i. Display a prompt to the user requesting that the user specify some files. If the multiple attribute is not set on element, there must be no more than one file selected; otherwise, any number may be selected. Files can be from the filesystem or created on the fly, e.g., a picture taken from a camera connected to the user's device.
      - 5c.ii. Wait for the user to have made their selection.
    - 5d. If dismissed is true or if the user dismissed the prompt without changing their selection, then queue an element task on the user interaction task source given element to fire an event named cancel at element, with the bubbles attribute initialized to true.
    - 5e. Otherwise, update the file selection for element.
  - 6. Otherwise, the user agent should show the relevant user interface for selecting a value for element, in the way it normally would when the user interacts with the control. When showing such a user interface, it must respect the requirements stated in the relevant parts of the specification for how element behaves given its type attribute state. (For example, various sections describe restrictions…

##### [4.10.5.5 Common event behaviors](https://html.spec.whatwg.org/#common-input-element-events)

Status: reviewed (2026-11-01)
> `input` and `change` from `InputType` and the editor.
