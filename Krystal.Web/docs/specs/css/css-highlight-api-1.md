# css-highlight-api-1 review checklist

Generated from <https://drafts.csswg.org/css-highlight-api-1/> on 2026-11-25 by `Krystal.Web/tools/spec_checklist.py`: 2 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Web/tools/spec_status.py css-highlight-api-1` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[css-highlight-api-1:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Web/tools/spec_status.py css-highlight-api-1 --tick <section anchor>` ticks a section's algorithms and sets its status.

## [Abstract](https://drafts.csswg.org/css-highlight-api-1/#abstract)

Status: n/a (front matter)

## [Status of this document](https://drafts.csswg.org/css-highlight-api-1/#sotd)

Status: n/a (front matter)

## [1. Introduction](https://drafts.csswg.org/css-highlight-api-1/#intro)

Status: n/a (introduction)

## [2. Module Interactions](https://drafts.csswg.org/css-highlight-api-1/#interaction)

Status: n/a (no requirements)

## [3. Setting up Custom Highlights](https://drafts.csswg.org/css-highlight-api-1/#highlights-set-up)

Status: n/a (heading only)

### [3.1. Creating Custom Highlights](https://drafts.csswg.org/css-highlight-api-1/#creation)

Status: reviewed (2026-11-25)
> Code: Css/Highlights/Highlight.hpp. Highlight::Create adds the initial ranges as add() does, so a repeat is dropped; a range already in the set keeps its place.
<!-- definitions: custom-highlight highlight dom-highlight-highlight -->

### [3.2. Registering Custom Highlights](https://drafts.csswg.org/css-highlight-api-1/#registration)

Status: reviewed (2026-11-25)
> Code: Css/Highlights/HighlightRegistry.hpp; Document::GetHighlightRegistry() is CSS.highlights (see DIFFERENCES.md). Names keep their registration order, which a replacement keeps.
<!-- definitions: highlight-registry registered -->

- [x] [custom highlight name](https://drafts.csswg.org/css-highlight-api-1/#custom-highlight-name) — The custom highlight name assigned to a custom highlight when it is registered is used to identify the highlight during styling (see § 4 Styling Custom Highlights).

## [4. Styling Custom Highlights](https://drafts.csswg.org/css-highlight-api-1/#styling-highlights)

Status: n/a (heading only)

### [4.1. The Custom Highlight Pseudo-element: ::highlight()](https://drafts.csswg.org/css-highlight-api-1/#custom-highlight-pseudo)

Status: reviewed (2026-11-25)
> Code: SelectorChecker matches ::highlight(name) against the requested name; MarkedText::CollectForHighlights asks for each registered name's style.
<!-- definitions: custom-highlight-pseudo-element -->

### [4.2. Processing Model](https://drafts.csswg.org/css-highlight-api-1/#processing-model)

Status: n/a (heading only)

#### [4.2.1. Applicable Properties](https://drafts.csswg.org/css-highlight-api-1/#applicable-properties)

Status: reviewed (2026-11-25)
> Code: shared with css-pseudo-4's highlight pseudo-elements (RenderStyle's highlight pseudo styles, StyledMarkedText).

#### [4.2.2. Default Styles](https://drafts.csswg.org/css-highlight-api-1/#default-styles)

Status: reviewed (2026-11-25)
> Code: shared with css-pseudo-4's highlight pseudo-elements (RenderStyle's highlight pseudo styles, StyledMarkedText).

#### [4.2.3. Cascading and Inheritance](https://drafts.csswg.org/css-highlight-api-1/#c-and-h)

Status: reviewed (2026-11-25)
> Code: shared with css-pseudo-4's highlight pseudo-elements (RenderStyle's highlight pseudo styles, StyledMarkedText).

#### [4.2.4. Painting](https://drafts.csswg.org/css-highlight-api-1/#painting)

Status: reviewed (2026-11-25)
> Code: shared with css-pseudo-4's highlight pseudo-elements (RenderStyle's highlight pseudo styles, StyledMarkedText).

#### [4.2.5. Priority of Overlapping Highlights](https://drafts.csswg.org/css-highlight-api-1/#priorities)

Status: reviewed (2026-11-25)
> Code: MarkedText::CollectForHighlights keeps highlights in priority order, then registration order (stable sorts in Subdivide).
<!-- definitions: priority -->

#### [4.2.6. Highlight types](https://drafts.csswg.org/css-highlight-api-1/#highlight-types)

Status: reviewed (2026-11-25)
> Code: Highlight::Type. Not exposed to assistive technologies yet (FIXME; see DIFFERENCES.md).

## [5. Responding to Changes](https://drafts.csswg.org/css-highlight-api-1/#responding-to-changes)

Status: n/a (heading only)

### [5.1. Repaints](https://drafts.csswg.org/css-highlight-api-1/#repaint)

Status: reviewed (2026-11-25)
> Code: Highlight::RepaintRange and Repaint invalidate the renderers the ranges intersect, from Add, Delete, Clear, SetPriority and the registry's Set, Delete and Clear; Document::UpdateHighlightPositions follows live ranges.

