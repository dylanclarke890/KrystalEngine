# HTML review checklist: 14 The XML syntax

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 4 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html xhtml` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

## [14 The XML syntax](https://html.spec.whatwg.org/#the-xhtml-syntax)

Status: unreviewed

### [14.1 Writing documents in the XML syntax](https://html.spec.whatwg.org/#writing-xhtml-documents)

Status: unreviewed

### [14.2 Parsing XML documents](https://html.spec.whatwg.org/#parsing-xhtml-documents)

Status: unreviewed
<!-- definitions: xml-parser xml-scripting-support-enabled feed-the-parser -->

- [ ] [When an XML parser with XML scripting support enabled creates a script element,…](https://html.spec.whatwg.org/#xml-scripting-support-disabled) — When an XML parser with XML scripting support enabled creates a script element, it must have its parser document set and its force async set to false. If the parser was created as part of the XML fragment parsing algorithm, then the element's already started must be set to true. When the element's…
  - 1. Block this instance of the XML parser, such that the event loop will not run tasks that invoke it.
  - 2. Spin the event loop until the parser's Document has no style sheet that is blocking scripts and the pending parsing-blocking script's ready to be parser-executed is true.
  - 3. Unblock this instance of the XML parser, such that tasks that invoke it can again be run.
  - 4. Execute the script element given by the pending parsing-blocking script.
  - 5. Set the pending parsing-blocking script to null.

### [14.3 Serializing XML fragments](https://html.spec.whatwg.org/#serialising-xhtml-fragments)

Status: unreviewed

- [ ] [XML fragment serialization algorithm](https://html.spec.whatwg.org/#xml-fragment-serialisation-algorithm) — The XML fragment serialization algorithm for a Document or Element node either returns a fragment of XML that represents that node or throws an exception.

- [ ] [relevant child nodes](https://html.spec.whatwg.org/#relevant-child-nodes) — A node's relevant child nodes are those that apply given the following rules:
  - • For template elements: The relevant child nodes are the child nodes of the template element's template contents, if any.
  - • For all other nodes: The relevant child nodes are the child nodes of node itself, if any.

### [14.4 Parsing XML fragments](https://html.spec.whatwg.org/#parsing-xhtml-fragments)

Status: unreviewed

- [ ] [XML fragment parsing algorithm](https://html.spec.whatwg.org/#xml-fragment-parsing-algorithm) — The XML fragment parsing algorithm given an Element or DocumentFragment node target and a string input, runs the following steps. They return a DocumentFragment.
  - 1. Let context be target if target is an Element; otherwise target's host.
  - 2. Assert: context is non-null.
  - 3. Create a new XML parser.
  - 4. Feed the parser just created the string corresponding to the start tag of context, declaring all the namespace prefixes that are in scope on that element in the DOM, as well as declaring the default namespace (if any) that is in scope on that element in the DOM. A namespace prefix is in scope if the DOM lookupNamespaceURI() method on the element would return a non-null value for that prefix. The…
  - 5. Feed the parser just created the string input.
  - 6. Feed the parser just created the string corresponding to the end tag of context.
  - 7. If there is an XML well-formedness or XML namespace well-formedness error, then throw a "SyntaxError" DOMException.
  - 8. If the document element of the resulting Document has any sibling nodes, then throw a "SyntaxError" DOMException.
  - 9. Let newChildren be the resulting Document node's document element's children, in tree order.
  - 10. Let fragment be the result of creating a document fragment given context's node document.
  - 11. For each node of newChildren, in tree order: append node to fragment.
  - 12. Return fragment.
