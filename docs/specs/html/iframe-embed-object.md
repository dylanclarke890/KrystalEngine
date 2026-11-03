# HTML review checklist: 4.8.5 The iframe element

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 22 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html iframe-embed-object` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

#### [4.8.5 The iframe element](https://html.spec.whatwg.org/#the-iframe-element)

Status: reviewed (2026-11-01)
> `HTMLIFrameElement`: its attributes are reflected, `sandbox` is a token list and its presentational hints are Rendering's. There's no frame tree, so iframes navigate nothing and have no content document or window (see `KNOWN-ISSUES.md`).
> Fixed: `loading` wasn't limited to its known values; its keywords are now shared with `<img>` (`EnumeratedAttributeTraits<"loading", HTMLElement>`).
<!-- definitions: attr-iframe-src an-iframe-srcdoc-document process-iframe-initial-insertion iframe-load-in-progress mute-iframe-load current-navigation-was-lazy-loaded iframe-pending-resource-timing-start-time iframe-pending-resource-timing-url attr-iframe-name attr-iframe-sandbox attr-iframe-allow attr-iframe-allowfullscreen attr-iframe-referrerpolicy -->

- [x] [The srcdoc attribute, if present, must have a value using the HTML syntax that…](https://html.spec.whatwg.org/#attr-iframe-srcdoc) — The srcdoc attribute, if present, must have a value using the HTML syntax that consists of the following syntactic components, in the given order:
  - 1. Any number of comments and ASCII whitespace.
  - 2. Optionally, a DOCTYPE.
  - 3. Any number of comments and ASCII whitespace.
  - 4. The document element, in the form of an html element.
  - 5. Any number of comments and ASCII whitespace.
  > n/a: author conformance.

- [x] [The iframe HTML element post-connection steps, given insertedNode,](https://html.spec.whatwg.org/#the-iframe-element) — The iframe HTML element post-connection steps, given insertedNode, are:
  - 1. If insertedNode's node document's browsing context is null, then return.
  - 2. If insertedNode has a sandbox attribute, then parse the sandboxing directive given the attribute's value and insertedNode's iframe sandboxing flag set.
  - 3. Create a new child navigable for insertedNode.
  - 4. Process the iframe attributes for insertedNode, with initialInsertion set to true.
  > n/a: there's no frame tree.

- [x] [process the iframe attributes](https://html.spec.whatwg.org/#process-the-iframe-attributes) — To process the iframe attributes for an element element, with an optional boolean initialInsertion (default false):
  - 1. If element's srcdoc attribute is specified:
    - 1a. Set element's current navigation was lazy loaded boolean to false.
    - 1b. If the will lazy load element steps given element return true:
      - 1b.i. Set element's lazy load resumption steps to the rest of this algorithm starting with the step labeled navigate to the srcdoc resource.
      - 1b.ii. Set element's current navigation was lazy loaded boolean to true.
      - 1b.iii. Start intersection-observing a lazy loading element for element.
      - 1b.iv. Return.
    - 1c. Navigate to the srcdoc resource: Navigate an iframe or frame given element, about:srcdoc, the empty string, and the value of element's srcdoc attribute. The resulting Document must be considered an iframe srcdoc document.
  - 2. Otherwise:
    - 2a. Let url be the result of running the shared attribute processing steps for iframe and frame elements given element and initialInsertion.
    - 2b. If url is null, then return.
    - 2c. If url matches about:blank and initialInsertion is true:
      - 2c.i. Run the iframe load event steps given element.
      - 2c.ii. Return.
    - 2d. Let referrerPolicy be the current state of element's referrerpolicy content attribute.
    - 2e. Set element's current navigation was lazy loaded boolean to false.
    - 2f. If the will lazy load element steps given element return true:
      - 2f.i. Set element's lazy load resumption steps to the rest of this algorithm starting with the step labeled navigate.
      - 2f.ii. Set element's current navigation was lazy loaded boolean to true.
      - 2f.iii. Start intersection-observing a lazy loading element for element.
      - 2f.iv. Return.
    - 2g. Navigate: Navigate an iframe or frame given element, url, referrerPolicy, null, and initialInsertion.
  > n/a: there's no frame tree.

- [x] [shared attribute processing steps for iframe and frame elements](https://html.spec.whatwg.org/#shared-attribute-processing-steps-for-iframe-and-frame-elements) — The shared attribute processing steps for iframe and frame elements, given an element element and a boolean initialInsertion, are:
  - 1. Let url be the URL record about:blank.
  - 2. If element has a src attribute specified, and its value is not the empty string:
    - 2a. Let maybeURL be the result of encoding-parsing a URL given that attribute's value, relative to element's node document.
    - 2b. If maybeURL is not failure, then set url to maybeURL.
  - 3. If the inclusive ancestor navigables of element's node navigable contains a navigable whose active document's URL equals url with exclude fragments set to true, then return null.
  - 4. If url matches about:blank and initialInsertion is true, then perform the URL and history update steps given element's content navigable's active document and url.
  - 5. Return url.
  > n/a: there's no frame tree.

- [x] [navigate an iframe or frame](https://html.spec.whatwg.org/#navigate-an-iframe-or-frame) — To navigate an iframe or frame given an element element, a URL url, a referrer policy referrerPolicy, an optional string-or-null srcdocString (default null), and an optional boolean initialInsertion (default false):
  - 1. Let historyHandling be "auto".
  - 2. If element's content navigable's active document is not completely loaded, then set historyHandling to "replace".
  - 3. If element is an iframe:
    - 3a. Set element's pending resource-timing start time to the current high resolution time given element's node document's relevant global object.
    - 3b. Set element's pending resource-timing URL to url.
  - 4. Navigate element's content navigable to url using element's node document, with historyHandling set to historyHandling, referrerPolicy set to referrerPolicy, documentResource set to srcdocString, and initialInsertion set to initialInsertion.
  > n/a: there's no frame tree.

- [x] [iframe load event steps](https://html.spec.whatwg.org/#iframe-load-event-steps) — To run the iframe load event steps, given an iframe element element:
  - 1. Assert: element's content navigable is not null.
  - 2. Let childDocument be element's content navigable's active document.
  - 3. If childDocument has its mute iframe load flag set, then return.
  - 4. If element's pending resource-timing start time is not null:
    - 4a. Assert: element's pending resource-timing URL is not null.
    - 4b. Let global be element's node document's relevant global object.
    - 4c. Let fallbackTimingInfo be a new fetch timing info whose start time is element's pending resource-timing start time and whose response end time is the current high resolution time given global.
    - 4d. Mark resource timing given fallbackTimingInfo, the result of parsing element's pending resource-timing URL, "iframe", global, the empty string, a new response body info, and 0.
    - 4e. Set element's pending resource-timing start time to null.
    - 4f. Set element's pending resource-timing URL to null.
  - 5. Set childDocument's iframe load in progress flag.
  - 6. Fire an event named load at element.
  - 7. Unset childDocument's iframe load in progress flag.
  > n/a: there's no frame tree.

- [x] [potentially delays the load event](https://html.spec.whatwg.org/#potentially-delays-the-load-event) — If an element type potentially delays the load event, then for each element element of that type, the user agent must delay the load event of element's node document if element's content navigable is non-null and any of the following are true:
  > n/a: there's no frame tree.

- [x] [allowed to use](https://html.spec.whatwg.org/#allowed-to-use) — To determine whether a Document object document is allowed to use the policy-controlled-feature feature, run these steps:
  - 1. If document's browsing context is null, then return false.
  - 2. If document is not fully active, then return false.
  - 3. If the result of running is feature enabled in document for origin on feature, document, and document's origin is "Enabled", then return true.
  - 4. Return false.
  > n/a: no permissions policy.

- [x] [When the loading attribute's state is changed to the Eager state, the user agen…](https://html.spec.whatwg.org/#attr-iframe-loading) — When the loading attribute's state is changed to the Eager state, the user agent must run these steps:
  - 1. Let resumptionSteps be the iframe element's lazy load resumption steps.
  - 2. If resumptionSteps is null, then return.
  - 3. Set the iframe's lazy load resumption steps to null.
  - 4. Invoke resumptionSteps.
  > n/a: there's no frame tree.

- [x] [HTMLIFrameElement.srcdoc](https://html.spec.whatwg.org/#dom-iframe-srcdoc) — The srcdoc getter steps are:
  - 1. Let attribute be the result of getting an attribute by namespace and local name given null, srcdoc's local name, and this.
  - 2. If attribute is null, then return the empty string.
  - 3. Return attribute's value.
  > `HTMLIFrameElement::SrcDoc`; there are no Trusted Types.

- [x] [The srcdoc setter steps](https://html.spec.whatwg.org/#dom-iframe-srcdoc) — The srcdoc setter steps are:
  - 1. Let compliantString be the result of invoking the get trusted type compliant string algorithm with TrustedHTML, this's relevant global object, the given value, "HTMLIFrameElement srcdoc", and "script".
  - 2. Set an attribute value given this, srcdoc's local name, and compliantString.

- [x] [HTMLIFrameElement.referrerPolicy](https://html.spec.whatwg.org/#dom-iframe-referrerpolicy) — The referrerPolicy IDL attribute must reflect the referrerpolicy content attribute, limited to only known values.

- [x] [HTMLIFrameElement.loading](https://html.spec.whatwg.org/#dom-iframe-loading) — The loading IDL attribute must reflect the loading content attribute, limited to only known values.

- [x] [HTMLIFrameElement.contentDocument](https://html.spec.whatwg.org/#dom-iframe-contentdocument) — The contentDocument getter steps are to return this's content document.
  > n/a: there's no frame tree.

- [x] [HTMLIFrameElement.contentWindow](https://html.spec.whatwg.org/#dom-iframe-contentwindow) — The contentWindow getter steps are to return this's content window.
  > n/a: there's no frame tree.

#### [4.8.6 The embed element](https://html.spec.whatwg.org/#the-embed-element)

Status: reviewed (2026-11-01)
> `HTMLEmbedElement`: its attributes are reflected and its presentational hints are `HTMLPlugInElement`'s. There are no plugins, and it doesn't display its resource (see `KNOWN-ISSUES.md`).
<!-- definitions: attr-embed-src attr-embed-type -->

- [x] [potentially active](https://html.spec.whatwg.org/#concept-embed-active) — An embed element is said to be potentially active when the following conditions are all met simultaneously:
  > n/a: embed doesn't load its resource.

- [x] [embed task source](https://html.spec.whatwg.org/#embed-task-source) — Whenever an embed element that was not potentially active becomes potentially active, and whenever a potentially active embed element that is remaining potentially active has its src attribute set, changed, or removed or its type attribute set, changed, or removed, the user agent must queue an element task on the embed task source given the element to run the embed element setup steps for that element.
  > n/a: embed doesn't load its resource.

- [x] [The embed element setup steps](https://html.spec.whatwg.org/#the-embed-element-setup-steps) — The embed element setup steps for a given embed element element are as follows:
  - 1. If another task has since been queued to run the embed element setup steps for element, then return.
  - 2. If element has a src attribute set:
    - 2a. Let url be the result of encoding-parsing a URL given element's src attribute's value, relative to element's node document.
    - 2b. If url is failure, then return.
    - 2c. Let request be a new request whose URL is url, client is element's node document's relevant settings object, destination is "embed", credentials mode is "include", mode is "navigate", initiator type is "embed", and whose use-URL-credentials flag is set.
    - 2d. Fetch request, with processResponse set to the following steps given response response: Fetching the resource must delay the load event of element's node document.
      - 2d.i. If another task has since been queued to run the embed element setup steps for element, then return.
      - 2d.ii. If response is a network error, then fire an event named load at element, and return.
      - 2d.iii. Let type be the result of determining the type of content given element and response.
      - 2d.iv. Switch on type: null Otherwise
        - 2d.iv.A. Display no plugin for element.
        - 2d.iv.A. If element's content navigable is null, then create a new child navigable for element.
        - 2d.iv.B. Navigate element's content navigable to response's URL using element's node document, with response set to response, and historyHandling set to "replace".
        - 2d.iv.C. element now represents its content navigable.
  - 3. Otherwise, display no plugin for element.
  > n/a: embed doesn't load its resource.

- [x] [type of the content](https://html.spec.whatwg.org/#concept-embed-type) — To determine the type of the content given an embed element element and a response response, run the following steps:
  - 1. If element has a type attribute, and that attribute's value is a type that a plugin supports, then return the value of the type attribute.
  - 2. If the path component of response's URL matches a pattern that a plugin supports, then return the type that that plugin can handle.
  - 3. If response has explicit Content-Type metadata, and that value is a type that a plugin supports, then return that value.
  - 4. Return null.
  > n/a: embed doesn't load its resource.

- [x] [display no plugin](https://html.spec.whatwg.org/#display-no-plugin) — To display no plugin for an embed element element:
  - 1. Destroy a child navigable given element.
  - 2. Display an indication that no plugin could be found for element, as the contents of element.
  - 3. element now represents nothing.
  > n/a: there are no plugins; nothing is displayed.

#### [4.8.7 The object element](https://html.spec.whatwg.org/#the-object-element)

Status: reviewed (2026-11-01)
> `HTMLObjectElement`: a form-associated element whose attributes are reflected. It doesn't display its resource, so it always shows its fallback content (see `KNOWN-ISSUES.md`).
<!-- definitions: attr-object-data attr-object-type attr-object-name -->

- [x] [HTMLObjectElement.contentDocument](https://html.spec.whatwg.org/#dom-object-contentdocument) — The contentDocument getter steps are to return this's content document.
  > n/a: there's no frame tree.

- [x] [HTMLObjectElement.contentWindow](https://html.spec.whatwg.org/#dom-object-contentwindow) — The contentWindow getter steps are to return this's content window.
  > n/a: there's no frame tree.
