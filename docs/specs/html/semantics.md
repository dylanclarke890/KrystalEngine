# HTML review checklist: 4 The elements of HTML

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 32 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html semantics` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

## [4 The elements of HTML](https://html.spec.whatwg.org/#semantics)

Status: unreviewed

### [4.1 The document element](https://html.spec.whatwg.org/#the-root-element)

Status: reviewed (2026-11-01)
> Headings only.

#### [4.1.1 The html element](https://html.spec.whatwg.org/#the-html-element)

Status: reviewed (2026-11-01)
> `HTMLHtmlElement`; the rest is author conformance.

### [4.2 Document metadata](https://html.spec.whatwg.org/#document-metadata)

Status: reviewed (2026-11-01)
> Headings only.

#### [4.2.1 The head element](https://html.spec.whatwg.org/#the-head-element)

Status: reviewed (2026-11-01)
> `HTMLHeadElement`; the rest is author conformance.

#### [4.2.2 The title element](https://html.spec.whatwg.org/#the-title-element)

Status: reviewed (2026-11-01)
> `HTMLTitleElement::Text`. The document's title is `Document::Title` (3.1.7).

- [x] [HTMLTitleElement.text](https://html.spec.whatwg.org/#dom-title-text) — The text attribute's getter must return this title element's child text content.

#### [4.2.3 The base element](https://html.spec.whatwg.org/#the-base-element)

Status: reviewed (2026-11-01)
> `Document::ProcessBaseElement` (the frozen base URL of the first base element with an href, and the first base target), `HTMLBaseElement::Href`; getting an element's target is `HyperlinkAlgorithms::EffectiveTarget` and `HTMLFormElement::EffectiveTarget`.
> Fixed: an empty `target` fell back to the base target, and a form's base target wasn't checked for dangling markup (DIFFERENCES.md, from upstream).
<!-- definitions: attr-base-href attr-base-target frozen-base-url -->

- [x] [get an element's target](https://html.spec.whatwg.org/#get-an-element's-target) — To get an element's target, given an a, area, or form element element, and an optional string-or-null target (default null), run these steps:
  - 1. If target is null:
    - 1a. If element has a target attribute, then set target to that attribute's value.
    - 1b. Otherwise, if element's node document contains a base element with a target attribute, set target to the value of the target attribute of the first such base element.
  - 2. If target is not null, and contains an ASCII tab or newline and a U+003C (<), then set target to "_blank".
  - 3. Return target.

- [x] [set the frozen base URL](https://html.spec.whatwg.org/#set-the-frozen-base-url) — To set the frozen base URL for an element element:
  - 1. Let document be element's node document.
  - 2. Let urlRecord be the result of parsing the value of element's href content attribute with document's fallback base URL, and document's character encoding. (Thus, the base element isn't affected by itself.)
  - 3. If any of the following are true: urlRecord is failure;; urlRecord's scheme is "data" or "javascript"; or; running Is base allowed for Document? on urlRecord and document returns "Blocked", then set element's frozen base URL to document's fallback base URL and return.
  - 4. Set element's frozen base URL to urlRecord.
  - 5. Respond to base URL changes given document.

- [x] [HTMLBaseElement.href](https://html.spec.whatwg.org/#dom-base-href) — The href IDL attribute, on getting, must return the result of running the following algorithm:
  - 1. Let document be element's node document.
  - 2. Let url be the value of the href attribute of this element, if it has one, and the empty string otherwise.
  - 3. Let urlRecord be the result of parsing url with document's fallback base URL, and document's character encoding. (Thus, the base element isn't affected by other base elements or itself.)
  - 4. If urlRecord is failure, return url.
  - 5. Return the serialization of urlRecord.

#### [4.2.4 The link element](https://html.spec.whatwg.org/#the-link-element)

Status: unreviewed
<!-- definitions: attr-link-href attr-link-rel attr-link-crossorigin attr-link-media attr-link-integrity attr-link-hreflang attr-link-type attr-link-referrerpolicy attr-link-title attr-link-imagesrcset attr-link-imagesizes attr-link-sizes attr-link-as attr-link-blocking attr-link-color explicitly-enabled attr-link-disabled attr-link-fetchpriority -->

- [ ] [allowed in the body](https://html.spec.whatwg.org/#allowed-in-the-body) — If a link element has an itemprop attribute, or has a rel attribute that contains only keywords that are body-ok, then the element is said to be allowed in the body. This means that the element can be used where phrasing content is expected.

- [ ] [HTMLLinkElement.as](https://html.spec.whatwg.org/#dom-link-as) — The as IDL attribute must reflect the as content attribute, limited to only known values.

- [ ] [HTMLLinkElement.crossOrigin](https://html.spec.whatwg.org/#dom-link-crossorigin) — The crossOrigin IDL attribute must reflect the crossorigin content attribute, limited to only known values.

- [ ] [HTMLLinkElement.referrerPolicy](https://html.spec.whatwg.org/#dom-link-referrerpolicy) — The referrerPolicy IDL attribute must reflect the referrerpolicy content attribute, limited to only known values.

- [ ] [HTMLLinkElement.fetchPriority](https://html.spec.whatwg.org/#dom-link-fetchpriority) — The fetchPriority IDL attribute must reflect the fetchpriority content attribute, limited to only known values.

##### [4.2.4.1 Processing the media attribute](https://html.spec.whatwg.org/#processing-the-media-attribute)

Status: unreviewed

##### [4.2.4.2 Processing the type attribute](https://html.spec.whatwg.org/#processing-the-type-attribute)

Status: unreviewed

##### [4.2.4.3 Fetching and processing a resource from a link element](https://html.spec.whatwg.org/#fetching-and-processing-a-resource-from-a-link-element)

Status: unreviewed
<!-- definitions: fetch-and-process-the-linked-resource linked-resource-fetch-setup-steps process-the-linked-resource -->

- [ ] [default fetch and process the linked resource](https://html.spec.whatwg.org/#default-fetch-and-process-the-linked-resource) — The default fetch and process the linked resource, given a link element el, is as follows:
  - 1. Let options be the result of creating link options from el.
  - 2. Let request be the result of creating a link request given options.
  - 3. If request is null, then return.
  - 4. Run the linked resource fetch setup steps, given el and request. If the result is false, then return.
  - 5. Set request's initiator type to "css" if el's rel attribute contains the keyword stylesheet; "link" otherwise.
  - 6. Fetch request with processResponseConsumeBody set to the following steps given response response and null, failure, or a byte sequence bodyBytes:
    - 6a. Let success be true.
    - 6b. If any of the following are true: bodyBytes is null or failure; or; response's status is not an ok status, then set success to false.
    - 6c. Otherwise, wait for the link resource's critical subresources to finish loading.
    - 6d. Process the linked resource given el, success, response, and bodyBytes.

- [ ] [create a link request](https://html.spec.whatwg.org/#create-a-link-request) — To create a link request given a link processing options options:
  - 1. Assert: options's href is not the empty string.
  - 2. Let url be the result of encoding-parsing a URL given options's href, relative to options's base URL.
  - 3. If url is failure, then return null.
  - 4. Let request be the result of creating a potential-CORS request given url, options's destination, and options's crossorigin.
  - 5. Set request's policy container to options's policy container.
  - 6. Set request's integrity metadata to options's integrity.
  - 7. Set request's cryptographic nonce metadata to options's cryptographic nonce metadata.
  - 8. Set request's referrer policy to options's referrer policy.
  - 9. Set request's client to options's environment.
  - 10. Set request's priority to options's fetch priority.
  - 11. Return request.

##### [4.2.4.4 Processing `Link` headers](https://html.spec.whatwg.org/#processing-link-headers)

Status: unreviewed
<!-- definitions: process-a-link-header link-processing-options link-options-href link-options-initiator link-options-integrity link-options-type link-options-nonce link-options-destination link-options-crossorigin link-options-referrer-policy link-options-source-set link-options-base-url link-options-origin link-options-environment link-options-policy-container link-options-document link-options-on-document-ready link-options-fetch-priority -->

- [ ] [create link options from element](https://html.spec.whatwg.org/#create-link-options-from-element) — To create link options from element given a link element el:
  - 1. Let document be el's node document.
  - 2. Let options be a new link processing options with crossoriginthe state of el's crossorigin content attributereferrer policythe state of el's referrerpolicy content attributesource setel's source setbase URLdocument's document base URLorigindocument's originenvironmentdocument's relevant settings objectpolicy containerdocument's policy containerdocumentdocumentcryptographic nonce metadatathe curre…
  - 3. If el has an href attribute, then set options's href to the value of el's href attribute.
  - 4. If el has an integrity attribute, then set options's integrity to the value of el's integrity content attribute.
  - 5. If el has a type attribute, then set options's type to the value of el's type attribute.
  - 6. Assert: options's href is not the empty string, or options's source set is not null. A link element with neither an href or an imagesrcset does not represent a link.
  - 7. Return options.

- [ ] [extract links from headers](https://html.spec.whatwg.org/#extract-links-from-headers) — To extract links from headers given a header list headers:
  - 1. Let links be a new list.
  - 2. Let rawLinkHeaders be the result of getting, decoding, and splitting `Link` from headers.
  - 3. For each linkHeader of rawLinkHeaders:
    - 3a. Let linkObject be the result of parsing linkHeader. [WEBLINK]
    - 3b. If linkObject["target_uri"] does not exist, then continue.
    - 3c. Append linkObject to links.
  - 4. Return links.

- [ ] [process link headers](https://html.spec.whatwg.org/#process-link-headers) — To process link headers given a Document doc, a response response, and a "pre-media" or "media" phase:
  - 1. Let links be the result of extracting links from response's header list.
  - 2. For each linkObject in links:
    - 2a. Let rel be linkObject["relation_type"].
    - 2b. Let attribs be linkObject["target_attributes"].
    - 2c. Let expectedPhase be "media" if either "srcset", "imagesrcset", or "media" exist in attribs; otherwise "pre-media".
    - 2d. If expectedPhase is not phase, then continue.
    - 2e. If attribs["media"] exists and attribs["media"] does not match the environment, then continue.
    - 2f. Let options be a new link processing options with hreflinkObject["target_uri"]base URLdoc's document base URLorigindoc's originenvironmentdoc's relevant settings objectpolicy containerdoc's policy containerdocumentdoc
    - 2g. Apply link options from parsed header attributes to options given attribs and rel. If that returned false, then return.
    - 2h. If attribs["imagesrcset"] exists and attribs["imagesizes"] exists, then set options's source set to the result of creating a source set given linkObject["target_uri"], attribs["imagesrcset"], attribs["imagesizes"], and null.
    - 2i. Run the process a link header steps for rel given options.

- [ ] [apply link options from parsed header attributes](https://html.spec.whatwg.org/#apply-link-options-from-parsed-header-attributes) — To apply link options from parsed header attributes to a link processing options options given attribs and a string rel:
  - 1. If rel is "preload":
    - 1a. If attribs["as"] does not exist, then return false.
    - 1b. Let destination be the result of translating attribs["as"].
    - 1c. If destination is null, then return false.
    - 1d. Set options's destination to destination.
  - 2. If attribs["crossorigin"] exists and is an ASCII case-insensitive match for one of the CORS settings attribute keywords, then set options's crossorigin to the CORS settings attribute state corresponding to that keyword.
  - 3. If attribs["integrity"] exists, then set options's integrity to attribs["integrity"].
  - 4. If attribs["referrerpolicy"] exists and is an ASCII case-insensitive match for some referrer policy, then set options's referrer policy to that referrer policy.
  - 5. If attribs["nonce"] exists, then set options's nonce to attribs["nonce"].
  - 6. If attribs["type"] exists, then set options's type to attribs["type"].
  - 7. If attribs["fetchpriority"] exists and is an ASCII case-insensitive match for a fetch priority attribute keyword, then set options's fetch priority to that fetch priority attribute keyword.
  - 8. Return true.

##### [4.2.4.5 Early hints](https://html.spec.whatwg.org/#early-hints)

Status: unreviewed
<!-- definitions: early-hints-2 -->

- [ ] [process early hint headers](https://html.spec.whatwg.org/#process-early-hint-headers) — To process early hint headers given a response response and an environment reservedEnvironment:
  - 1. Let earlyPolicyContainer be the result of creating a policy container from a fetch response given response and reservedEnvironment.
  - 2. Let links be the result of extracting links from response's header list.
  - 3. Let earlyHints be an empty list.
  - 4. For each linkObject in links:
    - 4a. Let rel be linkObject["relation_type"].
    - 4b. Let options be a new link processing options with hreflinkObject["target_uri"]initiator"early-hint"base URLresponse's URLoriginresponse's URL's originenvironmentreservedEnvironmentpolicy containerearlyPolicyContainer
    - 4c. Let attribs be linkObject["target_attributes"].
    - 4d. Apply link options from parsed header attributes to options given attribs and rel. If that returned false, then return.
    - 4e. Run the process a link header steps for rel given options.
    - 4f. Append options to earlyHints.
  - 5. Return the following substeps given Document doc: for each options in earlyHints:
    - 5a. If options's on document ready is null, then set options's document to doc.
    - 5b. Otherwise, call options's on document ready with doc.

##### [4.2.4.6 Providing users with a means to follow hyperlinks created using the link element](https://html.spec.whatwg.org/#providing-users-with-a-means-to-follow-hyperlinks-created-using-the-link-element)

Status: unreviewed

#### [4.2.5 The meta element](https://html.spec.whatwg.org/#the-meta-element)

Status: unreviewed
<!-- definitions: attr-meta-charset attr-meta-content attr-meta-name attr-meta-media -->

##### [4.2.5.1 Standard metadata names](https://html.spec.whatwg.org/#standard-metadata-names)

Status: unreviewed
<!-- definitions: meta-author meta-description meta-generator -->

- [ ] [To find the application name to use given an ordered list of languages (e.g. Br…](https://html.spec.whatwg.org/#meta-application-name) — To find the application name to use given an ordered list of languages (e.g. British English, American English, and English), user agents must run the following steps:
  - 1. Let languages be the list of languages.
  - 2. Let default language be the language of the Document's document element, if any, and if that language is not unknown.
  - 3. If there is a default language, and if it is not the same language as any of the languages in languages, append it to languages.
  - 4. Let winning language be the first language in languages for which there is a meta element in the Document where the name attribute value is an ASCII case-insensitive match for application-name and whose language is the language in question. If none of the languages have such a meta element, then return; there's no given application name.
  - 5. Return the value of the content attribute of the first meta element in the Document in tree order where the name attribute value is an ASCII case-insensitive match for application-name and whose language is winning language.

- [ ] [To obtain the list of keywords that the author has specified as applicable to t…](https://html.spec.whatwg.org/#meta-keywords) — To obtain the list of keywords that the author has specified as applicable to the page, the user agent must run the following steps:
  - 1. Let keywords be an empty list.
  - 2. For each meta element with a name attribute and a content attribute and where the name attribute value is an ASCII case-insensitive match for keywords:
    - 2a. Split the value of the element's content attribute on commas.
    - 2b. Add the resulting tokens, if any, to keywords.
  - 3. Remove any duplicates from keywords.
  - 4. Return keywords. This is the list of keywords that the author has specified as applicable to the page.

- [ ] [If any meta element element is inserted into the document, or has its name or c…](https://html.spec.whatwg.org/#meta-referrer) — If any meta element element is inserted into the document, or has its name or content attributes changed, user agents must run the following algorithm:
  - 1. If element is not in a document tree, then return.
  - 2. If element does not have a name attribute whose value is an ASCII case-insensitive match for "referrer", then return.
  - 3. If element does not have a content attribute, or that attribute's value is the empty string, then return.
  - 4. Let value be the value of element's content attribute, converted to ASCII lowercase.
  - 5. If value is one of the values given in the first column of the following table, then set value to the value given in the second column:
  - 6. If value is a referrer policy, then set element's node document's policy container's referrer policy to policy.

- [ ] [To obtain a page's theme color, user agents must run the following steps](https://html.spec.whatwg.org/#meta-theme-color) — To obtain a page's theme color, user agents must run the following steps:
  - 1. Let candidate elements be the list of all meta elements that meet the following criteria, in tree order: the element is in a document tree;; the element has a name attribute, whose value is an ASCII case-insensitive match for theme-color; and; the element has a content attribute.
  - 2. For each element in candidate elements:
    - 2a. If element has a media attribute and the value of element's media attribute does not match the environment, then continue.
    - 2b. Let value be the result of stripping leading and trailing ASCII whitespace from the value of element's content attribute.
    - 2c. Let color be the result of parsing value.
    - 2d. If color is not failure, then return color.
  - 3. Return nothing (the page has no theme color).

- [ ] [To obtain a page's supported color-schemes, user agents must run the following…](https://html.spec.whatwg.org/#meta-color-scheme) — To obtain a page's supported color-schemes, user agents must run the following steps:
  - 1. Let candidate elements be the list of all meta elements that meet the following criteria, in tree order: the element is in a document tree;; the element has a name attribute, whose value is an ASCII case-insensitive match for color-scheme; and; the element has a content attribute.
  - 2. For each element in candidate elements:
    - 2a. Let parsed be the result of parsing a list of component values given the value of element's content attribute.
    - 2b. If parsed is a valid CSS 'color-scheme' property value, then return parsed.
  - 3. Return null.

##### [4.2.5.2 Other metadata names](https://html.spec.whatwg.org/#other-metadata-names)

Status: unreviewed
<!-- definitions: concept-meta-extensions -->

##### [4.2.5.3 Pragma directives](https://html.spec.whatwg.org/#pragma-directives)

Status: unreviewed
<!-- definitions: attr-meta-http-equiv attr-meta-http-equiv-content-language attr-meta-http-equiv-content-type attr-meta-http-equiv-default-style attr-meta-http-equiv-refresh attr-meta-http-equiv-set-cookie attr-meta-http-equiv-x-ua-compatible attr-meta-http-equiv-content-security-policy -->

- [ ] [pragma-set default language](https://html.spec.whatwg.org/#pragma-set-default-language) — This pragma sets the pragma-set default language. Until such a pragma is successfully processed, there is no pragma-set default language.

- [ ] [pragma-set default language](https://html.spec.whatwg.org/#pragma-set-default-language) — This pragma sets the pragma-set default language. Until such a pragma is successfully processed, there is no pragma-set default language.
  - 1. If the meta element has no content attribute, then return.
  - 2. If the element's content attribute contains a U+002C COMMA character (,), then return.
  - 3. Let input be the value of the element's content attribute.
  - 4. Let position point at the first character of input.
  - 5. Skip ASCII whitespace within input given position.
  - 6. Collect a sequence of code points that are not ASCII whitespace from input given position.
  - 7. Let candidate be the string that resulted from the previous step.
  - 8. If candidate is the empty string, return.
  - 9. Set the pragma-set default language to candidate.

- [ ] [will declaratively refresh (Document)](https://html.spec.whatwg.org/#will-declaratively-refresh) — A Document object has an associated will declaratively refresh (a boolean). It is initially false.
  - 1. If the meta element has no content attribute, or if that attribute's value is the empty string, then return.
  - 2. Let input be the value of the element's content attribute.
  - 3. Run the shared declarative refresh steps with the meta element's node document, input, and the meta element.

- [ ] [shared declarative refresh steps](https://html.spec.whatwg.org/#shared-declarative-refresh-steps) — The shared declarative refresh steps, given a Document object document, string input, and optionally a meta element meta, are as follows:
  - 1. If document's will declaratively refresh is true, then return.
  - 2. Let position point at the first code point of input.
  - 3. Skip ASCII whitespace within input given position.
  - 4. Let time be 0.
  - 5. Collect a sequence of code points that are ASCII digits from input given position, and let timeString be the result.
  - 6. If timeString is the empty string:
    - 6a. If the code point in input pointed to by position is not U+002E (.), then return.
  - 7. Otherwise, set time to the result of parsing timeString using the rules for parsing non-negative integers.
  - 8. Collect a sequence of code points that are ASCII digits and U+002E FULL STOP characters (.) from input given position. Ignore any collected characters.
  - 9. Let urlRecord be document's URL.
  - 10. If position is not past the end of input:
    - 10a. If the code point in input pointed to by position is not U+003B (;), U+002C (,), or ASCII whitespace, then return.
    - 10b. Skip ASCII whitespace within input given position.
    - 10c. If the code point in input pointed to by position is U+003B (;) or U+002C (,), then advance position to the next code point.
    - 10d. Skip ASCII whitespace within input given position.
  - 11. If position is not past the end of input:
    - 11a. Let urlString be the substring of input from the code point at position to the end of the string.
    - 11b. If the code point in input pointed to by position is U+0055 (U) or U+0075 (u), then advance position to the next code point. Otherwise, jump to the step labeled skip quotes.
    - 11c. If the code point in input pointed to by position is U+0052 (R) or U+0072 (r), then advance position to the next code point. Otherwise, jump to the step labeled parse.
    - 11d. If the code point in input pointed to by position is U+004C (L) or U+006C (l), then advance position to the next code point. Otherwise, jump to the step labeled parse.
    - 11e. Skip ASCII whitespace within input given position.
    - 11f. If the code point in input pointed to by position is U+003D (=), then advance position to the next code point. Otherwise, jump to the step labeled parse.
    - 11g. Skip ASCII whitespace within input given position.
    - 11h. Skip quotes: If the code point in input pointed to by position is U+0027 (') or U+0022 ("), then let quote be that code point, and advance position to the next code point. Otherwise, let quote be the empty string.
    - 11i. Set urlString to the substring of input from the code point at position to the end of the string.
    - 11j. If quote is not the empty string, and there is a code point in urlString equal to quote, then truncate urlString at that code point, so that it and all subsequent code points are removed.
    - 11k. Parse: Set urlRecord to the result of encoding-parsing a URL given urlString, relative to document.
    - 11l. If urlRecord is failure, then return.
    - 11m. If urlRecord's scheme is "javascript", then return.
  - 12. Set document's will declaratively refresh to true.
  - 13. Perform one or more of the following steps: After the refresh has come due (as defined below), if the user has not canceled the redirect and, if meta is given, document's active sandboxing flag set does not have the sandboxed automatic features browsing context flag set, then navigate document's node navigable to urlRecord using document, with historyHandling set to "replace". For the purposes of…

##### [4.2.5.4 Specifying the document's character encoding](https://html.spec.whatwg.org/#charset)

Status: unreviewed
<!-- definitions: character-encoding-declaration -->

#### [4.2.6 The style element](https://html.spec.whatwg.org/#the-style-element)

Status: unreviewed
<!-- definitions: attr-style-media attr-style-blocking attr-style-title -->

- [ ] [HTMLStyleElement.disabled](https://html.spec.whatwg.org/#dom-style-disabled) — The disabled getter steps are:
  - 1. If this does not have an associated CSS style sheet, return false.
  - 2. If this's associated CSS style sheet's disabled flag is set, return true.
  - 3. Return false.

- [ ] [The disabled setter steps](https://html.spec.whatwg.org/#dom-style-disabled) — The disabled setter steps are:
  - 1. If this does not have an associated CSS style sheet, return.
  - 2. If the given value is true, set this's associated CSS style sheet's disabled flag. Otherwise, unset this's associated CSS style sheet's disabled flag.

- [ ] [update a style block](https://html.spec.whatwg.org/#update-a-style-block) — The update a style block algorithm is as follows:
  - 1. Let element be the style element.
  - 2. If element has an associated CSS style sheet, remove the CSS style sheet in question.
  - 3. If element is not connected, then return.
  - 4. If element's type attribute is present and its value is neither the empty string nor an ASCII case-insensitive match for "text/css", then return.
  - 5. If the Should element's inline behavior be blocked by Content Security Policy? algorithm returns "Blocked" when executed upon element, "style", and element's child text content, then return. [CSP]
  - 6. Create a CSS style sheet with the following properties: typetext/cssowner nodeelementmedia The media attribute of element. title The title attribute of element, if element is in a document tree, or the empty string otherwise. alternate flagUnset.origin-clean flagSet.locationparent CSS style sheetowner CSS rulenulldisabled flagLeft at its default value.CSS rules Left uninitialized.
  - 7. If element contributes a script-blocking style sheet, append element to its node document's script-blocking style sheet set.
  - 8. If element's media attribute's value matches the environment and element is potentially render-blocking, then block rendering on element.

- [ ] [Once the attempts to obtain the style sheet's critical subresources, if any, ar…](https://html.spec.whatwg.org/#update-a-style-block) — Once the attempts to obtain the style sheet's critical subresources, if any, are complete, or, if the style sheet has no critical subresources, once the style sheet has been parsed and processed, the user agent must run these steps:
  - 1. Let element be the style element associated with the style sheet in question.
  - 2. Let success be true.
  - 3. If the attempts to obtain any of the style sheet's critical subresources failed for any reason (e.g., DNS error, HTTP 404 response, a connection being prematurely closed, unsupported Content-Type), set success to false.
  - 4. Queue an element task on the networking task source given element and the following steps:
    - 4a. If success is true, fire an event named load at element.
    - 4b. Otherwise, fire an event named error at element.
    - 4c. If element contributes a script-blocking style sheet:
      - 4c.i. Assert: element's node document's script-blocking style sheet set contains element.
      - 4c.ii. Remove element from its node document's script-blocking style sheet set.
    - 4d. Unblock rendering on element.

#### [4.2.7 Interactions of styling and scripting](https://html.spec.whatwg.org/#interactions-of-styling-and-scripting)

Status: unreviewed
<!-- definitions: script-blocking-style-sheet-set -->

- [ ] [contributes a script-blocking style sheet](https://html.spec.whatwg.org/#contributes-a-script-blocking-style-sheet) — An element el in the context of a Document of an HTML parser or XML parser contributes a script-blocking style sheet if all of the following are true:

- [ ] [has a style sheet that is blocking scripts](https://html.spec.whatwg.org/#has-a-style-sheet-that-is-blocking-scripts) — A Document document has a style sheet that is blocking scripts if the following steps return true:
  - 1. If document's script-blocking style sheet set is not empty, then return true.
  - 2. If document's node navigable is null, then return false.
  - 3. Let containerDocument be document's node navigable's container document.
  - 4. If containerDocument is non-null and containerDocument's script-blocking style sheet set is not empty, then return true.
  - 5. Return false.

- [ ] [has no style sheet that is blocking scripts](https://html.spec.whatwg.org/#has-no-style-sheet-that-is-blocking-scripts) — A Document has no style sheet that is blocking scripts if it does not have a style sheet that is blocking scripts.
