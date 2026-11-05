# HTML review checklist: 8.4 Dynamic markup insertion

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 84 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html dynamic-markup-insertion` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [8.4 Dynamic markup insertion](https://html.spec.whatwg.org/#dynamic-markup-insertion)

Status: n/a (there is no `document.open()`/`write()`: scripting isn't planned, see `OVERVIEW.md`)
> The sections below.
<!-- definitions: throw-on-dynamic-markup-insertion-counter -->

#### [8.4.1 Opening the input stream](https://html.spec.whatwg.org/#opening-the-input-stream)

Status: n/a (there is no `document.open()`/`write()`: scripting isn't planned, see `OVERVIEW.md`)
> There's no script-created parser or insertion point.
<!-- definitions: active-parser-was-aborted script-created-parser -->

- [x] [document open steps](https://html.spec.whatwg.org/#document-open-steps) — The document open steps, given a document, are as follows:
  - 1. If document is an XML document, then throw an "InvalidStateError" DOMException.
  - 2. If document's throw-on-dynamic-markup-insertion counter is greater than 0, then throw an "InvalidStateError" DOMException.
  - 3. Let entryDocument be the entry global object's associated Document.
  - 4. If document's origin is not same origin to entryDocument's origin, then throw a "SecurityError" DOMException.
  - 5. If document has an active parser whose script nesting level is greater than 0, then return document.
  - 6. Similarly, if document's unload counter is greater than 0, then return document.
  - 7. If document's active parser was aborted is true, then return document.
  - 8. If document's node navigable is non-null and document's node navigable's ongoing navigation is a navigation ID, then stop loading document's node navigable.
  - 9. For each shadow-including inclusive descendant node of document, erase all event listeners and handlers given node.
  - 10. If document is the associated Document of document's relevant global object, then erase all event listeners and handlers given document's relevant global object.
  - 11. Replace all with null within document.
  - 12. If document is fully active:
    - 12a. Let newURL be a copy of entryDocument's URL.
    - 12b. If entryDocument is not document, then set newURL's fragment to null.
    - 12c. Run the URL and history update steps with document and newURL.
  - 13. Set document's is initial about:blank to false.
  - 14. If document's iframe load in progress flag is set, then set document's mute iframe load flag.
  - 15. Set document to no-quirks mode.
  - 16. Create an HTML parser whose allow declarative shadow roots is document's allow declarative shadow roots, and associate it with document. This is a script-created parser (meaning that it can be closed by the document.open() and document.close() methods, and that the tokenizer will wait for an explicit call to document.close() before emitting an end-of-file token). The encoding confidence is irrele…
  - 17. Set the insertion point to point at just before the end of the input stream (which at this point will be empty).
  - 18. Update the current document readiness of document to "loading".
  - 19. Return document.

- [x] [Document.open(unused1, unused2)](https://html.spec.whatwg.org/#dom-document-open) — The open(unused1, unused2) method must return the result of running the document open steps with this.

- [x] [Document.open(url, name, features)](https://html.spec.whatwg.org/#dom-document-open-window) — The open(url, name, features) method must run these steps:
  - 1. If this is not fully active, then throw an "InvalidAccessError" DOMException.
  - 2. Return the result of running the window open steps with url, name, and features.

#### [8.4.2 Closing the input stream](https://html.spec.whatwg.org/#closing-the-input-stream)

Status: n/a (there is no `document.open()`/`write()`: scripting isn't planned, see `OVERVIEW.md`)

- [x] [Document.close()](https://html.spec.whatwg.org/#dom-document-close) — The close() method must run the following steps:
  - 1. If this is an XML document, then throw an "InvalidStateError" DOMException.
  - 2. If this's throw-on-dynamic-markup-insertion counter is greater than zero, then throw an "InvalidStateError" DOMException.
  - 3. If there is no script-created parser associated with this, then return.
  - 4. Insert an explicit "EOF" character at the end of the parser's input stream.
  - 5. If this's pending parsing-blocking script is not null, then return.
  - 6. Run the tokenizer, processing resulting tokens as they are emitted, and stopping when the tokenizer reaches the explicit "EOF" character or spins the event loop.

#### [8.4.3 document.write()](https://html.spec.whatwg.org/#document.write%28%29)

Status: n/a (there is no `document.open()`/`write()`: scripting isn't planned, see `OVERVIEW.md`)
<!-- definitions: ignore-destructive-writes-counter -->

- [x] [document write steps](https://html.spec.whatwg.org/#document-write-steps) — The document write steps, given a Document object document, a list text, a boolean lineFeed, and a string sink, are as follows:
  - 1. Let string be the empty string.
  - 2. Let isTrusted be false if text contains a string; otherwise true.
  - 3. For each value of text:
    - 3a. If value is a TrustedHTML object, then append value's associated data to string.
    - 3b. Otherwise, append value to string.
  - 4. If isTrusted is false, set string to the result of invoking the get trusted type compliant string algorithm with TrustedHTML, this's relevant global object, string, sink, and "script".
  - 5. If lineFeed is true, append U+000A LINE FEED to string.
  - 6. If document is an XML document, then throw an "InvalidStateError" DOMException.
  - 7. If document's throw-on-dynamic-markup-insertion counter is greater than 0, then throw an "InvalidStateError" DOMException.
  - 8. If document's active parser was aborted is true, then return.
  - 9. If the insertion point is undefined:
    - 9a. If document's unload counter is greater than 0 or document's ignore-destructive-writes counter is greater than 0, then return.
    - 9b. Run the document open steps with document.
  - 10. Insert string into the input stream just before the insertion point.
  - 11. If document's pending parsing-blocking script is null, then have the HTML parser process string, one code point at a time, processing resulting tokens as they are emitted, and stopping when the tokenizer reaches the insertion point or when the processing of the tokenizer is aborted by the tree construction stage (this can happen if a script end tag token is emitted by the tokenizer).

- [x] [Document.document.write(...text)](https://html.spec.whatwg.org/#dom-document-write) — The document.write(...text) method steps are to run the document write steps with this, text, false, and "Document write".

#### [8.4.4 document.writeln()](https://html.spec.whatwg.org/#document.writeln%28%29)

Status: n/a (there is no `document.open()`/`write()`: scripting isn't planned, see `OVERVIEW.md`)

- [x] [Document.document.writeln(...text)](https://html.spec.whatwg.org/#dom-document-writeln) — The document.writeln(...text) method steps are to run the document write steps with this, text, true, and "Document writeln".

### [8.5 DOM parsing and serialization APIs](https://html.spec.whatwg.org/#dom-parsing-and-serialization)

Status: reviewed (2026-11-01)
> The sections below.

#### [8.5.1 The DOMParser interface](https://html.spec.whatwg.org/#the-domparser-interface)

Status: n/a (there is no `DOMParser`: `Document::ParseHTMLUnsafe` and `Document::ParseHTML` parse HTML into a new document)
> `Document::ParseHTMLIntoNewDocument` implements parse HTML from a string, for `parseHTMLUnsafe()` and `parseHTML()`.
<!-- definitions: dom-domparsersupportedtype-texthtml dom-domparsersupportedtype-otherwise -->

- [x] [new DOMParser()](https://html.spec.whatwg.org/#dom-domparser-constructor) — The new DOMParser() constructor steps are to do nothing.

- [x] [DOMParser.parseFromString(string, type)](https://html.spec.whatwg.org/#dom-domparser-parsefromstring) — The parseFromString(string, type) method steps are:
  - 1. Let compliantString be the result of invoking the get trusted type compliant string algorithm with TrustedHTML, this's relevant global object, string, "DOMParser parseFromString", and "script".
  - 2. Let relevantDocument be this's relevant global object's associated Document.
  - 3. Let document be a new Document, whose content type is type, origin is relevantDocument's origin, and URL is relevantDocument's URL.
  - 4. Switch on type: "text/html" Otherwise
    - 4a. Parse HTML from a string given document and compliantString.
    - 4a. Create an XML parser parser, associated with document, and with XML scripting support disabled.
    - 4b. Parse compliantString using parser.
    - 4c. If the previous step resulted in an XML well-formedness or XML namespace well-formedness error:
      - 4c.i. Assert: document has no child nodes.
      - 4c.ii. Let root be the result of creating an element given document, "parsererror", and "http://www.mozilla.org/newlayout/xml/parsererror.xml".
      - 4c.iii. Optionally, add attributes or children to root to describe the nature of the parsing error.
      - 4c.iv. Append root to document.
  - 5. Return document.

- [x] [parse HTML from a string](https://html.spec.whatwg.org/#parse-html-from-a-string) — To parse HTML from a string, given a Document document, a string html, and an optional SanitizerConfig or null configuration (default null):
  - 1. Set document's type to "html".
  - 2. Let parser be a new HTML parser whose allow declarative shadow roots is document's allow declarative shadow roots and parser sanitizer configuration is configuration, associated with document.
  - 3. Place html into the input stream for parser. The encoding confidence is irrelevant.
  - 4. Start parser and let it run until it has consumed all the characters just inserted into the input stream.

#### [8.5.2 HTML parsing and insertion methods](https://html.spec.whatwg.org/#html-parsing-methods)

Status: reviewed (2026-11-01)
> `Element`, `ShadowRoot` and `Document`'s `SetHTML`/`SetHTMLUnsafe` and the insertion methods (`HTMLInsertionAlgorithms.cpp`); `Document::ParseHTMLUnsafe` and `ParseHTML`. There are no Trusted Types and no scripts to run.
<!-- definitions: html-insertion-methods -->

- [x] [Element.setHTML(html, options)](https://html.spec.whatwg.org/#dom-element-sethtml) — Element's setHTML(html, options) method steps are:
  - 1. Let target be the target for HTML insertion given this.
  - 2. Filter and set HTML given target, html, options, and Safe.

- [x] [Element.appendHTML(html, options)](https://html.spec.whatwg.org/#dom-element-appendhtml) — Element's appendHTML(html, options) method steps are:
  - 1. Let target be the target for HTML insertion given this.
  - 2. Filter and pre-insert HTML given target, null, html, options, and Safe.

- [x] [Element.prependHTML(html, options)](https://html.spec.whatwg.org/#dom-element-prependhtml) — Element's prependHTML(html, options) method steps are:
  - 1. Let target be the target for HTML insertion given this.
  - 2. Filter and pre-insert HTML given target, target's first child, html, options, and Safe.

- [x] [ShadowRoot.setHTML(html, options)](https://html.spec.whatwg.org/#dom-shadowroot-sethtml) — ShadowRoot's setHTML(html, options) method steps are to filter and set HTML given this, html, options, and Safe.

- [x] [ShadowRoot.appendHTML(html, options)](https://html.spec.whatwg.org/#dom-shadowroot-appendhtml) — ShadowRoot's appendHTML(html, options) method steps are to filter and pre-insert HTML given this, null, html, options, and Safe.

- [x] [ShadowRoot.prependHTML(html, options)](https://html.spec.whatwg.org/#dom-shadowroot-prependhtml) — ShadowRoot's prependHTML(html, options) method steps are to filter and pre-insert HTML given this, this's first child, html, options, and Safe.

- [x] [NonDocumentTypeChildNode.beforeHTML(html, options)](https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-beforehtml) — NonDocumentTypeChildNode's beforeHTML(html, options) method steps are:
  - 1. Let parent be the parent for HTML insertion given this.
  - 2. If parent is null, then return.
  - 3. Filter and pre-insert HTML given parent, this, html, options, and Safe.

- [x] [NonDocumentTypeChildNode.afterHTML(html, options)](https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-afterhtml) — NonDocumentTypeChildNode's afterHTML(html, options) method steps are:
  - 1. Let parent be the parent for HTML insertion given this.
  - 2. If parent is null, then return.
  - 3. Filter and pre-insert HTML given parent, this's next sibling, html, options, and Safe.

- [x] [NonDocumentTypeChildNode.replaceWithHTML(html, options)](https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-replacewithhtml) — NonDocumentTypeChildNode's replaceWithHTML(html, options) method steps are:
  - 1. Let parent be the parent for HTML insertion given this.
  - 2. If parent is null, then return.
  - 3. Filter and replace with HTML given parent, this, html, options, and Safe.

- [x] [Element.setHTMLUnsafe(html, options)](https://html.spec.whatwg.org/#dom-element-sethtmlunsafe) — Element's setHTMLUnsafe(html, options) method steps are:
  - 1. Let (compliantHTML, compliantOptions) be the result of invoking the get trusted type compliant input algorithm with this's relevant global object, html, options, and "Element setHTMLUnsafe".
  - 2. Let target be the target for HTML insertion given this.
  - 3. Filter and set HTML given target, compliantHTML, compliantOptions, and Unsafe.

- [x] [Element.appendHTMLUnsafe(html, options)](https://html.spec.whatwg.org/#dom-element-appendhtmlunsafe) — Element's appendHTMLUnsafe(html, options) method steps are:
  - 1. Let (compliantHTML, compliantOptions) be the result of invoking the get trusted type compliant input algorithm with this's relevant global object, html, options, and "Element appendHTMLUnsafe".
  - 2. Let target be the target for HTML insertion given this.
  - 3. Filter and pre-insert HTML given target, null, compliantHTML, compliantOptions, and Unsafe.

- [x] [Element.prependHTMLUnsafe(html, options)](https://html.spec.whatwg.org/#dom-element-prependhtmlunsafe) — Element's prependHTMLUnsafe(html, options) method steps are:
  - 1. Let (compliantHTML, compliantOptions) be the result of invoking the get trusted type compliant input algorithm with this's relevant global object, html, options, and "Element prependHTMLUnsafe".
  - 2. Let target be the target for HTML insertion given this.
  - 3. Filter and pre-insert HTML given target, target's first child, compliantHTML, compliantOptions, and Unsafe.

- [x] [ShadowRoot.setHTMLUnsafe(html, options)](https://html.spec.whatwg.org/#dom-shadowroot-sethtmlunsafe) — ShadowRoot's setHTMLUnsafe(html, options) method steps are:
  - 1. Let (compliantHTML, compliantOptions) be the result of invoking the get trusted type compliant input algorithm with this's relevant global object, html, options, and "ShadowRoot setHTMLUnsafe".
  - 2. Filter and set HTML given this, compliantHTML, compliantOptions, and Unsafe.

- [x] [ShadowRoot.appendHTMLUnsafe(html, options)](https://html.spec.whatwg.org/#dom-shadowroot-appendhtmlunsafe) — ShadowRoot's appendHTMLUnsafe(html, options) method steps are:
  - 1. Let (compliantHTML, compliantOptions) be the result of invoking the get trusted type compliant input algorithm with this's relevant global object, html, options, and "ShadowRoot appendHTMLUnsafe".
  - 2. Filter and pre-insert HTML given this, null, compliantHTML, compliantOptions, and Unsafe.

- [x] [ShadowRoot.prependHTMLUnsafe(html, options)](https://html.spec.whatwg.org/#dom-shadowroot-prependhtmlunsafe) — ShadowRoot's prependHTMLUnsafe(html, options) method steps are:
  - 1. Let (compliantHTML, compliantOptions) be the result of invoking the get trusted type compliant input algorithm with this's relevant global object, html, options, and "ShadowRoot prependHTMLUnsafe".
  - 2. Filter and pre-insert HTML given this, this's first child, compliantHTML, compliantOptions, and Unsafe.

- [x] [NonDocumentTypeChildNode.beforeHTMLUnsafe(html, options)](https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-beforehtmlunsafe) — NonDocumentTypeChildNode's beforeHTMLUnsafe(html, options) method steps are:
  - 1. Let (compliantHTML, compliantOptions) be the result of invoking the get trusted type compliant input algorithm with this's relevant global object, html, options, and "Node beforeHTMLUnsafe".
  - 2. Let parent be the parent for HTML insertion given this.
  - 3. If parent is null, then return.
  - 4. Filter and pre-insert HTML given parent, this, compliantHTML, compliantOptions, and Unsafe.

- [x] [NonDocumentTypeChildNode.afterHTMLUnsafe(html, options)](https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-afterhtmlunsafe) — NonDocumentTypeChildNode's afterHTMLUnsafe(html, options) method steps are:
  - 1. Let (compliantHTML, compliantOptions) be the result of invoking the get trusted type compliant input algorithm with this's relevant global object, html, options, and "Node afterHTMLUnsafe".
  - 2. Let parent be the parent for HTML insertion given this.
  - 3. If parent is null, then return.
  - 4. Filter and pre-insert HTML given parent, this's next sibling, compliantHTML, compliantOptions, and Unsafe.

- [x] [NonDocumentTypeChildNode.replaceWithHTMLUnsafe(html, options)](https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-replacewithhtmlunsafe) — NonDocumentTypeChildNode's replaceWithHTMLUnsafe(html, options) method steps are:
  - 1. Let (compliantHTML, compliantOptions) be the result of invoking the get trusted type compliant input algorithm with this's relevant global object, html, options, and "Node replaceWithHTMLUnsafe".
  - 2. Let parent be the parent for HTML insertion given this.
  - 3. If parent is null, then return.
  - 4. Filter and replace with HTML given parent, this, compliantHTML, compliantOptions, and Unsafe.

- [x] [Document.parseHTML(html, options)](https://html.spec.whatwg.org/#dom-parsehtml) — The static parseHTML(html, options) method steps are:
  - 1. Let document be a new Document, whose content type is "text/html" and origin is this's relevant global object's associated Document's origin.
  - 2. Set document's allow declarative shadow roots to true.
  - 3. Let sanitizerConfig be the result of getting a sanitizer config from options given options and true.
  - 4. Assert: sanitizerConfig is non-null.
  - 5. Parse HTML from a string given document, html, and sanitizerConfig.
  - 6. Return document.

- [x] [Document.parseHTMLUnsafe(html, options)](https://html.spec.whatwg.org/#dom-parsehtmlunsafe) — The static parseHTMLUnsafe(html, options) method steps are:
  - 1. Let (compliantHTML, compliantOptions) be the result of invoking the get trusted type compliant input algorithm with the current global object, html, options, and "Document parseHTMLUnsafe".
  - 2. Let document be a new Document, whose content type is "text/html" and origin is this's relevant global object's associated Document's origin.
  - 3. Set document's allow declarative shadow roots to true.
  - 4. Let sanitizerConfig be the result of getting a sanitizer config from options given compliantOptions and false.
  - 5. Parse HTML from a string given document, compliantHTML, and sanitizerConfig.
  - 6. Return document.

#### [8.5.3 HTML serialization methods](https://html.spec.whatwg.org/#html-serialization-methods)

Status: reviewed (2026-11-01)
> `Element::GetHTML` and `ShadowRoot::GetHTML`.

- [x] [Element.getHTML(options)](https://html.spec.whatwg.org/#dom-element-gethtml) — Element's getHTML(options) method steps are to return the result of HTML fragment serialization algorithm with this, options["serializableShadowRoots"], and options["shadowRoots"].

- [x] [ShadowRoot.getHTML(options)](https://html.spec.whatwg.org/#dom-shadowroot-gethtml) — ShadowRoot's getHTML(options) method steps are to return the result of HTML fragment serialization algorithm with this, options["serializableShadowRoots"], and options["shadowRoots"].

#### [8.5.4 The innerHTML property](https://html.spec.whatwg.org/#the-innerhtml-property)

Status: reviewed (2026-11-01)
> The fragment parsing and serializing algorithm steps (`HTMLInsertionAlgorithms.cpp`, `Editing/Markup/Markup.cpp`); `innerHTML` follows the existing markup code (see `DIFFERENCES.md`).
<!-- definitions: fragment-parser-mode fragment-parser-mode-safe fragment-parser-mode-unsafe fragment-parser-mode-legacy -->

- [x] [fragment serializing algorithm steps](https://html.spec.whatwg.org/#fragment-serializing-algorithm-steps) — The fragment serializing algorithm steps, given an Element, Document, or DocumentFragment node and a boolean require well-formed, are:
  - 1. Let context document be node's node document.
  - 2. If context document is an HTML document, then return the result of HTML fragment serialization algorithm with node, false, and « ».
  - 3. Return the XML serialization of node given require well-formed.

- [x] [target for HTML insertion](https://html.spec.whatwg.org/#target-for-html-insertion) — To get the target for HTML insertion, given an Element element:
  - 1. If element is a template element, then return element's template contents.
  - 2. Return element.

- [x] [parent for HTML insertion](https://html.spec.whatwg.org/#parent-for-html-insertion) — To get the parent for HTML insertion, given a Node node:
  - 1. Let parent be node's parent.
  - 2. If parent is null, then return null.
  - 3. If parent is a template element, then throw a "HierarchyRequestError" DOMException.
  - 4. If parent is an Element, or a DocumentFragment with a non-null host, then return parent.
  - 5. Throw a "HierarchyRequestError" DOMException.

- [x] [filter and set HTML](https://html.spec.whatwg.org/#set-and-filter-html) — To filter and set HTML, given an Element or DocumentFragment target, a string html, a SetHTMLOptions or SetHTMLUnsafeOptions dictionary, or a TrustedHTMLParserOptions object options, and a fragment parser mode mode:
  - 1. Let fragment be the result of parsing HTML for insertion given target, html, options, and mode.
  - 2. If fragment is non-null, then replace all with fragment within target.

- [x] [filter and pre-insert HTML](https://html.spec.whatwg.org/#filter-and-pre-insert-html) — To filter and pre-insert HTML, given an Element or DocumentFragment parent, a Node-or-null child, a string html, a SetHTMLOptions or SetHTMLUnsafeOptions dictionary, or a TrustedHTMLParserOptions object options, and a fragment parser mode mode:
  - 1. If parent is an html element, then throw a "HierarchyRequestError" DOMException.
  - 2. Let fragment be the result of parsing HTML for insertion given parent, html, options, and mode.
  - 3. If fragment is non-null, then pre-insert fragment into parent before child.

- [x] [filter and replace with HTML](https://html.spec.whatwg.org/#filter-and-replace-with-html) — To filter and replace with HTML, given an Element or DocumentFragment parent, a Node node, a string html, a SetHTMLOptions or SetHTMLUnsafeOptions dictionary, or a TrustedHTMLParserOptions object options, and a fragment parser mode mode:
  - 1. If parent is an html element, then throw a "HierarchyRequestError" DOMException.
  - 2. Let fragment be the result of parsing HTML for insertion given parent, html, options, and mode.
  - 3. If fragment is non-null, then replace node with fragment within parent.

- [x] [parse HTML for insertion](https://html.spec.whatwg.org/#parse-html-for-insertion) — To parse HTML for insertion, given an Element or DocumentFragment target, a string html, a SetHTMLOptions or SetHTMLUnsafeOptions dictionary, or a TrustedHTMLParserOptions object options, and a fragment parser mode mode:
  - 1. If mode is Safe:
    - 1a. Let context be target if target is an Element; otherwise target's host.
    - 1b. Assert: context is non-null.
    - 1c. If all of the following are true: context's local name is "script"; and; context's namespace is the HTML namespace or the SVG namespace, then return null.
  - 2. Return the result of invoking the fragment parsing algorithm steps given target, html, options, and mode.

- [x] [fragment parsing algorithm steps](https://html.spec.whatwg.org/#fragment-parsing-algorithm-steps) — The fragment parsing algorithm steps, given an Element or DocumentFragment target, a string markup, a SetHTMLOptions or SetHTMLUnsafeOptions dictionary, or a TrustedHTMLParserOptions object options, and a fragment parser mode mode, are:
  - 1. If mode is Legacy and target's node document is an XML document, then return the result of invoking the XML fragment parsing algorithm given target and markup.
  - 2. Let safe be true if mode is Safe; otherwise false.
  - 3. Let sanitizerConfig be the result of getting a sanitizer config from options given options and safe.
  - 4. Let scriptingMode be Inert.
  - 5. Let runScripts be options's runScripts if options is a TrustedHTMLParserOptions; otherwise options["runScripts"] with default false.
  - 6. If runScripts is true:
    - 6a. Assert: mode is not Safe.
    - 6b. Set scriptingMode to Fragment.
  - 7. Let allowDeclarativeShadowRoots be false if mode is Legacy; otherwise true.
  - 8. Return the result of the HTML fragment parsing algorithm given target, markup, allowDeclarativeShadowRoots, scriptingMode, and sanitizerConfig.

- [x] [Element.innerHTML](https://html.spec.whatwg.org/#dom-element-innerhtml) — Element's innerHTML getter steps are to return the result of running fragment serializing algorithm steps with this and true.

- [x] [ShadowRoot.innerHTML](https://html.spec.whatwg.org/#dom-shadowroot-innerhtml) — ShadowRoot's innerHTML getter steps are to return the result of running fragment serializing algorithm steps with this and true.

- [x] [Element's innerHTML setter steps](https://html.spec.whatwg.org/#dom-shadowroot-innerhtml) — Element's innerHTML setter steps are:
  - 1. Let (compliantString, compliantOptions) be the result of invoking the get trusted type compliant input algorithm with this's relevant global object, the given value, a new SetHTMLUnsafeOptions dictionary, "Element innerHTML", this's node document's type, and false.
  - 2. Let target be the target for HTML insertion given this.
  - 3. Filter and set HTML given target, compliantString, compliantOptions, and Legacy.

- [x] [ShadowRoot's innerHTML setter steps](https://html.spec.whatwg.org/#dom-shadowroot-innerhtml) — ShadowRoot's innerHTML setter steps are:
  - 1. Let (compliantString, compliantOptions) be the result of invoking the get trusted type compliant input algorithm with this's relevant global object, the given value, a new SetHTMLUnsafeOptions dictionary, "ShadowRoot innerHTML", this's node document's type, and false.
  - 2. Filter and set HTML given this, compliantString, compliantOptions, and Legacy.

#### [8.5.5 The outerHTML property](https://html.spec.whatwg.org/#the-outerhtml-property)

Status: reviewed (2026-11-01)
> `Element::SetOuterHTML` and `OuterHTML`.

- [x] [Element.outerHTML](https://html.spec.whatwg.org/#dom-element-outerhtml) — Element's outerHTML getter steps are:
  - 1. Let element be a fictional node whose only child is this.
  - 2. Return the result of running fragment serializing algorithm steps with element and true.

- [x] [Element's outerHTML setter steps](https://html.spec.whatwg.org/#dom-element-outerhtml) — Element's outerHTML setter steps are:
  - 1. Let (compliantString, compliantOptions) be the result of invoking the get trusted type compliant input algorithm with this's relevant global object, the given value, a new SetHTMLUnsafeOptions dictionary, "Element outerHTML", this's node document's type, and false.
  - 2. Let parent be this's parent.
  - 3. If parent is null, return. There would be no way to obtain a reference to the nodes created even if the remaining steps were run.
  - 4. If parent is a Document, throw a "NoModificationAllowedError" DOMException.
  - 5. If parent is a DocumentFragment, set parent to the result of creating an element given this's node document, "body", and the HTML namespace.
  - 6. Let fragment be the result of invoking the fragment parsing algorithm steps given parent, compliantString, compliantOptions, and Legacy.
  - 7. Replace this with fragment within this's parent.

#### [8.5.6 The insertAdjacentHTML() method](https://html.spec.whatwg.org/#the-insertadjacenthtml%28%29-method)

Status: reviewed (2026-11-01)
> `Element::InsertAdjacentHTML`.

- [x] [Element.insertAdjacentHTML(position, string)](https://html.spec.whatwg.org/#dom-element-insertadjacenthtml) — Element's insertAdjacentHTML(position, string) method steps are:
  - 1. Let (compliantString, compliantOptions) be the result of invoking the get trusted type compliant input algorithm with this's relevant global object, string, a new SetHTMLUnsafeOptions dictionary, "Element insertAdjacentHTML", this's node document's type, and false.
  - 2. Let context be null.
  - 3. Use the first matching item from this list: If position is an ASCII case-insensitive match for the string "beforebegin"If position is an ASCII case-insensitive match for the string "afterend" If position is an ASCII case-insensitive match for the string "afterbegin"If position is an ASCII case-insensitive match for the string "beforeend"Set context to this.OtherwiseThrow a "SyntaxError" DOMExcept…
    - 3a. Set context to this's parent.
    - 3b. If context is null or a Document, throw a "NoModificationAllowedError" DOMException.
  - 4. If context is not an Element or all of the following are true: context's node document is an HTML document;; context's local name is "html"; and; context's namespace is the HTML namespace, then set context to the result of creating an element given this's node document, "body", and the HTML namespace.
  - 5. Let fragment be the result of invoking the fragment parsing algorithm steps given context, compliantString, compliantOptions, and Legacy.
  - 6. Use the first matching item from this list: If position is an ASCII case-insensitive match for the string "beforebegin" Insert fragment into this's parent before this. If position is an ASCII case-insensitive match for the string "afterbegin" Insert fragment into this before its first child. If position is an ASCII case-insensitive match for the string "beforeend" Append fragment to this. If posi…

#### [8.5.7 The createContextualFragment() method](https://html.spec.whatwg.org/#the-createcontextualfragment%28%29-method)

Status: reviewed (2026-11-01)
> Fixed: added `Range::CreateContextualFragment`.

- [x] [Range.createContextualFragment(string)](https://html.spec.whatwg.org/#dom-range-createcontextualfragment) — Range's createContextualFragment(string) method steps are:
  - 1. Let node be this's start node.
  - 2. Let (compliantString, compliantOptions) be the result of invoking the get trusted type compliant input algorithm with this's relevant global object, string, a new SetHTMLUnsafeOptions dictionary whose runScripts member is set to true and whose other members are set to their default values, "Range createContextualFragment", node's node document's type, and false.
  - 3. Let element be null.
  - 4. If node implements Element, set element to node.
  - 5. Otherwise, if node implements Text or Comment, set element to node's parent element.
  - 6. If element is null or all of the following are true: element's node document is an HTML document;; element's local name is "html"; and; element's namespace is the HTML namespace, then set element to the result of creating an element given node's node document, "body", and the HTML namespace.
  - 7. Return the result of invoking the fragment parsing algorithm steps given element, compliantString, compliantOptions, and Legacy.

#### [8.5.8 The XMLSerializer interface](https://html.spec.whatwg.org/#the-xmlserializer-interface)

Status: n/a (there is no `XMLSerializer`: `SerializeFragment` with `SerializationSyntax::XML` serializes a node as XML)

- [x] [new XMLSerializer()](https://html.spec.whatwg.org/#dom-xmlserializer-constructor) — The new XMLSerializer() constructor steps are to do nothing.

- [x] [XMLSerializer.serializeToString(root)](https://html.spec.whatwg.org/#dom-xmlserializer-serializetostring) — The serializeToString(root) method steps are:
  - 1. Return the XML serialization of root given false.

### [8.6 HTML sanitization](https://html.spec.whatwg.org/#html-sanitization)

Status: reviewed (2026-11-01)
> The sections below.

#### [8.6.1 Introduction](https://html.spec.whatwg.org/#introduction-12)

Status: reviewed (2026-11-01)
> No requirements.

##### [8.6.1.1 Safe and unsafe](https://html.spec.whatwg.org/#safe-and-unsafe)

Status: reviewed (2026-11-01)
> No requirements.

#### [8.6.2 The Sanitizer interface](https://html.spec.whatwg.org/#the-sanitizer-interface)

Status: reviewed (2026-11-01)
> `Sanitizer` (`Sanitizer.cpp`): the constructor, `get()` and the modifier methods, with the configure and canonicalize algorithms in `SanitizerAlgorithms.cpp`.
<!-- definitions: configuration -->

- [x] [Sanitizer.new Sanitizer(configuration)](https://html.spec.whatwg.org/#dom-sanitizer-constructor) — The new Sanitizer(configuration) constructor steps are:
  - 1. If configuration is a SanitizerPresets string:
    - 1a. Assert: configuration is "default".
    - 1b. Set configuration to the built-in safe default configuration.
  - 2. Configure this given configuration and true.

- [x] [configure (Sanitizer)](https://html.spec.whatwg.org/#configure-a-sanitizer) — To configure a Sanitizer sanitizer, given a dictionary configuration and a boolean permissiveDefaults:
  - 1. Let freshConfiguration be a clone of configuration.
  - 2. Canonicalize the configuration freshConfiguration with permissiveDefaults.
  - 3. If freshConfiguration is not valid, then throw a TypeError.
  - 4. Set sanitizer's configuration to freshConfiguration.

- [x] [canonicalize the configuration](https://html.spec.whatwg.org/#canonicalize-the-configuration) — To canonicalize the configuration SanitizerConfig configuration with a boolean permissiveDefaults:
  - 1. If neither configuration["elements"] nor configuration["removeElements"] exists, then set configuration["removeElements"] to an empty list.
  - 2. If neither configuration["attributes"] nor configuration["removeAttributes"] exists, then set configuration["removeAttributes"] to an empty list.
  - 3. If neither configuration["processingInstructions"] nor configuration["removeProcessingInstructions"] exists:
    - 3a. If permissiveDefaults is true, then set configuration["removeProcessingInstructions"] to an empty list.
    - 3b. Otherwise, set configuration["processingInstructions"] to an empty list.
  - 4. If configuration["elements"] exists:
    - 4a. Let newElements be « ».
    - 4b. For each element of configuration["elements"], append the result of canonicalizing element to newElements.
    - 4c. Set configuration["elements"] to newElements.
  - 5. If configuration["removeElements"] exists, then set configuration["removeElements"] to the result of canonicalizing configuration["removeElements"].
  - 6. If configuration["attributes"] exists, then set configuration["attributes"] to the result of canonicalizing configuration["attributes"].
  - 7. If configuration["removeAttributes"] exists, then set configuration["removeAttributes"] to the result of canonicalizing configuration["removeAttributes"].
  - 8. If configuration["replaceWithChildrenElements"] exists, then set configuration["replaceWithChildrenElements"] to the result of canonicalizing configuration["replaceWithChildrenElements"].
  - 9. If configuration["processingInstructions"] exists, then set configuration["processingInstructions"] to the result of canonicalizing configuration["processingInstructions"].
  - 10. If configuration["removeProcessingInstructions"] exists, then set configuration["removeProcessingInstructions"] to the result of canonicalizing configuration["removeProcessingInstructions"].
  - 11. If configuration["comments"] does not exist, then set it to permissiveDefaults.
  - 12. If configuration["attributes"] exists and configuration["dataAttributes"] does not exist, then set it to permissiveDefaults.
  - 13. If configuration["javascriptURLs"] does not exist, then set it to permissiveDefaults.

- [x] [canonicalize a sanitizer list](https://html.spec.whatwg.org/#canonicalize-a-sanitizer-list) — To canonicalize a sanitizer list list:
  - 1. Let newList be « ».
  - 2. For each item in list, append the result of canonicalizing item to newList.
  - 3. Return newList.

- [x] [canonicalize a processing instruction list](https://html.spec.whatwg.org/#canonicalize-a-processing-instruction-list) — To canonicalize a processing instruction list list:
  - 1. Let newList be « ».
  - 2. For each item in list, append the result of canonicalizing item to newList.
  - 3. Return newList.

- [x] [canonicalize a processing instruction](https://html.spec.whatwg.org/#canonicalize-a-processing-instruction) — To canonicalize a processing instruction given a SanitizerPI pi:
  - 1. If pi is a DOMString, then return «[ "target" → pi ]».
  - 2. Assert: pi is a dictionary and pi["target"] exists.
  - 3. Return «[ "target" → pi["target"] ]».

- [x] [canonicalize a sanitizer name](https://html.spec.whatwg.org/#canonicalize-a-sanitizer-name) — To canonicalize a sanitizer name given a DOMString or dictionary name, and a default namespace defaultNamespace (default null):
  - 1. If name is a DOMString, then return «[ "name" → name, "namespace" → defaultNamespace ]».
  - 2. Assert: name is a dictionary and both name["name"] and name["namespace"] exist.
  - 3. If name["namespace"] is the empty string, then set it to null.
  - 4. Return «[ "name" → name["name"], "namespace" → name["namespace"] ]».

- [x] [canonicalize a sanitizer element](https://html.spec.whatwg.org/#canonicalize-a-sanitizer-element) — To canonicalize a sanitizer element given a SanitizerElement element:
  - 1. Return the result of canonicalizing element with the HTML namespace as the default namespace.

- [x] [canonicalize a sanitizer element list](https://html.spec.whatwg.org/#canonicalize-a-sanitizer-element-list) — To canonicalize a sanitizer element list list:
  - 1. Let newList be « ».
  - 2. For each item in list, append the result of canonicalizing item to newList.
  - 3. Return newList.

- [x] [canonicalized intersection](https://html.spec.whatwg.org/#sanitizer-canonicalized-intersection) — To find the canonicalized intersection of lists A and B:
  - 1. Let setA be « ».
  - 2. Let setB be « ».
  - 3. For each entry of A, append the result of canonicalizing entry to setA.
  - 4. For each entry of B, append the result of canonicalizing entry to setB.
  - 5. Return the intersection of setA and setB.

- [x] [Sanitizer.get()](https://html.spec.whatwg.org/#dom-sanitizer-get) — The get() method steps are:
  - 1. Let config be this's configuration.
  - 2. Assert: config is valid.
  - 3. If config["elements"] exists:
    - 3a. For each element of config["elements"]:
      - 3a.i. If element["attributes"] exists, then set element["attributes"] to the result of sorting element["attributes"], with compare sanitizer items.
      - 3a.ii. If element["removeAttributes"] exists, then set element["removeAttributes"] to the result of sorting element["removeAttributes"], with compare sanitizer items.
    - 3b. Set config["elements"] to the result of sorting config["elements"], with compare sanitizer items.
  - 4. Otherwise:
    - 4a. Set config["removeElements"] to the result of sorting config["removeElements"], with compare sanitizer items.
  - 5. If config["replaceWithChildrenElements"] exists, then set config["replaceWithChildrenElements"] to the result of sorting config["replaceWithChildrenElements"], with compare sanitizer items.
  - 6. If config["processingInstructions"] exists, then set config["processingInstructions"] to the result of sorting config["processingInstructions"], with piA["target"] being code unit less than piB["target"].
  - 7. Otherwise:
    - 7a. Set config["removeProcessingInstructions"] to the result of sorting config["removeProcessingInstructions"], with piA["target"] being code unit less than piB["target"].
  - 8. If config["attributes"] exists, then set config["attributes"] to the result of sorting config["attributes"] given compare sanitizer items.
  - 9. Otherwise:
    - 9a. Set config["removeAttributes"] to the result of sorting config["removeAttributes"] given compare sanitizer items.
  - 10. Return config.

- [x] [Sanitizer.allowElement(element)](https://html.spec.whatwg.org/#dom-sanitizer-allowelement) — The allowElement(element) method steps are:
  - 1. Let configuration be this's configuration.
  - 2. Assert: configuration is valid.
  - 3. Set element to the result of canonicalizing element.
  - 4. If configuration["elements"] exists:
    - 4a. Let modified be the result of removing element from configuration["replaceWithChildrenElements"].
    - 4b. If configuration["attributes"] exists:
      - 4b.i. If element["attributes"] exists:
        - 4b.i.A. Set element["attributes"] to the result of creating a set from element["attributes"].
        - 4b.i.B. Set element["attributes"] to the difference of element["attributes"] and configuration["attributes"].
        - 4b.i.C. If configuration["dataAttributes"] is true, then remove all items item from element["attributes"] where item is a custom data attribute.
      - 4b.ii. If element["removeAttributes"] exists:
        - 4b.ii.A. Set element["removeAttributes"] to the result of creating a set from element["removeAttributes"].
        - 4b.ii.B. Set element["removeAttributes"] to the intersection of element["removeAttributes"] and configuration["attributes"].
    - 4c. Otherwise:
      - 4c.i. If element["attributes"] exists:
        - 4c.i.A. Set element["attributes"] to the result of creating a set from element["attributes"].
        - 4c.i.B. Set element["attributes"] to the difference of element["attributes"] and element["removeAttributes"] with default « ».
        - 4c.i.C. Remove element["removeAttributes"].
        - 4c.i.D. Set element["attributes"] to the difference of element["attributes"] and configuration["removeAttributes"].
      - 4c.ii. If element["removeAttributes"] exists:
        - 4c.ii.A. Set element["removeAttributes"] to the result of creating a set from element["removeAttributes"].
        - 4c.ii.B. Set element["removeAttributes"] to the difference of element["removeAttributes"] and configuration["removeAttributes"].
    - 4d. If configuration["elements"] does not contain element:
      - 4d.i. Append element to configuration["elements"].
      - 4d.ii. Return true.
    - 4e. Let currentElement be the item in configuration["elements"] whose name member is element's name member and whose namespace member is element's namespace member.
    - 4f. If element is equal to currentElement, then return modified.
    - 4g. Remove element from configuration["elements"].
    - 4h. Append element to configuration["elements"].
    - 4i. Return true.
  - 5. Otherwise:
    - 5a. If element["attributes"] exists or element["removeAttributes"] with default « » is not empty, then return false.
    - 5b. Let modified be the result of removing element from configuration["replaceWithChildrenElements"].
    - 5c. If configuration["removeElements"] does not contain element, then return modified.
    - 5d. Remove element from configuration["removeElements"].
    - 5e. Return true.

- [x] [Sanitizer.removeElement(element)](https://html.spec.whatwg.org/#dom-sanitizer-removeelement) — The removeElement(element) method steps are to return the result of removing element from this's configuration.

- [x] [Sanitizer.replaceElementWithChildren(element)](https://html.spec.whatwg.org/#dom-sanitizer-replaceelementwithchildren) — The replaceElementWithChildren(element) method steps are:
  - 1. Let configuration be this's configuration.
  - 2. Assert: configuration is valid.
  - 3. Set element to the result of canonicalizing element.
  - 4. If the built-in non-replaceable elements list contains element, then return false.
  - 5. Let modified be the result of removing element from configuration["elements"].
  - 6. If removing element from configuration["removeElements"] is true, then set modified to true.
  - 7. If configuration["replaceWithChildrenElements"] does not contain element:
    - 7a. Append element to configuration["replaceWithChildrenElements"].
    - 7b. Return true.
  - 8. Return modified.

- [x] [Sanitizer.allowAttribute(attribute)](https://html.spec.whatwg.org/#dom-sanitizer-allowattribute) — The allowAttribute(attribute) method steps are:
  - 1. Let configuration be this's configuration.
  - 2. Assert: configuration is valid.
  - 3. Set attribute to the result of canonicalizing attribute.
  - 4. If configuration["attributes"] exists:
    - 4a. If configuration["dataAttributes"] is true and attribute is a custom data attribute, then return false.
    - 4b. If configuration["attributes"] contains attribute, then return false.
    - 4c. If configuration["elements"] exists:
      - 4c.i. For each element in configuration["elements"]:
        - 4c.i.A. If element["attributes"] with default « » contains attribute, then remove attribute from element["attributes"].
    - 4d. Append attribute to configuration["attributes"].
    - 4e. Return true.
  - 5. Otherwise:
    - 5a. If configuration["removeAttributes"] does not contain attribute, then return false.
    - 5b. Remove attribute from configuration["removeAttributes"].
    - 5c. Return true.

- [x] [Sanitizer.removeAttribute(attribute)](https://html.spec.whatwg.org/#dom-sanitizer-removeattribute) — The removeAttribute(attribute) method steps are to return the result of removing attribute from this's configuration.

- [x] [Sanitizer.setComments(allow)](https://html.spec.whatwg.org/#dom-sanitizer-setcomments) — The setComments(allow) method steps are:
  - 1. Let configuration be this's configuration.
  - 2. Assert: configuration is valid.
  - 3. If configuration["comments"] is allow, then return false.
  - 4. Set configuration["comments"] to allow.
  - 5. Return true.

- [x] [Sanitizer.setDataAttributes(allow)](https://html.spec.whatwg.org/#dom-sanitizer-setdataattributes) — The setDataAttributes(allow) method steps are:
  - 1. Let configuration be this's configuration.
  - 2. Assert: configuration is valid.
  - 3. If configuration["attributes"] does not exist, then return false.
  - 4. If configuration["dataAttributes"] exists and is equal to allow, then return false.
  - 5. If allow is true:
    - 5a. If configuration["elements"] exists:
      - 5a.i. For each element of configuration["elements"]:
        - 5a.i.A. If element["attributes"] exists, then remove all items item from element["attributes"] where item is a custom data attribute.
    - 5b. Remove all items item from configuration["attributes"] where item is a custom data attribute.
  - 6. Set configuration["dataAttributes"] to allow.
  - 7. Return true.

- [x] [Sanitizer.setJavascriptURLs(allow)](https://html.spec.whatwg.org/#dom-sanitizer-setjavascripturls) — The setJavascriptURLs(allow) method steps are:
  - 1. Let configuration be this's configuration.
  - 2. Assert: configuration is valid.
  - 3. If configuration["javascriptURLs"] is allow, then return false.
  - 4. Set configuration["javascriptURLs"] to allow.
  - 5. Return true.

- [x] [Sanitizer.allowProcessingInstruction(pi)](https://html.spec.whatwg.org/#dom-sanitizer-allowprocessinginstruction) — The allowProcessingInstruction(pi) method steps are:
  - 1. Let configuration be this's configuration.
  - 2. Assert: configuration is valid.
  - 3. Set pi to the result of canonicalizing pi.
  - 4. If configuration["processingInstructions"] exists:
    - 4a. If configuration["processingInstructions"] contains pi, then return false.
    - 4b. Append pi to configuration["processingInstructions"].
    - 4c. Return true.
  - 5. Otherwise:
    - 5a. If configuration["removeProcessingInstructions"] contains pi:
      - 5a.i. Remove pi from configuration["removeProcessingInstructions"].
      - 5a.ii. Return true.
    - 5b. Return false.

- [x] [Sanitizer.removeProcessingInstruction(pi)](https://html.spec.whatwg.org/#dom-sanitizer-removeprocessinginstruction) — The removeProcessingInstruction(pi) method steps are:
  - 1. Let configuration be this's configuration.
  - 2. Assert: configuration is valid.
  - 3. Set pi to the result of canonicalizing pi.
  - 4. If configuration["processingInstructions"] exists:
    - 4a. If configuration["processingInstructions"] contains pi:
      - 4a.i. Remove pi from configuration["processingInstructions"].
      - 4a.ii. Return true.
    - 4b. Return false.
  - 5. Otherwise:
    - 5a. If configuration["removeProcessingInstructions"] contains pi, then return false.
    - 5b. Append pi to configuration["removeProcessingInstructions"].
    - 5c. Return true.

- [x] [Sanitizer.removeUnsafe()](https://html.spec.whatwg.org/#dom-sanitizer-removeunsafe) — The removeUnsafe() method steps are to return the result of removing unsafe from this's configuration.

#### [8.6.3 Sanitizer configuration](https://html.spec.whatwg.org/#sanitizer-configuration)

Status: reviewed (2026-11-01)
> `SanitizerConfig.hpp`.

##### [8.6.3.1 Configuration invariants](https://html.spec.whatwg.org/#configuration-invariants)

Status: reviewed (2026-11-01)
> `IsValid` in `SanitizerAlgorithms.cpp`.
<!-- definitions: dom-sanitizerpresets-default -->

#### [8.6.4 Sanitization algorithms](https://html.spec.whatwg.org/#sanitization-algorithms)

Status: reviewed (2026-11-01)
> `SanitizerAlgorithms.cpp`: set and filter HTML, sanitize, the core sanitize steps and the allow checks.
<!-- definitions: sanitizer-action sanitizer-action-keep sanitizer-action-remove sanitizer-action-replace-with-children -->

- [x] [get a sanitizer config from options](https://html.spec.whatwg.org/#get-a-sanitizer-config-from-options) — To get a sanitizer config from options given a SetHTMLOptions, SetHTMLUnsafeOptions, or ParseHTMLUnsafeOptions dictionary, or a TrustedHTMLParserOptions object options, and a boolean safe:
  - 1. Let sanitizerInput be null.
  - 2. If options is a TrustedHTMLParserOptions:
    - 2a. If options's sanitizer configuration is null:
      - 2a.i. Assert: safe is false.
      - 2a.ii. Return null.
    - 2b. Set sanitizerInput to options's sanitizer configuration.
  - 3. Otherwise, if options["sanitizer"] exists, then set sanitizerInput to options["sanitizer"].
  - 4. Otherwise, if safe is false, then return null.
  - 5. Otherwise, set sanitizerInput to "default".
  - 6. Assert: sanitizerInput is either a Sanitizer instance, a SanitizerPresets member, or a SanitizerConfig dictionary.
  - 7. If sanitizerInput is a string:
    - 7a. Assert: sanitizerInput is "default".
    - 7b. Set sanitizerInput to the built-in safe default configuration.
  - 8. If sanitizerInput is a Sanitizer, then set sanitizerInput to sanitizerInput's configuration.
  - 9. Assert: sanitizerInput is a dictionary.
  - 10. Let sanitizer be a new Sanitizer object.
  - 11. Let permissiveDefaults be true if safe is false; false otherwise.
  - 12. Configure sanitizer given sanitizerInput and permissiveDefaults.
  - 13. Let configuration be sanitizer's configuration.
  - 14. If safe is true, then remove unsafe from configuration.
  - 15. Assert: configuration is valid.
  - 16. Return configuration.

- [x] [contains a javascript: URL](https://html.spec.whatwg.org/#contains-a-javascript:-url) — To determine whether a string value contains a javascript: URL:
  - 1. Let url be the result of running the basic URL parser on value.
  - 2. If url is failure, then return false.
  - 3. Return true if url's scheme is "javascript", and false otherwise.

- [x] [sanitizer config allows comments](https://html.spec.whatwg.org/#sanitizer-config-allows-comments) — To check if a sanitizer config allows comments given a SanitizerConfig configuration, return true if configuration["comments"] is true; otherwise false.

- [x] [sanitizer config allows processing instruction target](https://html.spec.whatwg.org/#sanitizer-config-allows-processing-instruction-target) — To check if a sanitizer config allows processing instruction target given a string piTarget and a SanitizerConfig configuration:
  - 1. Let pi be «[ "target" → piTarget ]».
  - 2. If configuration["processingInstructions"] exists, then return whether configuration["processingInstructions"] contains pi.
  - 3. If configuration["removeProcessingInstructions"] exists and configuration["removeProcessingInstructions"] contains pi, then return false.
  - 4. Return true.

- [x] [get the sanitizer action for an element name](https://html.spec.whatwg.org/#get-the-sanitizer-action-for-an-element-name) — To get the sanitizer action for an element name given a string localName, a string or null namespace, and a SanitizerConfig configuration:
  - 1. Let elementName be «[ "name" → localName, "namespace" → namespace ]».
  - 2. If configuration["replaceWithChildrenElements"] exists and configuration["replaceWithChildrenElements"] contains elementName, then return "Replace with children".
  - 3. If configuration["elements"] exists:
    - 3a. If there is no item in configuration["elements"] whose name member is localName and whose namespace member is namespace, then return "Remove".
  - 4. Otherwise, if configuration["removeElements"] contains elementName, then return "Remove".
  - 5. Return "Keep".

- [x] [sanitizer config allows an attribute](https://html.spec.whatwg.org/#sanitizer-config-allows-an-attribute) — To check if a sanitizer config allows an attribute given a string localName, a string or null namespace, a string value, a string elementLocalName, a string or null elementNamespace, and a SanitizerConfig configuration:
  - 1. Let attrName be «[ "name" → localName, "namespace" → namespace ]».
  - 2. Let elementName be «[ "name" → elementLocalName, "namespace" → elementNamespace ]».
  - 3. Let elementWithLocalAttributes be «[ ]».
  - 4. If configuration["elements"] exists and there is an item element in configuration["elements"] whose name member is elementLocalName and whose namespace member is elementNamespace, then set elementWithLocalAttributes to element.
  - 5. If elementWithLocalAttributes["removeAttributes"] exists and elementWithLocalAttributes["removeAttributes"] contains attrName, then return false.
  - 6. Otherwise, if configuration["attributes"] exists:
    - 6a. If configuration["attributes"] does not contain attrName and elementWithLocalAttributes["attributes"] with default « » does not contain attrName, and if "data-" is not a code unit prefix of localName or namespace is not null or configuration["dataAttributes"] is not true, then return false.
  - 7. Otherwise:
    - 7a. If elementWithLocalAttributes["attributes"] exists and elementWithLocalAttributes["attributes"] does not contain attrName, then return false.
    - 7b. Otherwise, if configuration["removeAttributes"] exists and configuration["removeAttributes"] contains attrName, then return false.
  - 8. If configuration["javascriptURLs"] is true, then return true.
  - 9. If the pair (elementName, attrName) matches an entry in the built-in navigating URL attributes list, and if value contains a javascript: URL, then return false.
  - 10. If elementNamespace is the MathML namespace, localName is "href", namespace is null or the XLink namespace, and value contains a javascript: URL, then return false.
  - 11. If the built-in animating URL attributes list contains the pair (elementName, attrName), and value is "href" or "xlink:href", then return false.
  - 12. Return true.

- [x] [remove an element](https://html.spec.whatwg.org/#remove-an-element-from-sanitizer-config) — To remove an element element from a SanitizerConfig configuration:
  - 1. Assert: configuration is valid.
  - 2. Set element to the result of canonicalizing element.
  - 3. Let modified be the result of removing element from configuration["replaceWithChildrenElements"].
  - 4. If configuration["elements"] exists:
    - 4a. If configuration["elements"] contains element:
      - 4a.i. Remove element from configuration["elements"].
      - 4a.ii. Return true.
    - 4b. Return modified.
  - 5. Otherwise:
    - 5a. If configuration["removeElements"] contains element, then return modified.
    - 5b. Append element to configuration["removeElements"].
    - 5c. Return true.

- [x] [remove an attribute](https://html.spec.whatwg.org/#remove-an-attribute-from-sanitizer-config) — To remove an attribute attribute from a SanitizerConfig configuration:
  - 1. Assert: configuration is valid.
  - 2. Set attribute to the result of canonicalizing attribute.
  - 3. If configuration["attributes"] exists:
    - 3a. Let modified be the result of removing attribute from configuration["attributes"].
    - 3b. If configuration["elements"] exists:
      - 3b.i. For each element of configuration["elements"]:
        - 3b.i.A. If element["attributes"] with default « » contains attribute:
          - 3b.i.A.1. Set modified to true.
          - 3b.i.A.2. Remove attribute from element["attributes"].
        - 3b.i.B. If element["removeAttributes"] with default « » contains attribute:
          - 3b.i.B.1. Assert: modified is true.
          - 3b.i.B.2. Remove attribute from element["removeAttributes"].
    - 3c. Return modified.
  - 4. Otherwise:
    - 4a. If configuration["removeAttributes"] contains attribute, then return false.
    - 4b. If configuration["elements"] exists:
      - 4b.i. For each element in configuration["elements"]:
        - 4b.i.A. If element["attributes"] with default « » contains attribute, then remove attribute from element["attributes"].
        - 4b.i.B. If element["removeAttributes"] with default « » contains attribute, then remove attribute from element["removeAttributes"].
    - 4c. Append attribute to configuration["removeAttributes"].
    - 4d. Return true.

- [x] [remove unsafe](https://html.spec.whatwg.org/#remove-unsafe) — To remove unsafe from a SanitizerConfig configuration:
  - 1. Assert: configuration is valid.
  - 2. Let result be false.
  - 3. For each element in built-in safe baseline configuration["removeElements"]:
    - 3a. If removing element from configuration is true, then set result to true.
  - 4. For each attribute in built-in safe baseline configuration["removeAttributes"]:
    - 4a. If removing attribute from configuration is true, then set result to true.
  - 5. For each attribute that is an event handler content attribute:
    - 5a. If removing attribute from configuration is true, then set result to true.
  - 6. If configuration["javascriptURLs"] is true:
    - 6a. Set result to true.
    - 6b. Set configuration["javascriptURLs"] to false.
  - 7. Return result.

- [x] [compare sanitizer items](https://html.spec.whatwg.org/#compare-sanitizer-items) — To compare sanitizer items itemA and itemB:
  - 1. Let namespaceA be itemA["namespace"].
  - 2. Let namespaceB be itemB["namespace"].
  - 3. If namespaceA is null:
    - 3a. If namespaceB is not null, then return true.
  - 4. Otherwise:
    - 4a. If namespaceB is null, then return false.
    - 4b. If namespaceA is code unit less than namespaceB, then return true.
    - 4c. If namespaceA is not namespaceB, then return false.
  - 5. If itemA["name"] is code unit less than itemB["name"], then return true.
  - 6. Return false.

- [x] [canonicalize](https://html.spec.whatwg.org/#canonicalize-a-sanitizer-element-with-attributes) — To canonicalize a SanitizerElementWithAttributes element:
  - 1. Let result be the result of canonicalizing element.
  - 2. If element is a dictionary:
    - 2a. If element["attributes"] exists, then set result["attributes"] to the result of canonicalizing element["attributes"].
    - 2b. If element["removeAttributes"] exists, then set result["removeAttributes"] to the result of canonicalizing element["removeAttributes"].
  - 3. If neither result["attributes"] nor result["removeAttributes"] exists, then set result["removeAttributes"] to an empty list.
  - 4. Return result.

- [x] [valid](https://html.spec.whatwg.org/#dom-sanitizerconfig-valid) — To determine whether a canonical SanitizerConfig config is valid:
  - 1. Assert: config["elements"] exists or config["removeElements"] exists.
  - 2. If config["elements"] exists and config["removeElements"] exists, then return false.
  - 3. Assert: Either config["processingInstructions"] exists or config["removeProcessingInstructions"] exists.
  - 4. If config["processingInstructions"] exists and config["removeProcessingInstructions"] exists, then return false.
  - 5. Assert: Either config["attributes"] exists or config["removeAttributes"] exists.
  - 6. If config["attributes"] exists and config["removeAttributes"] exists, then return false.
  - 7. Assert: All SanitizerElementNamespaceWithAttributes, SanitizerElementNamespace, SanitizerProcessingInstruction, and SanitizerAttributeNamespace items in config are canonical, meaning they have been run through canonicalizing, as appropriate.
  - 8. If config["elements"] exists:
    - 8a. If config["elements"] has duplicates, then return false.
  - 9. Otherwise:
    - 9a. If config["removeElements"] has duplicates, then return false.
  - 10. If config["replaceWithChildrenElements"] exists and has duplicates, then return false.
  - 11. If config["processingInstructions"] exists:
    - 11a. If config["processingInstructions"] has duplicates, then return false.
  - 12. Otherwise:
    - 12a. If config["removeProcessingInstructions"] has duplicates, then return false.
  - 13. If config["attributes"] exists:
    - 13a. If config["attributes"] has duplicates, then return false.
  - 14. Otherwise:
    - 14a. If config["removeAttributes"] has duplicates, then return false.
  - 15. If config["replaceWithChildrenElements"] exists:
    - 15a. For each element of config["replaceWithChildrenElements"]:
      - 15a.i. If the built-in non-replaceable elements list contains element, then return false.
    - 15b. If config["elements"] exists:
      - 15b.i. If the intersection of config["elements"] and config["replaceWithChildrenElements"] is not empty, then return false.
    - 15c. Otherwise:
      - 15c.i. If the intersection of config["removeElements"] and config["replaceWithChildrenElements"] is not empty, then return false.
  - 16. If config["attributes"] exists:
    - 16a. Assert: config["dataAttributes"] exists.
    - 16b. If config["elements"] exists:
      - 16b.i. For each element of config["elements"]:
        - 16b.i.A. If element["attributes"] exists and element["attributes"] has duplicates, then return false.
        - 16b.i.B. If element["removeAttributes"] exists and element["removeAttributes"] has duplicates, then return false.
        - 16b.i.C. If the intersection of config["attributes"] and element["attributes"] with default « » is not empty, then return false.
        - 16b.i.D. If element["removeAttributes"] with default « » is not a subset of config["attributes"], then return false.
        - 16b.i.E. If config["dataAttributes"] is true and element["attributes"] contains a custom data attribute, then return false.
    - 16c. If config["dataAttributes"] is true and config["attributes"] contains a custom data attribute, then return false.
  - 17. Otherwise:
    - 17a. If config["elements"] exists:
      - 17a.i. For each element of config["elements"]:
        - 17a.i.A. If element["attributes"] exists and element["removeAttributes"] exists, then return false.
        - 17a.i.B. If element["attributes"] exists and element["attributes"] has duplicates, then return false.
        - 17a.i.C. If element["removeAttributes"] exists and element["removeAttributes"] has duplicates, then return false.
        - 17a.i.D. If the intersection of config["removeAttributes"] and element["attributes"] with default « » is not empty, then return false.
        - 17a.i.E. If the intersection of config["removeAttributes"] and element["removeAttributes"] with default « » is not empty, then return false.
    - 17b. If config["dataAttributes"] exists, then return false.
  - 18. Return true.

#### [8.6.5 Sanitization constants](https://html.spec.whatwg.org/#sanitization-constants)

Status: reviewed (2026-11-01)
> `SanitizerConstants.cpp`: the built-in safe default and baseline configurations, and the URL attribute lists, checked against the specification's lists.
<!-- definitions: sanitization-category sanitizer-category-default sanitizer-category-unsafe sanitizer-uncategorized built-in-safe-baseline-configuration built-in-safe-default-configuration built-in-navigating-url-attributes-list navigating-url-attributes built-in-animating-url-attributes-list built-in-non-replaceable-elements-list -->

#### [8.6.6 Security considerations](https://html.spec.whatwg.org/#sanitizer-security-considerations)

Status: reviewed (2026-11-01)
> No requirements.

##### [8.6.6.1 Server-side reflected and stored XSS](https://html.spec.whatwg.org/#sanitizer-security-server-reflected)

Status: reviewed (2026-11-01)
> No requirements.

##### [8.6.6.2 DOM clobbering](https://html.spec.whatwg.org/#sanitizer-security-dom-clobbering)

Status: reviewed (2026-11-01)
> No requirements.

##### [8.6.6.3 XSS with script gadgets](https://html.spec.whatwg.org/#sanitizer-security-script-gadgets)

Status: reviewed (2026-11-01)
> No requirements.

##### [8.6.6.4 Mutation XSS](https://html.spec.whatwg.org/#sanitizer-security-mxss)

Status: reviewed (2026-11-01)
> No requirements.
