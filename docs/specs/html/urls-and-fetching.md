# HTML review checklist: 2.4 URLs

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 27 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html urls-and-fetching` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [2.4 URLs](https://html.spec.whatwg.org/#urls)

Status: unreviewed

#### [2.4.1 Terminology](https://html.spec.whatwg.org/#terminology-2)

Status: unreviewed
<!-- definitions: about:legacy-compat about:html-kind about:srcdoc -->

- [ ] [valid non-empty URL](https://html.spec.whatwg.org/#valid-non-empty-url) — A string is a valid non-empty URL if it is a valid URL string but it is not the empty string.

- [ ] [valid URL potentially surrounded by spaces](https://html.spec.whatwg.org/#valid-url-potentially-surrounded-by-spaces) — A string is a valid URL potentially surrounded by spaces if, after stripping leading and trailing ASCII whitespace from it, it is a valid URL string.

- [ ] [valid non-empty URL potentially surrounded by spaces](https://html.spec.whatwg.org/#valid-non-empty-url-potentially-surrounded-by-spaces) — A string is a valid non-empty URL potentially surrounded by spaces if, after stripping leading and trailing ASCII whitespace from it, it is a valid non-empty URL.

- [ ] [matches about:blank](https://html.spec.whatwg.org/#matches-about:blank) — A URL matches about:blank if its scheme is "about", its path contains a single string "blank", its username and password are the empty string, and its host is null.

- [ ] [matches about:srcdoc](https://html.spec.whatwg.org/#matches-about:srcdoc) — A URL matches about:srcdoc if its scheme is "about", its path contains a single string "srcdoc", its query is null, its username and password are the empty string, and its host is null.

#### [2.4.2 Parsing URLs](https://html.spec.whatwg.org/#resolving-urls)

Status: unreviewed

- [ ] [parse a URL](https://html.spec.whatwg.org/#parse-a-url) — To parse a URL, given a string url, relative to a Document object or environment settings object environment, run these steps. They return failure or a URL.
  - 1. Let baseURL be environment's base URL, if environment is a Document object; otherwise environment's API base URL.
  - 2. Return the result of applying the URL parser to url, with baseURL.

- [ ] [encoding-parse a URL](https://html.spec.whatwg.org/#encoding-parsing-a-url) — To encoding-parse a URL, given a string url, relative to a Document object or environment settings object environment, run these steps. They return failure or a URL.
  - 1. Let encoding be UTF-8.
  - 2. If environment is a Document object, then set encoding to environment's character encoding.
  - 3. Otherwise, if environment's relevant global object is a Window object, set encoding to environment's relevant global object's associated Document's character encoding.
  - 4. Let baseURL be environment's base URL, if environment is a Document object; otherwise environment's API base URL.
  - 5. Return the result of applying the URL parser to url, with baseURL and encoding.

- [ ] [encoding-parse-and-serialize a URL](https://html.spec.whatwg.org/#encoding-parsing-and-serializing-a-url) — To encoding-parse-and-serialize a URL, given a string url, relative to a Document object or environment settings object environment, run these steps. They return failure or a string.
  - 1. Let url be the result of encoding-parsing a URL given url, relative to environment.
  - 2. If url is failure, then return failure.
  - 3. Return the result of applying the URL serializer to url.

#### [2.4.3 Document base URLs](https://html.spec.whatwg.org/#document-base-urls)

Status: unreviewed

- [ ] [document base URL](https://html.spec.whatwg.org/#document-base-url) — The document base URL of a Document document is the URL record obtained by running these steps:
  - 1. If document has a descendant base element that has an href attribute, then return the frozen base URL of the first such base element in tree order.
  - 2. If document's base URL override is non-null, then return document's base URL override.
  - 3. Return document's fallback base URL.

- [ ] [fallback base URL](https://html.spec.whatwg.org/#fallback-base-url) — The fallback base URL of a Document object document is the URL record obtained by running these steps:
  - 1. If document's URL matches about:blank or matches about:srcdoc, and document's about base URL is non-null, then return document's about base URL.
  - 2. Return document's URL.

- [ ] [set the URL](https://html.spec.whatwg.org/#set-the-url) — To set the URL for a Document document to a URL record url:
  - 1. Set document's URL to url.
  - 2. Respond to base URL changes given document.

- [ ] [respond to base URL changes](https://html.spec.whatwg.org/#respond-to-base-url-changes) — To respond to base URL changes for a Document document:
  - 1. The user agent should update any user interface elements which are displaying affected URLs, or data derived from such URLs, to the user. Examples of such user interface elements would be a status bar that displays a hyperlink's url, or some user interface which displays the URL specified by a q, blockquote, ins, or del element's cite attribute.
  - 2. Ensure that the CSS :link/:visited/etc. pseudo-classes are updated appropriately.
  - 3. For each descendant of document's shadow-including descendants:
    - 3a. If descendant is a script element whose result is a speculation rules parse result:
      - 3a.i. Let oldResult be element's result.
      - 3a.ii. Let newResult be the result of creating a speculation rules parse result given element's child text content and element's node document.
      - 3a.iii. Update speculation rules given element's relevant global object, oldResult, and newResult.
  - 4. Consider speculative loads given document.

### [2.5 Fetching resources](https://html.spec.whatwg.org/#fetching-resources)

Status: unreviewed

#### [2.5.1 Terminology](https://html.spec.whatwg.org/#terminology-3)

Status: unreviewed

- [ ] [CORS-same-origin](https://html.spec.whatwg.org/#cors-same-origin) — A response whose type is "basic", "cors", or "default" is CORS-same-origin. [FETCH]

- [ ] [CORS-cross-origin](https://html.spec.whatwg.org/#cors-cross-origin) — A response whose type is "opaque" or "opaqueredirect" is CORS-cross-origin.

- [ ] [unsafe response](https://html.spec.whatwg.org/#unsafe-response) — A response's unsafe response is its internal response if it has one, and the response itself otherwise.

- [ ] [create a potential-CORS request](https://html.spec.whatwg.org/#create-a-potential-cors-request) — To create a potential-CORS request, given a url, destination, corsAttributeState, and an optional same-origin fallback flag, run these steps:
  - 1. Let mode be "no-cors" if corsAttributeState is No CORS, and "cors" otherwise.
  - 2. If same-origin fallback flag is set and mode is "no-cors", set mode to "same-origin".
  - 3. Let credentialsMode be "include".
  - 4. If corsAttributeState is Anonymous, set credentialsMode to "same-origin".
  - 5. Return a new request whose URL is url, destination is destination, mode is mode, credentials mode is credentialsMode, and whose use-URL-credentials flag is set.

#### [2.5.2 Determining the type of a resource](https://html.spec.whatwg.org/#content-type-sniffing)

Status: unreviewed
<!-- definitions: content-type content-type-sniffing-2 content-type-sniffing:-image content-type-sniffing:-text-or-binary content-type-sniffing:-video -->

#### [2.5.3 Extracting character encodings from meta elements](https://html.spec.whatwg.org/#extracting-character-encodings-from-meta-elements)

Status: unreviewed

- [ ] [algorithm for extracting a character encoding from a meta element](https://html.spec.whatwg.org/#algorithm-for-extracting-a-character-encoding-from-a-meta-element) — The algorithm for extracting a character encoding from a meta element, given a string s, is as follows. It returns either a character encoding or nothing.
  - 1. Let position be a pointer into s, initially pointing at the start of the string.
  - 2. Loop: Find the first seven characters in s after position that are an ASCII case-insensitive match for the word "charset". If no such match is found, return nothing.
  - 3. Skip any ASCII whitespace that immediately follow the word "charset" (there might not be any).
  - 4. If the next character is not a U+003D EQUALS SIGN (=), then move position to point just before that next character, and jump back to the step labeled loop.
  - 5. Skip any ASCII whitespace that immediately follow the equals sign (there might not be any).
  - 6. Process the next character as follows: If it is a U+0022 QUOTATION MARK character (") and there is a later U+0022 QUOTATION MARK character (") in sIf it is a U+0027 APOSTROPHE character (') and there is a later U+0027 APOSTROPHE character (') in sReturn the result of getting an encoding from the substring that is between this character and the next earliest occurrence of this character.If it is a…

#### [2.5.4 CORS settings attributes](https://html.spec.whatwg.org/#cors-settings-attributes)

Status: unreviewed
<!-- definitions: cors-settings-attribute -->

- [ ] [No CORS](https://html.spec.whatwg.org/#attr-crossorigin-none) — The attribute's missing value default is the No CORS state, and its invalid value default and empty value default are both the Anonymous state.

- [ ] [CORS settings attribute credentials mode](https://html.spec.whatwg.org/#cors-settings-attribute-credentials-mode) — For more modern features, where the request's mode is always "cors", certain CORS settings attributes have been repurposed to have a slightly different meaning, wherein they only impact the request's credentials mode. To perform this translation, we define the CORS settings attribute credentials mo…
  - • No CORS / Anonymous: "same-origin"
  - • Use Credentials: "include"

#### [2.5.5 Referrer policy attributes](https://html.spec.whatwg.org/#referrer-policy-attributes)

Status: unreviewed
<!-- definitions: referrer-policy-attribute -->

#### [2.5.6 Nonce attributes](https://html.spec.whatwg.org/#nonce-attributes)

Status: n/a (no Content Security Policy)
<!-- definitions: attr-nonce cryptographicnonce -->

- [x] [HTMLOrSVGOrMathMLElement.nonce](https://html.spec.whatwg.org/#dom-noncedelement-nonce) — The nonce IDL attribute must, on getting, return the value of this element's [[CryptographicNonce]]; and on setting, set this element's [[CryptographicNonce]] to the given value.

- [x] [The following attribute change steps are used for the nonce content attribute](https://html.spec.whatwg.org/#dom-noncedelement-nonce) — The following attribute change steps are used for the nonce content attribute:
  - 1. If element does not include HTMLOrSVGOrMathMLElement, then return.
  - 2. If localName is not nonce or namespace is not null, then return.
  - 3. If value is null, then set element's [[CryptographicNonce]] to the empty string.
  - 4. Otherwise, set element's [[CryptographicNonce]] to value.

- [x] [Whenever an element including HTMLOrSVGOrMathMLElement becomes browsing-context…](https://html.spec.whatwg.org/#dom-noncedelement-nonce) — Whenever an element including HTMLOrSVGOrMathMLElement becomes browsing-context connected, the user agent must execute the following steps on the element:
  - 1. Let CSP list be element's shadow-including root's policy container's CSP list.
  - 2. If CSP list contains a header-delivered Content Security Policy, and element has a nonce content attribute whose value is not the empty string:
    - 2a. Let nonce be element's [[CryptographicNonce]].
    - 2b. Set an attribute value for element using "nonce" and the empty string.
    - 2c. Set element's [[CryptographicNonce]] to nonce.

#### [2.5.7 Lazy loading attributes](https://html.spec.whatwg.org/#lazy-loading-attributes)

Status: unreviewed
<!-- definitions: lazy-loading-attribute lazy-load-resumption-steps poster-lazy-load-resumption-steps lazy-load-intersection-observer lazy-load-root-margin -->

- [ ] [will lazy load element steps](https://html.spec.whatwg.org/#will-lazy-load-element-steps) — The will lazy load element steps, given an element element, are as follows:
  - 1. If scripting is disabled for element, then return false.
  - 2. If element's lazy loading attribute is in the Lazy state, then return true.
  - 3. Return false.

- [ ] [start intersection-observing a lazy loading element](https://html.spec.whatwg.org/#start-intersection-observing-a-lazy-loading-element) — To start intersection-observing a lazy loading element element, run these steps:
  - 1. Let doc be element's node document.
  - 2. If doc's lazy load intersection observer is null, set it to a new IntersectionObserver instance, initialized as follows: The callback is these steps, with arguments entries and observer:; The options is an IntersectionObserverInit dictionary with the following dictionary members: «[ "scrollMargin" → lazy load scroll margin ]»
    - 2a. For each entry in entries :
      - 2a.i. Let resumptionSteps be null.
      - 2a.ii. If entry.isIntersecting is true, then set resumptionSteps to entry.target's lazy load resumption steps.
      - 2a.iii. Let posterResumptionSteps be null.
      - 2a.iv. If entry.isIntersecting is true and entry.target is a video element, then set posterResumptionSteps to the video element's poster lazy load resumption steps.
      - 2a.v. If resumptionSteps is null and posterResumptionSteps is null, then continue.
      - 2a.vi. Stop intersection-observing a lazy loading element for entry.target.
      - 2a.vii. If resumptionSteps is not null:
        - 2a.vii.A. Set entry.target's lazy load resumption steps to null.
        - 2a.vii.B. Invoke resumptionSteps.
      - 2a.viii. If posterResumptionSteps is not null:
        - 2a.viii.A. Set the video element's poster lazy load resumption steps to null.
        - 2a.viii.B. Invoke posterResumptionSteps.
  - 3. Call doc's lazy load intersection observer's observe method with element as the argument.

- [ ] [stop intersection-observing a lazy loading element](https://html.spec.whatwg.org/#stop-intersection-observing-a-lazy-loading-element) — To stop intersection-observing a lazy loading element element, run these steps:
  - 1. Let doc be element's node document.
  - 2. Assert: doc's lazy load intersection observer is not null.
  - 3. Call doc's lazy load intersection observer's unobserve method with element as the argument.

#### [2.5.8 Blocking attributes](https://html.spec.whatwg.org/#blocking-attributes)

Status: unreviewed
<!-- definitions: blocking-attribute possible-blocking-token implicitly-potentially-render-blocking -->

- [ ] [blocking tokens set](https://html.spec.whatwg.org/#blocking-tokens-set) — The blocking tokens set for an element el are the result of the following steps:
  - 1. Let value be the value of el's blocking attribute, or the empty string if no such attribute exists.
  - 2. Set value to value, converted to ASCII lowercase.
  - 3. Let rawTokens be the result of splitting value on ASCII whitespace.
  - 4. Return a set containing the elements of rawTokens that are possible blocking tokens.

- [ ] [potentially render-blocking](https://html.spec.whatwg.org/#potentially-render-blocking) — An element is potentially render-blocking if its blocking tokens set contains "render", or if it is implicitly potentially render-blocking, which will be defined at the individual elements. By default, an element is not implicitly potentially render-blocking.

#### [2.5.9 Fetch priority attributes](https://html.spec.whatwg.org/#fetch-priority-attributes)

Status: unreviewed
<!-- definitions: fetch-priority-attribute -->
