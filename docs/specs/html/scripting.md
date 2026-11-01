# HTML review checklist: 4.12 Scripting

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 33 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html scripting` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [4.12 Scripting](https://html.spec.whatwg.org/#scripting-3)

Status: unreviewed

#### [4.12.1 The script element](https://html.spec.whatwg.org/#the-script-element)

Status: unreviewed
<!-- definitions: attr-script-type data-block attr-script-src attr-script-nomodule attr-script-async attr-script-defer attr-script-blocking attr-script-crossorigin attr-script-referrerpolicy attr-script-integrity attr-script-fetchpriority -->

- [ ] [HTMLScriptElement.src](https://html.spec.whatwg.org/#dom-script-src) — The src getter steps are:
  - 1. Let attribute be the result of getting an attribute by namespace and local name given null, src's local name, and this.
  - 2. If attribute is null, then return the empty string.
  - 3. Let urlString be the result of encoding-parsing-and-serializing a URL given attribute's value, relative to this's node document.
  - 4. If urlString is not failure, then return urlString.
  - 5. Return attribute's value, converted to a scalar value string.

- [ ] [The src setter steps](https://html.spec.whatwg.org/#dom-script-src) — The src setter steps are:
  - 1. Let compliantScriptURL be the result of invoking the get trusted type compliant string algorithm with TrustedScriptURL, this's relevant global object, the given value, "HTMLScriptElement src", and "script".
  - 2. Set an attribute value given this, src's local name, and compliantScriptURL.

- [ ] [HTMLScriptElement.crossOrigin](https://html.spec.whatwg.org/#dom-script-crossorigin) — The crossOrigin IDL attribute must reflect the crossorigin content attribute, limited to only known values.

- [ ] [HTMLScriptElement.referrerPolicy](https://html.spec.whatwg.org/#dom-script-referrerpolicy) — The referrerPolicy IDL attribute must reflect the referrerpolicy content attribute, limited to only known values.

- [ ] [HTMLScriptElement.fetchPriority](https://html.spec.whatwg.org/#dom-script-fetchpriority) — The fetchPriority IDL attribute must reflect the fetchpriority content attribute, limited to only known values.

- [ ] [HTMLScriptElement.async](https://html.spec.whatwg.org/#dom-script-async) — The async getter steps are:
  - 1. If this's force async is true, then return true.
  - 2. If this's async content attribute is present, then return true.
  - 3. Return false.

- [ ] [The async setter steps](https://html.spec.whatwg.org/#dom-script-async) — The async setter steps are:
  - 1. Set this's force async to false.
  - 2. If the given value is true, then set this's async content attribute to the empty string.
  - 3. Otherwise, remove this's async content attribute.

- [ ] [HTMLScriptElement.innerText](https://html.spec.whatwg.org/#dom-script-innertext) — The innerText getter steps are to return the result of running get the text steps with this.

- [ ] [The innerText setter steps](https://html.spec.whatwg.org/#dom-script-innertext) — The innerText setter steps are:
  - 1. Let compliantScript be the result of invoking the get trusted type compliant string algorithm with TrustedScript, this's relevant global object, the given value, "HTMLScriptElement innerText", and "script".
  - 2. Update script trustedness for this given compliantScript.
  - 3. Run set the inner text steps with this and compliantScript.

- [ ] [HTMLScriptElement.textContent](https://html.spec.whatwg.org/#dom-script-textcontent) — The textContent getter steps are to return the result of running get text content with this.

- [ ] [The textContent setter steps](https://html.spec.whatwg.org/#dom-script-textcontent) — The textContent setter steps are:
  - 1. Let value be the given value.
  - 2. If value is null, then set value to the empty string.
  - 3. Let compliantScript be the result of invoking the get trusted type compliant string algorithm with TrustedScript, this's relevant global object, value, "HTMLScriptElement textContent", and "script".
  - 4. Update script trustedness for this given compliantScript.
  - 5. Run set text content with this and compliantScript.

- [ ] [HTMLScriptElement.text](https://html.spec.whatwg.org/#dom-script-text) — The text getter steps are to return this's child text content.

- [ ] [The text setter steps](https://html.spec.whatwg.org/#dom-script-text) — The text setter steps are:
  - 1. Let compliantScript be the result of invoking the get trusted type compliant string algorithm with TrustedScript, this's relevant global object, the given value, "HTMLScriptElement text", and "script".
  - 2. Update script trustedness for this given compliantScript.
  - 3. String replace all with compliantScript within this.

- [ ] [HTMLScriptElement.supports(type)](https://html.spec.whatwg.org/#dom-script-supports) — The static supports(type) method steps are:
  - 1. If type is "classic", then return true.
  - 2. If type is "module", then return true.
  - 3. If type is "importmap", then return true.
  - 4. If type is "speculationrules", then return true.
  - 5. Return false.

##### [4.12.1.1 Processing model](https://html.spec.whatwg.org/#script-processing-model)

Status: n/a (scripts don't run)
<!-- definitions: parser-document parser-inserted preparation-time-document script-force-async concept-script-external ready-to-be-parser-executed already-started concept-script-delay-load concept-script-type concept-script-result pending-parsing-blocking-script set-of-scripts-that-will-execute-as-soon-as-possible list-of-scripts-that-will-execute-in-order-as-soon-as-possible list-of-scripts-that-will-execute-when-the-document-has-finished-parsing -->

- [x] [steps to run when the result is ready](https://html.spec.whatwg.org/#steps-to-run-when-the-result-is-ready) — A script element has steps to run when the result is ready, which are a series of steps or null, initially null. To mark as ready a script element el given a result:
  - 1. Set el's result to result.
  - 2. If el's steps to run when the result is ready are not null, then run them.
  - 3. Set el's steps to run when the result is ready to null.
  - 4. Set el's delaying the load event to false.

- [x] [The script HTML element post-connection steps, given insertedNode,](https://html.spec.whatwg.org/#the-script-element) — The script HTML element post-connection steps, given insertedNode, are:
  - 1. If insertedNode is parser-inserted, then return.
  - 2. Prepare the script element given insertedNode.

- [x] [The script HTML element removing steps given removedNode](https://html.spec.whatwg.org/#the-script-element) — The script HTML element removing steps given removedNode are:
  - 1. If removedNode's result is a speculation rules parse result:
    - 1a. Unregister speculation rules given removedNode's relevant global object and removedNode's result.
    - 1b. Set removedNode's already started to false.
    - 1c. Set removedNode's result to null.

- [x] [The script children changed steps given changedNode](https://html.spec.whatwg.org/#the-script-element) — The script children changed steps given changedNode are:
  - 1. If the script element is not connected, then return.
  - 2. Run the script HTML element post-connection steps, given changedNode.

- [x] [The following attribute change steps, given element, localName, oldValue, value…](https://html.spec.whatwg.org/#mark-as-ready) — The following attribute change steps, given element, localName, oldValue, value, and namespace, are used for all script elements:
  - 1. If namespace is not null, then return.
  - 2. If localName is src, value is not null, and element is connected, then run the script HTML element post-connection steps, given element.

- [x] [prepare the script element](https://html.spec.whatwg.org/#prepare-the-script-element) — To prepare the script element given a script element el:
  - 1. If el's already started is true, then return.
  - 2. Let parser document be el's parser document.
  - 3. Set el's parser document to null.
  - 4. If parser document is non-null and el does not have an async attribute, then set el's force async to true.
  - 5. Let source text be el's child text content.
  - 6. If el has no src attribute, and source text is the empty string, then return.
  - 7. If el is not connected, then return.
  - 8. If any of the following are true: el has a type attribute whose value is the empty string;; el has no type attribute but it has a language attribute and that attribute's value is the empty string; or; el has neither a type attribute nor a language attribute, then let the script block's type string for this script element be "text/javascript". Otherwise, if el has a type attribute, then let the sc…
  - 9. If the script block's type string is a JavaScript MIME type essence match, or el has a type attribute whose value with leading and trailing ASCII whitespace stripped is a JavaScript MIME type essence match, then set el's type to "classic".
  - 10. Otherwise, if the script block's type string is an ASCII case-insensitive match for the string "module", then set el's type to "module".
  - 11. Otherwise, if the script block's type string is an ASCII case-insensitive match for the string "importmap", then set el's type to "importmap".
  - 12. Otherwise, if the script block's type string is an ASCII case-insensitive match for the string "speculationrules", then set el's type to "speculationrules".
  - 13. Otherwise, return. (No script is executed, and el's type is left as null.)
  - 14. If parser document is non-null, then set el's parser document back to parser document and set el's force async to false.
  - 15. Set el's already started to true.
  - 16. Set el's preparation-time document to its node document.
  - 17. If parser document is non-null, and parser document is not equal to el's preparation-time document, then return.
  - 18. If scripting is disabled for el, then return.
  - 19. If el has a nomodule content attribute and its type is "classic", then return.
  - 20. Let cspType be "script speculationrules" if el's type is "speculationrules"; otherwise, "script".
  - 21. If el does not have a src content attribute, and the Should element's inline behavior be blocked by Content Security Policy? algorithm returns "Blocked" when given el, cspType, and source text, then return. [CSP]
  - 22. If el has an event attribute and a for attribute, and el's type is "classic":
    - 22a. Let for be the value of el's for attribute.
    - 22b. Let event be the value of el's event attribute.
    - 22c. Strip leading and trailing ASCII whitespace from event and for.
    - 22d. If for is not an ASCII case-insensitive match for the string "window", then return.
    - 22e. If event is not an ASCII case-insensitive match for either the string "onload" or the string "onload()", then return.
  - 23. If el has a charset attribute, then let encoding be the result of getting an encoding from the value of the charset attribute. If el does not have a charset attribute, or if getting an encoding failed, then let encoding be el's node document's the encoding.
  - 24. Let classic script CORS setting be the current state of el's crossorigin content attribute.
  - 25. Let module script credentials mode be the CORS settings attribute credentials mode for el's crossorigin content attribute.
  - 26. Let cryptographic nonce be el's [[CryptographicNonce]] internal slot's value.
  - 27. If el has an integrity attribute, then let integrity metadata be that attribute's value. Otherwise, let integrity metadata be the empty string.
  - 28. Let referrer policy be the current state of el's referrerpolicy content attribute.
  - 29. Let fetch priority be the current state of el's fetchpriority content attribute.
  - 30. Let parser metadata be "parser-inserted" if el is parser-inserted, and "not-parser-inserted" otherwise.
  - 31. Let options be a script fetch options whose cryptographic nonce is cryptographic nonce, integrity metadata is integrity metadata, parser metadata is parser metadata, credentials mode is module script credentials mode, referrer policy is referrer policy, and fetch priority is fetch priority.
  - 32. Let settings object be el's node document's relevant settings object.
  - 33. If el has a src content attribute:
    - 33a. If el's type is "importmap" or "speculationrules", then queue an element task on the DOM manipulation task source given el to fire an event named error at el, and return.
    - 33b. Let src be the value of el's src attribute.
    - 33c. If src is the empty string, then queue an element task on the DOM manipulation task source given el to fire an event named error at el, and return.
    - 33d. Set el's from an external file to true.
    - 33e. Let url be the result of encoding-parsing a URL given src, relative to el's node document.
    - 33f. If url is failure, then queue an element task on the DOM manipulation task source given el to fire an event named error at el, and return.
    - 33g. If el is potentially render-blocking, then block rendering on el.
    - 33h. Set el's delaying the load event to true.
    - 33i. If el is currently render-blocking, then set options's render-blocking to true.
    - 33j. Let onComplete given result be the following steps:
      - 33j.i. Mark as ready el given result.
    - 33k. Switch on el's type: "classic" Fetch a classic script given url, settings object, options, classic script CORS setting, encoding, and onComplete. "module" If el does not have an integrity attribute, then set options's integrity metadata to the result of resolving a module integrity metadata with url and settings object. Fetch an external module script graph given url, settings object, options, an…
  - 34. If el does not have a src content attribute:
    - 34a. Let base URL be el's node document's document base URL.
    - 34b. Switch on el's type: "classic" "module" "importmap" "speculationrules"
      - 34b.i. Let script be the result of creating a classic script using source text, settings object, base URL, and options.
      - 34b.ii. Mark as ready el given script.
      - 34b.i. Set el's delaying the load event to true.
      - 34b.ii. If el is potentially render-blocking:
        - 34b.ii.A. Block rendering on el.
        - 34b.ii.B. Set options's render-blocking to true.
      - 34b.iii. Fetch an inline module script graph, given source text, base URL, settings object, options, and with the following steps given result:
        - 34b.iii.A. Queue an element task on the networking task source given el to perform the following steps:
          - 34b.iii.A.1. Mark as ready el given result.
      - 34b.i. Let result be the result of creating an import map parse result given source text and base URL.
      - 34b.ii. Mark as ready el given result.
      - 34b.i. Let result be the result of creating a speculation rules parse result given source text and el's node document.
      - 34b.ii. Mark as ready el given result.
  - 35. If el's type is "classic" and el has a src attribute, or el's type is "module":
    - 35a. Assert: el's result is "uninitialized".
    - 35b. If el has an async attribute or el's force async is true:
      - 35b.i. Let scripts be el's preparation-time document's set of scripts that will execute as soon as possible.
      - 35b.ii. Append el to scripts.
      - 35b.iii. Set el's steps to run when the result is ready to the following:
        - 35b.iii.A. Execute the script element el.
        - 35b.iii.B. Remove el from scripts.
    - 35c. Otherwise, if el is not parser-inserted:
      - 35c.i. Let scripts be el's preparation-time document's list of scripts that will execute in order as soon as possible.
      - 35c.ii. Append el to scripts.
      - 35c.iii. Set el's steps to run when the result is ready to the following:
        - 35c.iii.A. If scripts[0] is not el, then abort these steps.
        - 35c.iii.B. While scripts is not empty, and scripts[0]'s result is not "uninitialized":
          - 35c.iii.B.1. Execute the script element scripts[0].
          - 35c.iii.B.2. Remove scripts[0].
    - 35d. Otherwise, if el has a defer attribute or el's type is "module":
      - 35d.i. Append el to its parser document's list of scripts that will execute when the document has finished parsing.
      - 35d.ii. Set el's steps to run when the result is ready to the following: set el's ready to be parser-executed to true. (The parser will handle executing the script.)
    - 35e. Otherwise:
      - 35e.i. Set el's parser document's pending parsing-blocking script to el.
      - 35e.ii. Block rendering on el.
      - 35e.iii. Set el's steps to run when the result is ready to the following: set el's ready to be parser-executed to true. (The parser will handle executing the script.)
  - 36. Otherwise:
    - 36a. Assert: el's result is not "uninitialized".
    - 36b. If all of the following are true: el's type is "classic";; el is parser-inserted;; el's parser document has a style sheet that is blocking scripts; and; either the parser that created el is an XML parser, or it's an HTML parser whose script nesting level is not greater than one, then:
      - 36b.i. Set el's parser document's pending parsing-blocking script to el.
      - 36b.ii. Set el's ready to be parser-executed to true. (The parser will handle executing the script.)
    - 36c. Otherwise, immediately execute the script element el, even if other scripts are already executing.

- [x] [execute the script element](https://html.spec.whatwg.org/#execute-the-script-element) — To execute the script element given a script element el:
  - 1. Let document be el's node document.
  - 2. If el's preparation-time document is not equal to document, then return.
  - 3. Unblock rendering on el.
  - 4. If el's result is null, then fire an event named error at el, and return.
  - 5. If el's from an external file is true, or el's type is "module", then increment document's ignore-destructive-writes counter.
  - 6. Switch on el's type: "classic" "module" "importmap" "speculationrules"
    - 6a. Let oldCurrentScript be the value to which document's currentScript object was most recently set.
    - 6b. If el's root is not a shadow root, then set document's currentScript attribute to el. Otherwise, set it to null.
    - 6c. Run the classic script given by el's result.
    - 6d. Set document's currentScript attribute to oldCurrentScript.
    - 6a. Assert: document's currentScript attribute is null.
    - 6b. Run the module script given by el's result.
    - 6a. Register an import map given el's relevant global object and el's result.
    - 6a. Register speculation rules given el's relevant global object and el's result.
  - 7. Decrement the ignore-destructive-writes counter of document, if it was incremented in the earlier step.
  - 8. If el's from an external file is true, then fire an event named load at el.

##### [4.12.1.2 Scripting languages](https://html.spec.whatwg.org/#scriptingLanguages)

Status: n/a (author conformance)

##### [4.12.1.3 Restrictions for contents of script elements](https://html.spec.whatwg.org/#restrictions-for-contents-of-script-elements)

Status: n/a (author conformance)

##### [4.12.1.4 Inline documentation for external scripts](https://html.spec.whatwg.org/#inline-documentation-for-external-scripts)

Status: n/a (author conformance)

##### [4.12.1.5 Interaction of script elements and XSLT](https://html.spec.whatwg.org/#scriptTagXSLT)

Status: n/a (no XPath or XSLT)

#### [4.12.2 The noscript element](https://html.spec.whatwg.org/#the-noscript-element)

Status: unreviewed

#### [4.12.3 The template element](https://html.spec.whatwg.org/#the-template-element)

Status: unreviewed
<!-- definitions: attr-template-for attr-template-shadowrootmode attr-shadowrootmode-none-state attr-template-shadowrootdelegatesfocus attr-template-shadowrootserializable attr-template-shadowrootslotassignment attr-template-shadowrootclonable attr-template-shadowrootcustomelementregistry associated-inert-template-document insertion-target insertion-start-marker -->

- [ ] [template contents](https://html.spec.whatwg.org/#template-contents) — Each template element has an associated DocumentFragment object that is its template contents. The template contents have no conformance requirements. When a template element is created, the user agent must run the following steps to establish the template contents:
  - 1. Let document be the template element's node document's appropriate template contents owner document.
  - 2. Create a DocumentFragment object whose node document is document and host is the template element.
  - 3. Set the template element's template contents to the newly created DocumentFragment object.

- [ ] [appropriate template contents owner document](https://html.spec.whatwg.org/#appropriate-template-contents-owner-document) — A Document document's appropriate template contents owner document is the Document returned by the following algorithm:
  - 1. If document is not a Document created by this algorithm:
    - 1a. If document does not yet have an associated inert template document:
      - 1a.i. Let newDocument be the result of creating a document that implements Document, given document's relevant realm; its browsing context is null. This is "a Document created by this algorithm" for the purposes of the step above.
      - 1a.ii. If document is an HTML document, then mark newDocument as an HTML document also.
      - 1a.iii. Set document's associated inert template document to newDocument.
    - 1b. Set document to document's associated inert template document.
  - 2. Return document.

- [ ] [The adopting steps (with node and oldDocument as parameters) for template eleme…](https://html.spec.whatwg.org/#insertion-end-marker) — The adopting steps (with node and oldDocument as parameters) for template elements are the following:
  - 1. Let document be node's node document's appropriate template contents owner document.
  - 2. Adopt node's template contents (a DocumentFragment object) into document.

- [ ] [HTMLTemplateElement.content](https://html.spec.whatwg.org/#dom-template-content) — The content getter steps are:
  - 1. Assert: this's template contents is not a ShadowRoot node.
  - 2. Return this's template contents.

- [ ] [HTMLTemplateElement.shadowRootMode](https://html.spec.whatwg.org/#dom-template-shadowrootmode) — The shadowRootMode IDL attribute must reflect the shadowrootmode content attribute, limited to only known values.

- [ ] [HTMLTemplateElement.shadowRootSlotAssignment](https://html.spec.whatwg.org/#dom-template-shadowrootslotassignment) — The shadowRootSlotAssignment IDL attribute must reflect the shadowrootslotassignment content attribute, limited to only known values.

- [ ] [The cloning steps for template elements given node, copy, and subtree](https://html.spec.whatwg.org/#dom-template-shadowrootslotassignment) — The cloning steps for template elements given node, copy, and subtree are:
  - 1. If subtree is false, then return.
  - 2. For each child of node's template contents's children, in tree order: clone a node given child with document set to copy's template contents's node document, subtree set to true, and parent set to copy's template contents.

- [ ] [prepare content patching](https://html.spec.whatwg.org/#prepare-content-patching) — To prepare content patching given an Element or DocumentFragment scope and a template element template:
  - 1. Assert: template's for attribute is specified.
  - 2. Let markerName be template's for attribute's value.
  - 3. If markerName is the empty string, then return false.
  - 4. Let (start, end) be the result of find markers given scope and markerName.
  - 5. If start is null, then return false.
  - 6. Set template's insertion target to start's parent.
  - 7. Set template's insertion start marker to start.
  - 8. Set template's insertion end marker to end.
  - 9. If start is end, then return true.
  - 10. Let removedNodes be an empty list.
  - 11. Let currentNode be start's next sibling.
  - 12. While currentNode is not null:
    - 12a. If currentNode is end, then break.
    - 12b. Append currentNode to removedNodes.
    - 12c. Set currentNode to currentNode's next sibling.
  - 13. For each node of removedNodes:
    - 13a. If node's parent is not null, then remove node.
  - 14. Return true.

- [ ] [find markers](https://html.spec.whatwg.org/#find-markers) — To find markers given a node scope and a string name:
  - 1. For each descendant of scope's descendants, in tree order:
    - 1a. If descendant is not a ProcessingInstruction node, then continue.
    - 1b. If the result of getting a processing instruction attribute given descendant and "name" is not name, then continue.
    - 1c. If descendant's target is "marker", then return (descendant, descendant).
    - 1d. If descendant's target is "start":
      - 1d.i. Let start be descendant.
      - 1d.ii. Let nestingLevel be 0.
      - 1d.iii. Let sibling be start's next sibling.
      - 1d.iv. While sibling is not null:
        - 1d.iv.A. If sibling is a ProcessingInstruction node:
          - 1d.iv.A.1. If sibling's target is "start", then increment nestingLevel by one.
          - 1d.iv.A.2. If sibling's target is "end":
            - 1d.iv.A.2a. If nestingLevel is 0, then return (start, sibling).
            - 1d.iv.A.2b. Decrement nestingLevel by one.
        - 1d.iv.B. Set sibling to sibling's next sibling.
      - 1d.v. Return (start, null).
  - 2. Return (null, null).

##### [4.12.3.1 Interaction of template elements with XSLT and XPath](https://html.spec.whatwg.org/#template-XSLT-XPath)

Status: n/a (no XPath or XSLT)

#### [4.12.4 The slot element](https://html.spec.whatwg.org/#the-slot-element)

Status: unreviewed
<!-- definitions: attr-slot-name manually-assigned-nodes -->

- [ ] [HTMLSlotElement.assignedNodes(options)](https://html.spec.whatwg.org/#dom-slot-assignednodes) — The assignedNodes(options) method steps are:
  - 1. If options["flatten"] is false, then return this's assigned nodes.
  - 2. Return the result of finding flattened slottables with this.

- [ ] [HTMLSlotElement.assignedElements(options)](https://html.spec.whatwg.org/#dom-slot-assignedelements) — The assignedElements(options) method steps are:
  - 1. If options["flatten"] is false, then return this's assigned nodes, filtered to contain only Element nodes.
  - 2. Return the result of finding flattened slottables with this, filtered to contain only Element nodes.

- [ ] [HTMLSlotElement.assign(...nodes)](https://html.spec.whatwg.org/#dom-slot-assign) — The assign(...nodes) method steps are:
  - 1. For each node of this's manually assigned nodes, set node's manual slot assignment to null.
  - 2. Let nodesSet be a new ordered set.
  - 3. For each node of nodes:
    - 3a. If node's manual slot assignment refers to a slot, then remove node from that slot's manually assigned nodes.
    - 3b. Set node's manual slot assignment to this.
    - 3c. Append node to nodesSet.
  - 4. Set this's manually assigned nodes to nodesSet.
  - 5. Run assign slottables for a tree for this's root.
