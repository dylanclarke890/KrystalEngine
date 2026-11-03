# HTML review checklist: 4.10 Forms

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 14 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html forms` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [4.10 Forms](https://html.spec.whatwg.org/#forms)

Status: reviewed (2026-11-01)
> Headings only.

#### [4.10.1 Introduction](https://html.spec.whatwg.org/#introduction-4)

Status: n/a (introduction)

##### [4.10.1.1 Writing a form's user interface](https://html.spec.whatwg.org/#writing-a-form's-user-interface)

Status: n/a (introduction)

##### [4.10.1.2 Implementing the server-side processing for a form](https://html.spec.whatwg.org/#implementing-the-server-side-processing-for-a-form)

Status: n/a (introduction)

##### [4.10.1.3 Configuring a form to communicate with a server](https://html.spec.whatwg.org/#configuring-a-form-to-communicate-with-a-server)

Status: n/a (introduction)

##### [4.10.1.4 Client-side form validation](https://html.spec.whatwg.org/#client-side-form-validation)

Status: n/a (introduction)

##### [4.10.1.5 Enabling client-side automatic filling of form controls](https://html.spec.whatwg.org/#enabling-client-side-automatic-filling-of-form-controls)

Status: n/a (introduction)

##### [4.10.1.6 Improving the user experience on mobile devices](https://html.spec.whatwg.org/#improving-the-user-experience-on-mobile-devices)

Status: n/a (introduction)

##### [4.10.1.7 The difference between the field type, the autofill field name, and the input modality](https://html.spec.whatwg.org/#the-difference-between-the-field-type,-the-autofill-field-name,-and-the-input-modality)

Status: n/a (introduction)

##### [4.10.1.8 Date, time, and number formats](https://html.spec.whatwg.org/#input-author-notes)

Status: n/a (introduction)

#### [4.10.2 Categories](https://html.spec.whatwg.org/#categories)

Status: reviewed (2026-11-01)
> Form-associated elements are `FormAssociatedElement`s, listed elements `FormListedElement`s, submittable ones `AppendFormData`'s, resettable ones `ValidatedFormListedElement::Reset`'s and labelable ones `HTMLElement::IsLabelable`'s.
<!-- definitions: form-associated-element category-listed category-submit concept-button concept-submit-button category-reset category-autocapitalize category-label -->

#### [4.10.3 The form element](https://html.spec.whatwg.org/#the-form-element)

Status: reviewed (2026-11-01)
> `HTMLFormElement`. The supported property names are only for scripts' enumeration; named items keep the past names map. Resetting does nothing in a document without a frame, as in upstream and Chrome. `img` elements aren't named items yet (see `KNOWN-ISSUES.md`).
> Fixed: `autocomplete` wasn't limited to its known values, and `elements` was a new collection each time.
<!-- definitions: attr-form-accept-charset attr-form-name attr-form-autocomplete attr-form-rel locked-for-reset -->

- [x] [HTMLFormElement.autocomplete](https://html.spec.whatwg.org/#dom-form-autocomplete) — The autocomplete IDL attribute must reflect the content attribute of the same name, limited to only known values.

- [x] [HTMLFormElement.elements](https://html.spec.whatwg.org/#dom-form-elements) — The elements IDL attribute must return an HTMLFormControlsCollection rooted at the form element's root, whose filter matches listed elements whose form owner is the form element, with the exception of input elements whose type attribute is in the Image Button state, which must, for historical reasons, be excluded from this particular collection.

- [x] [HTMLFormElement.length](https://html.spec.whatwg.org/#dom-form-length) — The length IDL attribute must return the number of nodes represented by the elements collection.

- [x] [The supported property names consist of the names obtained from the following a…](https://html.spec.whatwg.org/#past-names-map) — The supported property names consist of the names obtained from the following algorithm, in the order obtained from this algorithm:
  - 1. Let sourced names be an initially empty ordered list of tuples consisting of a string, an element, a source, where the source is either id, name, or past, and, if the source is past, an age.
  - 2. For each listed element candidate whose form owner is the form element, with the exception of any input elements whose type attribute is in the Image Button state:
    - 2a. If candidate has an id attribute, add an entry to sourced names with that id attribute's value as the string, candidate as the element, and id as the source.
    - 2b. If candidate has a name attribute, add an entry to sourced names with that name attribute's value as the string, candidate as the element, and name as the source.
  - 3. For each img element candidate whose form owner is the form element:
    - 3a. If candidate has an id attribute, add an entry to sourced names with that id attribute's value as the string, candidate as the element, and id as the source.
    - 3b. If candidate has a name attribute, add an entry to sourced names with that name attribute's value as the string, candidate as the element, and name as the source.
  - 4. For each entry past entry in the past names map, add an entry to sourced names with the past entry's name as the string, past entry's element as the element, past as the source, and the length of time past entry has been in the past names map as the age.
  - 5. Sort sourced names by tree order of the element entry of each tuple, sorting entries with the same element by putting entries whose source is id first, then entries whose source is name, and finally entries whose source is past, and sorting entries with the same element and source by their age, oldest first.
  - 6. Remove any entries in sourced names that have the empty string as their name.
  - 7. Remove any entries in sourced names that have the same name as an earlier entry in the map.
  - 8. Return the list of names from sourced names, maintaining their relative order.
  > Only for scripts' enumeration: there's no list of names, only `HTMLFormElement::IsSupportedPropertyName`.

- [x] [To determine the value of a named property name for a form element, the user ag…](https://html.spec.whatwg.org/#past-names-map) — To determine the value of a named property name for a form element, the user agent must run the following steps:
  - 1. Let candidates be a live RadioNodeList object containing all the listed elements, whose form owner is the form element, that have either an id attribute or a name attribute equal to name, with the exception of input elements whose type attribute is in the Image Button state, in tree order.
  - 2. If candidates is empty, let candidates be a live RadioNodeList object containing all the img elements, whose form owner is the form element, that have either an id attribute or a name attribute equal to name, in tree order.
  - 3. If candidates is empty, name is the name of one of the entries in the form element's past names map: return the object associated with name in that map.
  - 4. If candidates contains more than one node, return candidates.
  - 5. Otherwise, candidates contains exactly one node. Add a mapping from name to the node in candidates in the form element's past names map, replacing the previous entry with the same name, if any.
  - 6. Return the node in candidates.

- [x] [HTMLFormElement.submit()](https://html.spec.whatwg.org/#dom-form-submit) — The submit() method steps are to submit this from this, with submitted from submit() method set to true.

- [x] [HTMLFormElement.requestSubmit(submitter)](https://html.spec.whatwg.org/#dom-form-requestsubmit) — The requestSubmit(submitter) method, when invoked, must run the following steps:
  - 1. If submitter is not null:
    - 1a. If submitter is not a submit button, then throw a TypeError.
    - 1b. If submitter's form owner is not this form element, then throw a "NotFoundError" DOMException.
  - 2. Otherwise, set submitter to this form element.
  - 3. Submit this form element, from submitter.

- [x] [HTMLFormElement.reset()](https://html.spec.whatwg.org/#dom-form-reset) — The reset() method, when invoked, must run the following steps:
  - 1. If the form element is marked as locked for reset, then return.
  - 2. Mark the form element as locked for reset.
  - 3. Reset the form element.
  - 4. Unmark the form element as locked for reset.

- [x] [HTMLFormElement.checkValidity()](https://html.spec.whatwg.org/#dom-form-checkvalidity) — If the checkValidity() method is invoked, the user agent must statically validate the constraints of the form element, and return true if the constraint validation returned a positive result, and false if it returned a negative result.

- [x] [HTMLFormElement.reportValidity()](https://html.spec.whatwg.org/#dom-form-reportvalidity) — If the reportValidity() method is invoked, the user agent must interactively validate the constraints of the form element, and return true if the constraint validation returned a positive result, and false if it returned a negative result.

#### [4.10.4 The label element](https://html.spec.whatwg.org/#the-label-element)

Status: reviewed (2026-11-01)
> `HTMLLabelElement`; `labels` is `HTMLElement::Labels`. A `for` attribute finds a control only in a connected label's tree, as in upstream and Chrome.
> Fixed: `labels` was a new list each time; nodes now keep it in their rare data.
<!-- definitions: labeled-control attr-label-for -->

- [x] [HTMLLabelElement.control](https://html.spec.whatwg.org/#dom-label-control) — The control IDL attribute must return the label element's labeled control, if any, or null if there isn't one.

- [x] [HTMLLabelElement.form](https://html.spec.whatwg.org/#dom-label-form) — The form IDL attribute must run the following steps:
  - 1. If the label element has no labeled control, then return null.
  - 2. If the label element's labeled control is not a form-associated element, then return null.
  - 3. Return the label element's labeled control's form owner (which can still be null).

- [x] [HTMLInputElement,HTMLButtonElement,HTMLSelectElement,HTMLTextAreaElement,HTMLOutputElement,HTMLProgressElement,HTMLMeterElement.labels](https://html.spec.whatwg.org/#dom-lfe-labels) — Labelable elements and all input elements have a live NodeList object associated with them that represents the list of label elements, in tree order, whose labeled control is the element in question. The labels IDL attribute of labelable elements that are not form-associated custom elements, and the labels IDL attribute of input elements, on getting, must return that NodeList object, and that same value must always be returned, unless this element is an input element whose type attribute is in the Hidden state, in which case it must instead return null.

- [x] [ElementInternals.labels](https://html.spec.whatwg.org/#dom-elementinternals-labels) — Form-associated custom elements don't have a labels IDL attribute. Instead, their ElementInternals object has a labels IDL attribute. On getting, it must throw a "NotSupportedError" DOMException if the target element is not a form-associated custom element. Otherwise, it must return that NodeList object, and that same value must always be returned.
