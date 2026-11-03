# HTML review checklist: 4.5 Text-level semantics

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 10 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html text-level-semantics` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [4.5 Text-level semantics](https://html.spec.whatwg.org/#text-level-semantics)

Status: reviewed (2026-11-01)
> Headings only.

#### [4.5.1 The a element](https://html.spec.whatwg.org/#the-a-element)

Status: reviewed (2026-11-01)
> `HTMLAnchorElement`: its attributes are reflected, `relList` supports `noreferrer`, `noopener` and `opener`, and `text` is the descendant text content. Its activation behaviour is reviewed with links.

- [x] [HTMLAnchorElement.referrerPolicy](https://html.spec.whatwg.org/#dom-a-referrerpolicy) — The IDL attribute referrerPolicy must reflect the referrerpolicy content attribute, limited to only known values.

- [x] [HTMLAnchorElement.text](https://html.spec.whatwg.org/#dom-a-text) — The text attribute's getter must return this element's descendant text content.

#### [4.5.2 The em element](https://html.spec.whatwg.org/#the-em-element)

Status: n/a (author conformance)

#### [4.5.3 The strong element](https://html.spec.whatwg.org/#the-strong-element)

Status: n/a (author conformance)

#### [4.5.4 The small element](https://html.spec.whatwg.org/#the-small-element)

Status: n/a (author conformance)

#### [4.5.5 The s element](https://html.spec.whatwg.org/#the-s-element)

Status: n/a (author conformance)

#### [4.5.6 The cite element](https://html.spec.whatwg.org/#the-cite-element)

Status: n/a (author conformance)

#### [4.5.7 The q element](https://html.spec.whatwg.org/#the-q-element)

Status: reviewed (2026-11-01)
> `HTMLQuoteElement`: `cite` is reflected as a URL; the quotation marks are Rendering's.
<!-- definitions: attr-q-cite -->

#### [4.5.8 The dfn element](https://html.spec.whatwg.org/#the-dfn-element)

Status: reviewed (2026-11-01)
> Only `HTMLElement`. The defining term is for authors and accessibility; nothing reads it.
<!-- definitions: attr-dfn-title -->

- [x] [Defining term](https://html.spec.whatwg.org/#defining-term) — Defining term: if the dfn element has a title attribute, then the exact value of that attribute is the term being defined. Otherwise, if it contains exactly one element child node and no child Text nodes, and that child element is an abbr element with a title attribute, then the exact value of that attribute is the term being defined. Otherwise, it is the descendant text content of the dfn element that gives the term being defined.
  > n/a: only for authors and accessibility.

#### [4.5.9 The abbr element](https://html.spec.whatwg.org/#the-abbr-element)

Status: n/a (author conformance)
<!-- definitions: attr-abbr-title -->

#### [4.5.10 The ruby element](https://html.spec.whatwg.org/#the-ruby-element)

Status: reviewed (2026-11-01)
> Only `HTMLElement`; ruby is laid out by CSS Ruby (`display: ruby` and its annotations in the user agent sheet), which segments it in layout instead of with these steps.

- [x] [At any particular time, the segmentation and categorization of content of a rub…](https://html.spec.whatwg.org/#the-ruby-element) — At any particular time, the segmentation and categorization of content of a ruby element is the result that would be obtained from running the following algorithm:
  - 1. Let base text segments be an empty list of base text segments, each potentially with a list of base text subsegments.
  - 2. Let annotation segments be an empty list of annotation segments, each potentially being associated with a base text segment or subsegment.
  - 3. Let root be the ruby element for which the algorithm is being run.
  - 4. If root has a ruby element ancestor, then jump to the step labeled end.
  - 5. Let current parent be root.
  - 6. Let index be 0.
  - 7. Let start index be null.
  - 8. Let saved start index be null.
  - 9. Let current base text be null.
  - 10. Start mode: If index is greater than or equal to the number of child nodes in current parent, then jump to the step labeled end mode.
  - 11. If the indexth node in current parent is an rt or rp element, jump to the step labeled annotation mode.
  - 12. Set start index to the value of index.
  - 13. Base mode: If the indexth node in current parent is a ruby element, and if current parent is the same element as root, then push a ruby level and then jump to the step labeled start mode.
  - 14. If the indexth node in current parent is an rt or rp element, then set the current base text and then jump to the step labeled annotation mode.
  - 15. Increment index by one.
  - 16. Base mode post-increment: If index is greater than or equal to the number of child nodes in current parent, then jump to the step labeled end mode.
  - 17. Jump back to the step labeled base mode.
  - 18. Annotation mode: If the indexth node in current parent is an rt element, then push a ruby annotation and jump to the step labeled annotation mode increment.
  - 19. If the indexth node in current parent is an rp element, jump to the step labeled annotation mode increment.
  - 20. If the indexth node in current parent is not a Text node, or is a Text node that is not inter-element whitespace, then jump to the step labeled base mode.
  - 21. Annotation mode increment: Let lookahead index be index plus one.
  - 22. Annotation mode white-space skipper: If lookahead index is equal to the number of child nodes in current parent then jump to the step labeled end mode.
  - 23. If the lookahead indexth node in current parent is an rt element or an rp element, then set index to lookahead index and jump to the step labeled annotation mode.
  - 24. If the lookahead indexth node in current parent is not a Text node, or is a Text node that is not inter-element whitespace, then jump to the step labeled base mode (without further incrementing index, so the inter-element whitespace seen so far becomes part of the next base text segment).
  - 25. Increment lookahead index by one.
  - 26. Jump to the step labeled annotation mode white-space skipper.
  - 27. End mode: If current parent is not the same element as root, then pop a ruby level and jump to the step labeled base mode post-increment.
  - 28. End: Return base text segments and annotation segments. Any content of the ruby element not described by segments in either of those lists is implicitly in an ignored segment.
  > n/a: CSS Ruby segments ruby in layout.

- [x] [set the current base text](https://html.spec.whatwg.org/#set-the-current-base-text) — When the steps above say to set the current base text, it means to run the following steps at that point in the algorithm:
  - 1. Let text range be a DOM range whose start is the boundary point (current parent, start index) and whose end is the boundary point (current parent, index).
  - 2. Let new text segment be a base text segment described by the range text range.
  - 3. Add new text segment to base text segments.
  - 4. Let current base text be new text segment.
  - 5. Let start index be null.
  > n/a: CSS Ruby segments ruby in layout.

- [x] [push a ruby level](https://html.spec.whatwg.org/#push-a-ruby-level) — When the steps above say to push a ruby level, it means to run the following steps at that point in the algorithm:
  - 1. Let current parent be the indexth node in current parent.
  - 2. Let index be 0.
  - 3. Set saved start index to the value of start index.
  - 4. Let start index be null.
  > n/a: CSS Ruby segments ruby in layout.

- [x] [pop a ruby level](https://html.spec.whatwg.org/#pop-a-ruby-level) — When the steps above say to pop a ruby level, it means to run the following steps at that point in the algorithm:
  - 1. Let index be the position of current parent in root.
  - 2. Let current parent be root.
  - 3. Increment index by one.
  - 4. Set start index to the value of saved start index.
  - 5. Let saved start index be null.
  > n/a: CSS Ruby segments ruby in layout.

- [x] [push a ruby annotation](https://html.spec.whatwg.org/#push-a-ruby-annotation) — When the steps above say to push a ruby annotation, it means to run the following steps at that point in the algorithm:
  - 1. Let rt be the rt element that is the indexth node of current parent.
  - 2. Let annotation range be a DOM range whose start is the boundary point (current parent, index) and whose end is the boundary point (current parent, index plus one) (i.e. that contains only rt).
  - 3. Let new annotation segment be an annotation segment described by the range annotation range.
  - 4. If current base text is not null, associate new annotation segment with current base text.
  - 5. Add new annotation segment to annotation segments.
  > n/a: CSS Ruby segments ruby in layout.

#### [4.5.11 The rt element](https://html.spec.whatwg.org/#the-rt-element)

Status: n/a (author conformance)

#### [4.5.12 The rp element](https://html.spec.whatwg.org/#the-rp-element)

Status: n/a (author conformance)

#### [4.5.13 The data element](https://html.spec.whatwg.org/#the-data-element)

Status: reviewed (2026-11-01)
> `HTMLDataElement`: `value` is reflected.
<!-- definitions: attr-data-value -->

#### [4.5.14 The time element](https://html.spec.whatwg.org/#the-time-element)

Status: reviewed (2026-11-01)
> `HTMLTimeElement`: `dateTime` is reflected. Its machine-readable value isn't parsed (see `DIFFERENCES.md`).
<!-- definitions: attr-time-datetime -->

- [x] [datetime value](https://html.spec.whatwg.org/#datetime-value) — The datetime value of a time element is the value of the element's datetime content attribute, if it has one, otherwise the child text content of the time element.
  > n/a: only for the machine-readable equivalent.

- [x] [machine-readable equivalent of the element's contents](https://html.spec.whatwg.org/#machine-readable-equivalent-of-the-element's-contents) — The machine-readable equivalent of the element's contents must be obtained from the element's datetime value by using the following algorithm:
  - 1. If parsing a month string from the element's datetime value returns a month, that is the machine-readable equivalent; return.
  - 2. If parsing a date string from the element's datetime value returns a date, that is the machine-readable equivalent; return.
  - 3. If parsing a yearless date string from the element's datetime value returns a yearless date, that is the machine-readable equivalent; return.
  - 4. If parsing a time string from the element's datetime value returns a time, that is the machine-readable equivalent; return.
  - 5. If parsing a local date and time string from the element's datetime value returns a local date and time, that is the machine-readable equivalent; return.
  - 6. If parsing a time-zone offset string from the element's datetime value returns a time-zone offset, that is the machine-readable equivalent; return.
  - 7. If parsing a global date and time string from the element's datetime value returns a global date and time, that is the machine-readable equivalent; return.
  - 8. If parsing a week string from the element's datetime value returns a week, that is the machine-readable equivalent; return.
  - 9. If the element's datetime value consists of only ASCII digits, at least one of which is not U+0030 DIGIT ZERO (0), then the machine-readable equivalent is the base-ten interpretation of those digits, representing a year; return.
  - 10. If parsing a duration string from the element's datetime value returns a duration, that is the machine-readable equivalent; return.
  - 11. There is no machine-readable equivalent.
  > n/a: not parsed, as in browsers (see `DIFFERENCES.md`).

#### [4.5.15 The code element](https://html.spec.whatwg.org/#the-code-element)

Status: n/a (author conformance)

#### [4.5.16 The var element](https://html.spec.whatwg.org/#the-var-element)

Status: n/a (author conformance)

#### [4.5.17 The samp element](https://html.spec.whatwg.org/#the-samp-element)

Status: n/a (author conformance)

#### [4.5.18 The kbd element](https://html.spec.whatwg.org/#the-kbd-element)

Status: n/a (author conformance)

#### [4.5.19 The sub and sup elements](https://html.spec.whatwg.org/#the-sub-and-sup-elements)

Status: n/a (author conformance)

#### [4.5.20 The i element](https://html.spec.whatwg.org/#the-i-element)

Status: n/a (author conformance)

#### [4.5.21 The b element](https://html.spec.whatwg.org/#the-b-element)

Status: n/a (author conformance)

#### [4.5.22 The u element](https://html.spec.whatwg.org/#the-u-element)

Status: n/a (author conformance)

#### [4.5.23 The mark element](https://html.spec.whatwg.org/#the-mark-element)

Status: n/a (author conformance)

#### [4.5.24 The bdi element](https://html.spec.whatwg.org/#the-bdi-element)

Status: reviewed (2026-11-01)
> `HTMLBDIElement`: its directionality defaults to auto (3.2.6's), and it's isolated by the user agent sheet's `unicode-bidi: isolate`.

#### [4.5.25 The bdo element](https://html.spec.whatwg.org/#the-bdo-element)

Status: reviewed (2026-11-01)
> Only `HTMLElement`; `unicode-bidi: isolate-override` and its direction come from the user agent sheet.

#### [4.5.26 The span element](https://html.spec.whatwg.org/#the-span-element)

Status: reviewed (2026-11-01)
> `HTMLSpanElement`.

#### [4.5.27 The br element](https://html.spec.whatwg.org/#the-br-element)

Status: reviewed (2026-11-01)
> `HTMLBRElement`: a `RenderLineBreak` unless `content` replaces it; its obsolete `clear` is reflected and a presentational hint.

#### [4.5.28 The wbr element](https://html.spec.whatwg.org/#the-wbr-element)

Status: reviewed (2026-11-01)
> `HTMLWBRElement`, a line break opportunity in layout.

#### [4.5.29 Usage summary](https://html.spec.whatwg.org/#usage-summary)

Status: n/a (author conformance)
