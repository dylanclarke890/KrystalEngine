# HTML review checklist: 4.8.15 MathML

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 2 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html embedded-content-other` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

#### [4.8.15 MathML](https://html.spec.whatwg.org/#mathml)

Status: reviewed (2026-11-01)
> MathML elements in HTML are parsed as foreign content and rendered with MathML Core (`OVERVIEW.md`).

#### [4.8.16 SVG](https://html.spec.whatwg.org/#svg-0)

Status: reviewed (2026-11-01)
> SVG elements in HTML are parsed as foreign content and rendered with the legacy SVG render tree. There's no `getSVGDocument()`: there are no content documents without a frame tree.

- [x] [HTMLIFrameElement,HTMLEmbedElement,HTMLObjectElement.getSVGDocument()](https://html.spec.whatwg.org/#dom-media-getsvgdocument) — The getSVGDocument() method steps are:
  - 1. Let document be this's content document.
  - 2. If document is non-null and was created by the page load processing model for XML files section because the computed type of the resource in the navigate algorithm was image/svg+xml, then return document.
  - 3. Return null.
  > n/a: there's no frame tree, so no content documents.

#### [4.8.17 Dimension attributes](https://html.spec.whatwg.org/#dimension-attributes)

Status: reviewed (2026-11-01)
> `width` and `height` are presentational hints (Rendering). The dimensions are `HTMLImageElement::Width` and `Height`, and `ImageInputType::Width` and `Height`.
> Fixed: `<input>`'s `width` and `height` returned the attribute for every type; they're now an image button's dimensions and 0 otherwise. An image button doesn't load its image yet (FIXME in `ImageInputType.cpp`).
<!-- definitions: attr-dim-width attr-dim-height -->

- [x] [determine the dimensions](https://html.spec.whatwg.org/#determine-dimensions) — To determine the dimensions of an img or input element element given a struct consisting of a width and a height, or null, naturalDimensions:
  - 1. If element is being rendered, then return the width and height of element's content box, in CSS pixels. [CSS]
  - 2. Let width and height be 0.
  - 3. If naturalDimensions is not null, then set width to naturalDimensions's width and height to naturalDimensions's height.
  - 4. If element has a width attribute:
    - 4a. Let attributeWidth be the result of parsing that attribute's value.
    - 4b. If attributeWidth is not an error, then set width to attributeWidth.
  - 5. If element has a height attribute:
    - 5a. Let attributeHeight be the result of parsing that attribute's value.
    - 5b. If attributeHeight is not an error, then set height to attributeHeight.
  - 6. Return a width of width and a height of height, in CSS pixels.
