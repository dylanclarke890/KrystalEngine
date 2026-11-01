# HTML review checklist: 4.14 Common idioms without dedicated elements

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 4 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html semantics-other` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [4.14 Common idioms without dedicated elements](https://html.spec.whatwg.org/#common-idioms)

Status: unreviewed

#### [4.14.1 Breadcrumb navigation](https://html.spec.whatwg.org/#rel-up)

Status: unreviewed

#### [4.14.2 Tag clouds](https://html.spec.whatwg.org/#tag-clouds)

Status: unreviewed

#### [4.14.3 Conversations](https://html.spec.whatwg.org/#conversations)

Status: unreviewed

#### [4.14.4 Footnotes](https://html.spec.whatwg.org/#footnotes)

Status: unreviewed

### [4.15 Disabled elements](https://html.spec.whatwg.org/#disabled-elements)

Status: unreviewed

- [ ] [actually disabled](https://html.spec.whatwg.org/#concept-element-disabled) — An element is said to be actually disabled if it is one of the following:

### [4.16 Matching HTML elements using selectors and CSS](https://html.spec.whatwg.org/#selectors)

Status: unreviewed

#### [4.16.1 Case-sensitivity of the CSS 'attr()' function](https://html.spec.whatwg.org/#case-sensitivity-of-the-css-'attr%28%29'-function)

Status: unreviewed

#### [4.16.2 Case-sensitivity of selectors](https://html.spec.whatwg.org/#case-sensitivity-of-selectors)

Status: unreviewed

#### [4.16.3 Pseudo-classes](https://html.spec.whatwg.org/#pseudo-classes)

Status: unreviewed
<!-- definitions: selector-defined selector-link selector-visited selector-active concept-selector-active selector-hover concept-selector-hover selector-focus selector-target selector-popover-open selector-enabled selector-disabled selector-checked selector-indeterminate selector-default selector-placeholder-shown selector-valid selector-invalid selector-user-valid selector-user-invalid selector-in-range selector-out-of-range selector-required selector-optional selector-autofill selector-webkit-autofill selector-read-only selector-read-write selector-modal selector-ltr selector-rtl selector-custom selector-heading selector-heading-functional selector-playing selector-paused selector-seeking selector-buffering selector-stalled selector-muted selector-volume-locked selector-open -->

- [ ] [in a formal activation state](https://html.spec.whatwg.org/#in-a-formal-activation-state) — An element is said to be in a formal activation state between the time the user begins to indicate an intent to trigger the element's activation behavior and either the time the user stops indicating an intent to trigger the element's activation behavior, or the time the element's activation behavior has finished running, which ever comes first.

- [ ] [being actively pointed at](https://html.spec.whatwg.org/#being-actively-pointed-at) — An element is said to be being actively pointed at while the user indicates the element using a pointing device while that pointing device is in the "down" state (e.g. for a mouse, between the time the mouse button is pressed and the time it is released; for a finger in a multitouch environment, while the finger is touching the display surface).

- [ ] [element has the focus](https://html.spec.whatwg.org/#element-has-the-focus) — For the purposes of the CSS :focus pseudo-class, an element has the focus when:
