# HTML review checklist: 4.4 Grouping content

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 5 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html grouping-content` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [4.4 Grouping content](https://html.spec.whatwg.org/#grouping-content)

Status: reviewed (2026-11-01)
> Headings only.

#### [4.4.1 The p element](https://html.spec.whatwg.org/#the-p-element)

Status: reviewed (2026-11-01)
> `HTMLParagraphElement`; its obsolete `align` is reflected (obsolete features).

#### [4.4.2 The hr element](https://html.spec.whatwg.org/#the-hr-element)

Status: reviewed (2026-11-01)
> `HTMLHRElement`; its obsolete attributes are reflected (obsolete features), and rendered with Rendering.

#### [4.4.3 The pre element](https://html.spec.whatwg.org/#the-pre-element)

Status: reviewed (2026-11-01)
> `HTMLPreElement`; the leading newline is dropped by the parser, and the obsolete `width` is reflected.

#### [4.4.4 The blockquote element](https://html.spec.whatwg.org/#the-blockquote-element)

Status: reviewed (2026-11-01)
> `HTMLQuoteElement` (shared with `q`): `cite` is reflected as a URL.
<!-- definitions: attr-blockquote-cite -->

#### [4.4.5 The ol element](https://html.spec.whatwg.org/#the-ol-element)

Status: reviewed (2026-11-01)
> `HTMLOListElement`: `StartValue` is the starting value; `type` is a presentational hint (Rendering). Removed a FIXME claiming `type` is reflected as an enumerated attribute: it isn't limited to known values.
<!-- definitions: attr-ol-reversed attr-ol-start attr-ol-type -->

- [x] [starting value](https://html.spec.whatwg.org/#concept-ol-start) — An ol element has a starting value, which is an integer determined as follows:
  - 1. If the ol element has a start attribute:
    - 1a. Let parsed be the result of parsing the value of the attribute as an integer.
    - 1b. If parsed is not an error, then return parsed.
  - 2. If the ol element has a reversed attribute, then return the number of owned li elements.
  - 3. Return 1.

#### [4.4.6 The ul element](https://html.spec.whatwg.org/#the-ul-element)

Status: reviewed (2026-11-01)
> `HTMLUListElement`; its obsolete attributes are reflected.

#### [4.4.7 The menu element](https://html.spec.whatwg.org/#the-menu-element)

Status: reviewed (2026-11-01)
> `HTMLMenuElement`; its obsolete `compact` is reflected.

#### [4.4.8 The li element](https://html.spec.whatwg.org/#the-li-element)

Status: reviewed (2026-11-01)
> `HTMLLIElement`; `value` is a presentational hint setting the `list-item` counter. List owners and ordinal values are `RenderListItem`'s, from the `list-item` counter rather than these steps (see `DIFFERENCES.md`).
<!-- definitions: attr-li-value -->

- [x] [list owner](https://html.spec.whatwg.org/#list-owner) — Any element whose computed value of 'display' is 'list-item' has a list owner, which is determined as follows:
  - 1. If the element is not being rendered, return null; the element has no list owner.
  - 2. Let ancestor be the element's parent.
  - 3. If the element has an ol, ul, or menu ancestor, set ancestor to the closest such ancestor element.
  - 4. Return the closest inclusive ancestor of ancestor that produces a CSS box.
  > `RenderListItem.cpp`'s `EnclosingList`, with the `list-item` counter (see `DIFFERENCES.md`).

- [x] [ordinal value](https://html.spec.whatwg.org/#ordinal-value) — To determine the ordinal value of each element owned by a given list owner owner, perform the following steps:
  - 1. Let i be 1.
  - 2. If owner is an ol element, let numbering be owner's starting value. Otherwise, let numbering be 1.
  - 3. Loop: If i is greater than the number of list items that owner owns, then return; all of owner's owned list items have been assigned ordinal values.
  - 4. Let item be the ith of owner's owned list items, in tree order.
  - 5. If item is an li element that has a value attribute:
    - 5a. Let parsed be the result of parsing the value of the attribute as an integer.
    - 5b. If parsed is not an error, then set numbering to parsed.
  - 6. The ordinal value of item is numbering.
  - 7. If owner is an ol element, and owner has a reversed attribute, decrement numbering by 1; otherwise, increment numbering by 1.
  - 8. Increment i by 1.
  - 9. Go to the step labeled loop.
  > `RenderListItem::UpdateValueNow`, with the `list-item` counter (see `DIFFERENCES.md`).

#### [4.4.9 The dl element](https://html.spec.whatwg.org/#the-dl-element)

Status: reviewed (2026-11-01)
> `HTMLDListElement`; its obsolete `compact` is reflected. Name-value groups are only for authors and accessibility.

- [x] [process dt or dd](https://html.spec.whatwg.org/#process-dt-or-dd) — To process dt or dd for a node node means to follow these steps:
  - 1. Let groups, current, and seenDd be the same variables as those of the same name in the algorithm that invoked these steps.
  - 2. If node is a dt element:
    - 2a. If seenDd is true, then append current to groups, set current to a new name-value group, and set seenDd to false.
    - 2b. Append node to current's name.
  - 3. Otherwise, if node is a dd element, then append node to current's value and set seenDd to true.
  > n/a: name-value groups are only for authors and accessibility.

#### [4.4.10 The dt element](https://html.spec.whatwg.org/#the-dt-element)

Status: n/a (author conformance)

#### [4.4.11 The dd element](https://html.spec.whatwg.org/#the-dd-element)

Status: n/a (author conformance)

#### [4.4.12 The figure element](https://html.spec.whatwg.org/#the-figure-element)

Status: n/a (author conformance)

#### [4.4.13 The figcaption element](https://html.spec.whatwg.org/#the-figcaption-element)

Status: n/a (author conformance)

#### [4.4.14 The main element](https://html.spec.whatwg.org/#the-main-element)

Status: n/a (author conformance)

- [x] [hierarchically correct main element](https://html.spec.whatwg.org/#hierarchically-correct-main-element) — A hierarchically correct main element is one whose ancestor elements are limited to html, body, div, form without an accessible name, and autonomous custom elements. Each main element must be a hierarchically correct main element.

#### [4.4.15 The search element](https://html.spec.whatwg.org/#the-search-element)

Status: n/a (author conformance)

#### [4.4.16 The div element](https://html.spec.whatwg.org/#the-div-element)

Status: reviewed (2026-11-01)
> `HTMLDivElement`; its obsolete `align` is reflected.