### [5.2. Range Updating and Invalidation](https://drafts.csswg.org/css-highlight-api-1/#range-invalidation)

Status: reviewed (2026-11-25)
> Code: MarkedText::CollectForHighlights skips ranges whose nodes aren't in the document and invalid static ranges; static ranges are stored as they are.

## [6. Interacting with Custom Highlights](https://drafts.csswg.org/css-highlight-api-1/#interactions)

Status: reviewed (2026-11-25)
> Code: HighlightRegistry::HighlightsFromPoint, hit testing with NodeFromPoint (null outside the visible content rect) and the ranges' client rects.

- [x] [HighlightRegistry.highlightsFromPoint(x, y, options)](https://drafts.csswg.org/css-highlight-api-1/#dom-highlightregistry-highlightsfrompoint) — The highlightsFromPoint(x, y, options) method must return the result of running these steps:
  - 1. If any of the following are true, return the empty sequence: x is negative; y is negative; x is greater than the viewport width excluding the size of a rendered scroll bar (if any); y is greater than the viewport height excluding the size of a rendered scroll bar (if any); The topmost box in the viewport in paint order that would be a target for hit testing at coordinates x,y when applying the tr…
  - 2. Otherwise, let results be an empty sequence.
  - 3. For each Highlight highlight in this HighlightRegistry:
    - 3a. Let result be a new HighlightHitResult with highlight set to highlight.
    - 3b. For each AbstractRange abstractRange in highlight:
      - 3b.i. If abstractRange is an invalid StaticRange, then continue.
      - 3b.ii. Let range be a new Range whose start node and end node are set to abstractRange’s start node and end node respectively, and start offset and end offset are set to abstractRange’s start offset and end offset respectively.
      - 3b.iii. If the coordinates x,y fall inside at least one of the DOMRects returned by calling getClientRects() on range, then append abstractRange to result.ranges.
    - 3c. If result.ranges is not empty, append result to results.
  - 4. Sort results by descending order of priority of its HighlightHitResults' highlight attributes.
  - 5. Return results.

## [7. Event Handling](https://drafts.csswg.org/css-highlight-api-1/#events)

Status: n/a (no requirements)

## [Appendix A. Privacy Considerations](https://drafts.csswg.org/css-highlight-api-1/#priv)

Status: n/a (informative)

## [Appendix B. Security Considerations](https://drafts.csswg.org/css-highlight-api-1/#sec)

Status: n/a (informative)

## [Appendix C. Acknowledgements](https://drafts.csswg.org/css-highlight-api-1/#credits)

Status: n/a (back matter)

## [Appendix D. Changes](https://drafts.csswg.org/css-highlight-api-1/#changes)

Status: n/a (back matter)

### [Changes since the 15 December 2021 Working Draft](https://drafts.csswg.org/css-highlight-api-1/#changes-since-20211215)

Status: n/a (back matter)

### [Changes since the 8 December 2020 Working Draft](https://drafts.csswg.org/css-highlight-api-1/#changes-since-20201208)

Status: n/a (back matter)

### [Changes since the 22 October 2020 Working Draft](https://drafts.csswg.org/css-highlight-api-1/#changes-since-20201022)

Status: n/a (back matter)

## [Conformance](https://drafts.csswg.org/css-highlight-api-1/#w3c-conformance)

Status: n/a (back matter)

### [Document conventions](https://drafts.csswg.org/css-highlight-api-1/#w3c-conventions)

Status: n/a (back matter)

### [Conformance classes](https://drafts.csswg.org/css-highlight-api-1/#w3c-conformance-classes)

Status: n/a (back matter)

### [Partial implementations](https://drafts.csswg.org/css-highlight-api-1/#w3c-partial)

Status: n/a (back matter)

#### [Implementations of Unstable and Proprietary Features](https://drafts.csswg.org/css-highlight-api-1/#w3c-conform-future-proofing)

Status: n/a (back matter)

### [Non-experimental implementations](https://drafts.csswg.org/css-highlight-api-1/#w3c-testing)

Status: n/a (back matter)

## [Index](https://drafts.csswg.org/css-highlight-api-1/#index)

Status: n/a (back matter)

### [Terms defined by this specification](https://drafts.csswg.org/css-highlight-api-1/#index-defined-here)

Status: n/a (back matter)

### [Terms defined by reference](https://drafts.csswg.org/css-highlight-api-1/#index-defined-elsewhere)

Status: n/a (back matter)

## [References](https://drafts.csswg.org/css-highlight-api-1/#references)

Status: n/a (back matter)

### [Normative References](https://drafts.csswg.org/css-highlight-api-1/#normative)

Status: n/a (back matter)

### [Non-Normative References](https://drafts.csswg.org/css-highlight-api-1/#informative)

Status: n/a (back matter)

## [IDL Index](https://drafts.csswg.org/css-highlight-api-1/#idl-index)

Status: n/a (back matter)

## [Issues Index](https://drafts.csswg.org/css-highlight-api-1/#issues-index)

Status: n/a (back matter)
