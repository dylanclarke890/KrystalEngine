# HTML review checklist: 4.7 Edits

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 0 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html edits` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [4.7 Edits](https://html.spec.whatwg.org/#edits)

Status: reviewed (2026-11-01)
> Headings only.

#### [4.7.1 The ins element](https://html.spec.whatwg.org/#the-ins-element)

Status: reviewed (2026-11-01)
> `HTMLModElement`, shared with `del`.

#### [4.7.2 The del element](https://html.spec.whatwg.org/#the-del-element)

Status: reviewed (2026-11-01)
> `HTMLModElement`, shared with `ins`.

#### [4.7.3 Attributes common to ins and del elements](https://html.spec.whatwg.org/#attributes-common-to-ins-and-del-elements)

Status: reviewed (2026-11-01)
> `HTMLModElement`: `cite` is reflected as a URL and `dateTime` as it's written; the date isn't parsed (see `DIFFERENCES.md`).
<!-- definitions: attr-mod-cite attr-mod-datetime -->

#### [4.7.4 Edits and paragraphs](https://html.spec.whatwg.org/#edits-and-paragraphs)

Status: n/a (author conformance)

#### [4.7.5 Edits and lists](https://html.spec.whatwg.org/#edits-and-lists)

Status: n/a (author conformance)

#### [4.7.6 Edits and tables](https://html.spec.whatwg.org/#edits-and-tables)

Status: n/a (author conformance)
