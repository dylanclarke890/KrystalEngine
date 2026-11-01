# HTML review checklist: 4.3 Sections

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 3 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html sections` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [4.3 Sections](https://html.spec.whatwg.org/#sections)

Status: reviewed (2026-11-01)
> Headings only.

#### [4.3.1 The body element](https://html.spec.whatwg.org/#the-body-element)

Status: reviewed (2026-11-01)
> `HTMLBodyElement`; its event handlers forwarded to the window are reviewed with event handlers, and its presentational attributes with Rendering.

#### [4.3.2 The article element](https://html.spec.whatwg.org/#the-article-element)

Status: n/a (author conformance)

#### [4.3.3 The section element](https://html.spec.whatwg.org/#the-section-element)

Status: n/a (author conformance)

#### [4.3.4 The nav element](https://html.spec.whatwg.org/#the-nav-element)

Status: n/a (author conformance)

#### [4.3.5 The aside element](https://html.spec.whatwg.org/#the-aside-element)

Status: n/a (author conformance)

#### [4.3.6 The h1, h2, h3, h4, h5, and h6 elements](https://html.spec.whatwg.org/#the-h1,-h2,-h3,-h4,-h5,-and-h6-elements)

Status: reviewed (2026-11-01)
> `HTMLHeadingElement`; its levels are 4.3.11.1's.

#### [4.3.7 The hgroup element](https://html.spec.whatwg.org/#the-hgroup-element)

Status: n/a (author conformance)

#### [4.3.8 The header element](https://html.spec.whatwg.org/#the-header-element)

Status: n/a (author conformance)

#### [4.3.9 The footer element](https://html.spec.whatwg.org/#the-footer-element)

Status: n/a (author conformance)

#### [4.3.10 The address element](https://html.spec.whatwg.org/#the-address-element)

Status: n/a (author conformance)

#### [4.3.11 Headings and outlines](https://html.spec.whatwg.org/#headings-and-outlines-2)

Status: reviewed (2026-11-01)
> A heading's level is `HTMLHeadingElement::ComputedHeadingLevel`. The outline is only for exposing to users, which Booey doesn't do.
<!-- definitions: heading-level concept-heading -->

- [x] [outline](https://html.spec.whatwg.org/#outline) — The outline is all headings in a document, in tree order.
  > n/a: only for exposing outlines to users.

##### [4.3.11.1 Heading levels & offsets](https://html.spec.whatwg.org/#heading-levels-&-offsets)

Status: reviewed (2026-11-01)
> `HTMLHeadingElement::ComputedHeadingLevel` and `ComputedHeadingOffset`, with `headingoffset` and `headingreset` reflected by `HTMLElement`. New: the computed heading level and offset weren't implemented (nothing uses them yet: there's no `:heading()` and no accessibility).
<!-- definitions: attr-headingoffset attr-headingreset -->

- [x] [get an element's computed heading level](https://html.spec.whatwg.org/#get-an-element's-computed-heading-level) — To get an element's computed heading level, given an element element:
  - 1. Let level be 0.
  - 2. If element's local name is h1, then set level to 1.
  - 3. If element's local name is h2, then set level to 2.
  - 4. If element's local name is h3, then set level to 3.
  - 5. If element's local name is h4, then set level to 4.
  - 6. If element's local name is h5, then set level to 5.
  - 7. If element's local name is h6, then set level to 6.
  - 8. Assert: level is not 0.
  - 9. Increment level by the result of getting an element's computed heading offset given element.
  - 10. If level is greater than 9, then return 9.
  - 11. Return level.

- [x] [get an element's computed heading offset](https://html.spec.whatwg.org/#get-an-element's-computed-heading-offset) — To get an element's computed heading offset, given an element element, perform the following steps. They return a non-negative integer.
  - 1. Let offset be 0.
  - 2. Let inclusiveAncestor be element.
  - 3. While inclusiveAncestor is not null:
    - 3a. Let nextOffset be 0.
    - 3b. If inclusiveAncestor is an HTML element and has a headingoffset attribute, then parse its value using the rules for parsing non-negative integers. If the result of parsing the value is not an error, then set nextOffset to that value.
    - 3c. Increment offset by nextOffset.
    - 3d. If inclusiveAncestor is an HTML element and has a headingreset attribute, then return offset.
    - 3e. If inclusiveAncestor's parent is a shadow root, then set inclusiveAncestor to that shadow root's host and continue.
    - 3f. Set inclusiveAncestor to inclusiveAncestor's parent element.
  - 4. Return offset.

##### [4.3.11.2 Sample outlines](https://html.spec.whatwg.org/#sample-outlines)

Status: n/a (examples)

##### [4.3.11.3 Exposing outlines to users](https://html.spec.whatwg.org/#exposing-outlines-to-users)

Status: n/a (Booey has no browser user interface)

#### [4.3.12 Usage summary](https://html.spec.whatwg.org/#usage-summary-2)

Status: n/a (author conformance)

##### [4.3.12.1 Article or section?](https://html.spec.whatwg.org/#article-or-section)

Status: n/a (author conformance)
