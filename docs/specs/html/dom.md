# HTML review checklist: 3 Semantics, structure, and APIs of HTML documents

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 65 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html dom` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

## [3 Semantics, structure, and APIs of HTML documents](https://html.spec.whatwg.org/#dom)

Status: unreviewed

### [3.1 Documents](https://html.spec.whatwg.org/#documents)

Status: reviewed (2026-11-01)
> A document's address is its URL (`Document::GetURL`, DOM). There are no origins (not planned), and no referrer until remote pages.
<!-- definitions: the-document's-address concept-document-origin the-document's-referrer -->

#### [3.1.1 The Document object](https://html.spec.whatwg.org/#the-document-object)

Status: reviewed (2026-11-01)
> The open dialogs list is `Document::OpenDialogs`; being the initial about:blank is `FrameLoader::IsDisplayingInitialEmptyDocument`. There's no policy container, permissions policy, module map, cross-origin opener policy, navigation ID or bfcache blocking details (no origins, policies or scripting), and no about base URL (DIFFERENCES.md).
<!-- definitions: concept-document-policy-container concept-document-permissions-policy concept-document-module-map concept-document-coop is-initial-about:blank concept-document-navigation-id concept-document-about-base-url concept-document-bfcache-blocking-details open-dialogs-list -->

#### [3.1.2 The DocumentOrShadowRoot interface](https://html.spec.whatwg.org/#the-documentorshadowroot-interface)

Status: reviewed (2026-11-01)
> `DocumentOrShadowRoot`; its `activeElement` is reviewed with focus.

#### [3.1.3 Ancestor origins](https://html.spec.whatwg.org/#ancestor-origins)

Status: n/a (no origins or frame tree: `Location::AncestorOrigins` is empty)

- [x] [internal ancestor origin objects list (Document)](https://html.spec.whatwg.org/#concept-document-internal-ancestor-origin-objects-list) — A Document object has an associated internal ancestor origin objects list, initially null.

- [x] [internal ancestor origin objects list creation steps](https://html.spec.whatwg.org/#internal-ancestor-origin-objects-list-creation-steps) — The internal ancestor origin objects list creation steps given a Document object document and a referrer policy referrerPolicy are:
  - 1. Let output be « ».
  - 2. Let parentDoc be document's container document.
  - 3. If parentDoc is null, then return output.
  - 4. Assert: parentDoc is fully active.
  - 5. Let ancestorOrigins be parentDoc's internal ancestor origin objects list.
  - 6. Assert: ancestorOrigins is not null.
  - 7. Let masked be false.
  - 8. If referrerPolicy is "no-referrer", then set masked to true.
  - 9. Otherwise, if referrerPolicy is "same-origin" and parentDoc's origin is not same origin with document's origin, then set masked to true.
  - 10. If masked is true, then append a new opaque origin to output.
  - 11. Otherwise, append parentDoc's origin to output.
  - 12. For each ancestorOrigin of ancestorOrigins:
    - 12a. If masked is true and ancestorOrigin is same origin with parentDoc's origin, then append a new opaque origin to output and continue.
    - 12b. Append ancestorOrigin to output and set masked to false.
  - 13. Return output.

- [x] [ancestor origins list (Document)](https://html.spec.whatwg.org/#concept-document-ancestor-origins-list) — A Document object has an associated ancestor origins list, initially null.

- [x] [ancestor origins list creation steps](https://html.spec.whatwg.org/#ancestor-origins-list-creation-steps) — The ancestor origins list creation steps given a Document object document are:
  - 1. Let ancestorOrigins be document's internal ancestor origin objects list.
  - 2. Assert: ancestorOrigins is not null.
  - 3. Let output be « ».
  - 4. For each origin of ancestorOrigins:
    - 4a. Append the serialization of origin to output.
  - 5. Return a new DOMStringList object whose associated list is output.

#### [3.1.4 Resource metadata management](https://html.spec.whatwg.org/#resource-metadata-management)

Status: reviewed (2026-11-01)
> `Document::LastModified` is the current time, the fallback for an unknown date, as responses have no headers (KNOWN-ISSUES.md). New: it wasn't implemented.
<!-- definitions: dom-document-cookie -->

- [x] [Document.referrer](https://html.spec.whatwg.org/#dom-document-referrer) — The referrer attribute must return the document's referrer.
  > n/a: no referrers until remote pages.

- [x] [cookie-averse Document object](https://html.spec.whatwg.org/#cookie-averse-document-object) — A Document object that falls into one of the following conditions is a cookie-averse Document object:
  > n/a: no cookies.

- [x] [Document.lastModified](https://html.spec.whatwg.org/#dom-document-lastmodified) — The lastModified attribute, on getting, must return the date and time of the Document's source file's last modification, in the user's local time zone, in the following format:
  - 1. The month component of the date.
  - 2. A U+002F SOLIDUS character (/).
  - 3. The day component of the date.
  - 4. A U+002F SOLIDUS character (/).
  - 5. The year component of the date.
  - 6. A U+0020 SPACE character.
  - 7. The hours component of the time.
  - 8. A U+003A COLON character (:).
  - 9. The minutes component of the time.
  - 10. A U+003A COLON character (:).
  - 11. The seconds component of the time.

#### [3.1.5 Reporting document loading status](https://html.spec.whatwg.org/#reporting-document-loading-status)

Status: reviewed (2026-11-01)
> `Document::ReadyState`, `DocumentAlgorithms::UpdateCurrentDocumentReadiness` (a document a frame loads, with a window, is the one associated with an HTML parser), and `Document::Parsing` (has an active parser). The load timing info is `DocumentLoadTimingInfo`.
<!-- definitions: current-document-readiness load-timing-info previous-document-unload-timing was-created-via-cross-origin-redirects document-load-timing-info navigation-start-time dom-interactive-time dom-content-loaded-event-start-time dom-content-loaded-event-end-time dom-complete-time load-event-start-time load-event-end-time document-unload-timing-info unload-event-start-time unload-event-end-time -->

- [x] [Document.readyState](https://html.spec.whatwg.org/#dom-document-readystate) — The readyState getter steps are to return this's current document readiness.

- [x] [update the current document readiness](https://html.spec.whatwg.org/#update-the-current-document-readiness) — To update the current document readiness for Document document to readinessValue:
  - 1. If document's current document readiness equals readinessValue, then return.
  - 2. Set document's current document readiness to readinessValue.
  - 3. If document is associated with an HTML parser:
    - 3a. Let now be the current high resolution time given document's relevant global object.
    - 3b. If readinessValue is "complete", and document's load timing info's DOM complete time is 0, then set document's load timing info's DOM complete time to now.
    - 3c. Otherwise, if readinessValue is "interactive", and document's load timing info's DOM interactive time is 0, then set document's load timing info's DOM interactive time to now.
  - 4. Fire an event named readystatechange at document.

- [x] [active parser](https://html.spec.whatwg.org/#active-parser) — A Document is said to have an active parser if it is associated with an HTML parser or an XML parser that has not yet been stopped or aborted.

#### [3.1.6 Render-blocking mechanism](https://html.spec.whatwg.org/#render-blocking-mechanism)

Status: reviewed (2026-11-01)
> `RenderBlocking`, with the render-blocking element set on the document. Fixed: render-blocking didn't check the document is render-blocked, and a frameset didn't count as the body element. The timeout and skipping render-blocked documents in the rendering update are FIXMEs (KNOWN-ISSUES.md).
<!-- definitions: render-blocking-element-set -->

- [x] [allows adding render-blocking elements](https://html.spec.whatwg.org/#allows-adding-render-blocking-elements) — A Document document allows adding render-blocking elements if document's content type is "text/html" and the body element of document is null.

- [x] [render-blocked](https://html.spec.whatwg.org/#render-blocked) — A Document document is render-blocked if both of the following are true:

- [x] [render-blocking](https://html.spec.whatwg.org/#render-blocking) — An element el is render-blocking if el's node document document is render-blocked, and el is in document's render-blocking element set.

- [x] [block rendering](https://html.spec.whatwg.org/#block-rendering) — To block rendering on an element el:
  - 1. Let document be el's node document.
  - 2. If document allows adding render-blocking elements, then append el to document's render-blocking element set.

- [x] [unblock rendering](https://html.spec.whatwg.org/#unblock-rendering) — To unblock rendering on an element el:
  - 1. Let document be el's node document.
  - 2. Remove el from document's render-blocking element set.

#### [3.1.7 DOM tree accessors](https://html.spec.whatwg.org/#dom-tree-accessors)

Status: reviewed (2026-11-01)
> `DOMTreeAccessors` (the html, head, title and body elements) and the `Document` accessors; the IDL `body` is `BodyOrFrameset`/`SetBodyOrFrameset`, and `Body()` is the first body child.
> Fixed: the title element was only looked for among the head's children, setting the title appended a new title to the document element rather than the head, the body element left out framesets, setting it replaced within the new value's parent (crashing for a new body), and `document.dir` set the head's `dir`. New: `images`, `embeds`, `plugins`, `links`, `forms`, `scripts` and `getElementsByName()` weren't implemented.

- [x] [The html element](https://html.spec.whatwg.org/#the-html-element-2) — The html element of a document is its document element, if it's an html element, and null otherwise.

- [x] [The head element](https://html.spec.whatwg.org/#the-head-element-2) — The head element of a document is the first head element that is a child of the html element, if there is one, or null otherwise.

- [x] [Document.head](https://html.spec.whatwg.org/#dom-document-head) — The head attribute, on getting, must return the head element of the document (a head element or null).

- [x] [The title element](https://html.spec.whatwg.org/#the-title-element-2) — The title element of a document is the first title element in the document (in tree order), if there is one, or null otherwise.

- [x] [Document.title](https://html.spec.whatwg.org/#document.title) — The title attribute must, on getting, run the following algorithm:
  - 1. If the document element is an SVG svg element, then let value be the child text content of the first SVG title element that is a child of the document element.
  - 2. Otherwise, let value be the child text content of the title element, or the empty string if the title element is null.
  - 3. Strip and collapse ASCII whitespace in value.
  - 4. Return value.

- [x] [The body element](https://html.spec.whatwg.org/#the-body-element-2) — The body element of a document is the first of the html element's children that is either a body element or a frameset element, or null if there is no such element.

- [x] [Document.body](https://html.spec.whatwg.org/#dom-document-body) — The body attribute, on getting, must return the body element of the document (either a body element, a frameset element, or null). On setting, the following algorithm must be run:
  - 1. If the new value is not a body or frameset element, then throw a "HierarchyRequestError" DOMException.
  - 2. Otherwise, if the new value is the same as the body element, return.
  - 3. Otherwise, if the body element is not null, then replace the body element with the new value within the body element's parent and return.
  - 4. Otherwise, if there is no document element, throw a "HierarchyRequestError" DOMException.
  - 5. Otherwise, the body element is null, but there's a document element. Append the new value to the document element.

- [x] [Document.images](https://html.spec.whatwg.org/#dom-document-images) — The images attribute must return an HTMLCollection rooted at the Document node, whose filter matches only img elements.

- [x] [Document.embeds](https://html.spec.whatwg.org/#dom-document-embeds) — The embeds attribute must return an HTMLCollection rooted at the Document node, whose filter matches only embed elements.

- [x] [Document.plugins](https://html.spec.whatwg.org/#dom-document-plugins) — The plugins attribute must return the same object as that returned by the embeds attribute.

- [x] [Document.links](https://html.spec.whatwg.org/#dom-document-links) — The links attribute must return an HTMLCollection rooted at the Document node, whose filter matches only a elements with href attributes and area elements with href attributes.

- [x] [Document.forms](https://html.spec.whatwg.org/#dom-document-forms) — The forms attribute must return an HTMLCollection rooted at the Document node, whose filter matches only form elements.

- [x] [Document.scripts](https://html.spec.whatwg.org/#dom-document-scripts) — The scripts attribute must return an HTMLCollection rooted at the Document node, whose filter matches only script elements.

- [x] [Document.getElementsByName(elementName)](https://html.spec.whatwg.org/#dom-document-getelementsbyname) — The getElementsByName(elementName) method steps are to return a live NodeList containing all the HTML elements in that document that have a name attribute whose value is identical to the elementName argument, in tree order. When the method is invoked on a Document object again with the same argument, the user agent may return the same as the object returned by the earlier call. In other cases, a new NodeList object must be returned.

- [x] [Document.currentScript](https://html.spec.whatwg.org/#dom-document-currentscript) — The currentScript attribute, on getting, must return the value to which it was most recently set. When the Document is created, the currentScript must be initialized to null.
  > n/a: no scripting, so there is never a current script (removed).

- [x] [To determine the value of a named property name for a Document, the user agent…](https://html.spec.whatwg.org/#dom-document-currentscript) — To determine the value of a named property name for a Document, the user agent must return the value obtained using the following steps:
  - 1. Let elements be the list of named elements with the name name that are in a document tree with the Document as their root.
  - 2. If elements has only one element, and that element is an iframe element, and that iframe element's content navigable is not null, then return the active WindowProxy of the element's content navigable.
  - 3. Otherwise, if elements has only one element, return that element.
  - 4. Otherwise, return an HTMLCollection rooted at the Document node, whose filter matches only named elements with the name name.
  > n/a: the named properties (`document[name]`) are only reachable from script.

- [x] [named element](https://html.spec.whatwg.org/#dom-document-nameditem-filter) — An element is a named element with the name name if name is not the empty string and any of the following are true:
  > n/a: only for the named properties, which are only reachable from script.

### [3.2 Elements](https://html.spec.whatwg.org/#elements)

Status: reviewed (2026-11-01)
> Headings only.

#### [3.2.1 Semantics](https://html.spec.whatwg.org/#semantics-2)

Status: n/a (author conformance)

#### [3.2.2 Elements in the DOM](https://html.spec.whatwg.org/#elements-in-the-dom)

Status: reviewed (2026-11-01)
> `HTMLElementFactory::TryCreate` (steps 1 to 5) and `ElementFactory::Create` (steps 6 and 7). `<frame>` has no interface, so it's an `HTMLUnknownElement` (DIFFERENCES.md). What an element represents is author conformance.
<!-- definitions: represents -->

- [x] [The element interface for an element with name name in the HTML namespace is de…](https://html.spec.whatwg.org/#referenced) — The element interface for an element with name name in the HTML namespace is determined as follows:
  - 1. If name is applet, bgsound, blink, isindex, keygen, multicol, nextid, or spacer, then return HTMLUnknownElement.
  - 2. If name is acronym, basefont, big, center, nobr, noembed, noframes, plaintext, rb, rtc, strike, or tt, then return HTMLElement.
  - 3. If name is listing or xmp, then return HTMLPreElement.
  - 4. Otherwise, if this specification defines an interface appropriate for the element type corresponding to the local name name, then return that interface.
  - 5. If other applicable specifications define an appropriate interface for name, then return the interface they define.
  - 6. If name is a valid custom element name, then return HTMLElement.
  - 7. Return HTMLUnknownElement.

#### [3.2.3 HTML element constructors](https://html.spec.whatwg.org/#html-element-constructors)

Status: n/a (no scripting: custom elements are defined in C++)

- [x] [Interfaces declared with constructor operations that are annotated with the [HT…](https://html.spec.whatwg.org/#htmlconstructor) — Interfaces declared with constructor operations that are annotated with the [HTMLConstructor] extended attribute have the following overridden constructor steps:
  - 1. If NewTarget is equal to the active function object, then throw a TypeError.
  - 2. Let registry be null.
  - 3. If the surrounding agent's active custom element constructor map[NewTarget] exists, then set registry to the surrounding agent's active custom element constructor map[NewTarget].
  - 4. Otherwise, set registry to the current global object's associated Document's custom element registry.
  - 5. Let definition be the item in registry's custom element definition set with constructor equal to NewTarget. If there is no such item, then throw a TypeError.
  - 6. Let isValue be null.
  - 7. If definition's local name is equal to definition's name (i.e., definition is for an autonomous custom element):
    - 7a. If the active function object is not HTMLElement, then throw a TypeError.
  - 8. Otherwise (i.e., if definition is for a customized built-in element):
    - 8a. Let valid local names be the list of local names for elements defined in this specification or in other applicable specifications that use the active function object as their element interface.
    - 8b. If valid local names does not contain definition's local name, then throw a TypeError.
    - 8c. Set isValue to definition's name.
  - 9. If definition's construction stack is empty:
    - 9a. Let element be the result of internally creating a new object implementing the interface to which the active function object corresponds, given the current realm and NewTarget.
    - 9b. Set element's node document to the current global object's associated Document.
    - 9c. Set element's namespace to the HTML namespace.
    - 9d. Set element's namespace prefix to null.
    - 9e. Set element's local name to definition's local name.
    - 9f. Set element's custom element registry to registry.
    - 9g. Set element's custom element state to "custom".
    - 9h. Set element's custom element definition to definition.
    - 9i. Set element's is value to isValue.
    - 9j. Return element.
  - 10. Let prototype be ? Get(NewTarget, "prototype").
  - 11. If prototype is not an Object:
    - 11a. Let realm be ? GetFunctionRealm(NewTarget).
    - 11b. Set prototype to the interface prototype object of realm whose interface is the same as the interface of the active function object.
  - 12. Let element be the last entry in definition's construction stack.
  - 13. If element is an already constructed marker, then throw a TypeError.
  - 14. Perform ? element.[[SetPrototypeOf]](prototype).
  - 15. Replace the last entry in definition's construction stack with an already constructed marker.
  - 16. Return element.

#### [3.2.4 Element definitions](https://html.spec.whatwg.org/#element-definitions)

Status: n/a (author conformance)
<!-- definitions: concept-element-categories concept-element-contexts concept-element-content-model concept-element-tag-omission concept-element-attributes concept-element-accessibility-considerations concept-element-sanitization concept-element-dom -->

##### [3.2.4.1 Attributes](https://html.spec.whatwg.org/#attributes)

Status: n/a (author conformance)

#### [3.2.5 Content models](https://html.spec.whatwg.org/#content-models)

Status: n/a (author conformance)
<!-- definitions: concept-html-contents inter-element-whitespace -->

##### [3.2.5.1 The "nothing" content model](https://html.spec.whatwg.org/#the-nothing-content-model)

Status: n/a (author conformance)
<!-- definitions: concept-content-nothing -->

##### [3.2.5.2 Kinds of content](https://html.spec.whatwg.org/#kinds-of-content)

Status: n/a (author conformance)
<!-- definitions: content-categories -->

###### [3.2.5.2.1 Metadata content](https://html.spec.whatwg.org/#metadata-content)

Status: n/a (author conformance)
<!-- definitions: metadata-content-2 -->

###### [3.2.5.2.2 Flow content](https://html.spec.whatwg.org/#flow-content)

Status: n/a (author conformance)
<!-- definitions: flow-content-2 -->

###### [3.2.5.2.3 Sectioning content](https://html.spec.whatwg.org/#sectioning-content)

Status: n/a (author conformance)
<!-- definitions: sectioning-content-2 -->

###### [3.2.5.2.4 Heading content](https://html.spec.whatwg.org/#heading-content)

Status: n/a (author conformance)
<!-- definitions: heading-content-2 -->

###### [3.2.5.2.5 Phrasing content](https://html.spec.whatwg.org/#phrasing-content)

Status: n/a (author conformance)
<!-- definitions: phrasing-content-2 text-content -->

###### [3.2.5.2.6 Embedded content](https://html.spec.whatwg.org/#embedded-content-2)

Status: reviewed (2026-11-01)
> Author conformance; fallback content is reviewed with `<canvas>` and `<object>`.
<!-- definitions: embedded-content-category fallback-content -->

###### [3.2.5.2.7 Interactive content](https://html.spec.whatwg.org/#interactive-content)

Status: reviewed (2026-11-01)
> `HTMLElement::IsInteractiveContent`, which label activation and `<summary>` use. Fixed: `<embed>`, `<iframe>` and `<img usemap>` weren't interactive content. `<audio controls>` and `<video controls>` wait on media playback.
<!-- definitions: interactive-content-2 -->

###### [3.2.5.2.8 Palpable content](https://html.spec.whatwg.org/#palpable-content)

Status: n/a (author conformance)
<!-- definitions: palpable-content-2 -->

###### [3.2.5.2.9 Script-supporting elements](https://html.spec.whatwg.org/#script-supporting-elements)

Status: n/a (author conformance)
<!-- definitions: script-supporting-elements-2 -->

##### [3.2.5.3 Transparent content models](https://html.spec.whatwg.org/#transparent-content-models)

Status: n/a (author conformance)
<!-- definitions: transparent -->

##### [3.2.5.4 Paragraphs](https://html.spec.whatwg.org/#paragraphs)

Status: n/a (author conformance)
<!-- definitions: paragraph -->

#### [3.2.6 Global attributes](https://html.spec.whatwg.org/#global-attributes)

Status: reviewed (2026-11-01)
> `class`, `id` and `slot` are the DOM's (reviewed with it); the other global attributes are reviewed with their sections and elements.
<!-- definitions: classes the-id-attribute attr-slot -->

##### [3.2.6.1 The title attribute](https://html.spec.whatwg.org/#the-title-attribute)

Status: reviewed (2026-11-01)
> Nothing shows advisory information yet (KNOWN-ISSUES.md: titles are never shown); the attribute is reflected by `HTMLElement::Title`.
<!-- definitions: attr-title -->

- [x] [advisory information](https://html.spec.whatwg.org/#advisory-information) — The advisory information of an element is the value that the following algorithm returns, with the algorithm being aborted once a value is returned. When the algorithm returns the empty string, then there is no advisory information.
  - 1. If the element has a title attribute, then return the result of running normalize newlines on its value.
  - 2. If the element has a parent element, then return the parent element's advisory information.
  - 3. Return the empty string.
  > Missing: nothing shows titles (KNOWN-ISSUES.md).

##### [3.2.6.2 The lang and xml:lang attributes](https://html.spec.whatwg.org/#the-lang-and-xml:lang-attributes)

Status: reviewed (2026-11-01)
> `css::EffectiveLang` (the language of a node), used by `:lang()`, locales and canvas text. Fixed: `lang` counted on every element rather than only HTML and SVG ones (DIFFERENCES.md, from upstream). The Content-Language header is a FIXME (KNOWN-ISSUES.md).
<!-- definitions: attr-lang attr-xml-lang concept-explicitly-unknown -->

- [x] [language (Node)](https://html.spec.whatwg.org/#language) — To determine the language of a node, user agents must use the first appropriate step in the following list:
  - • If the node is an element that has a lang attribute in the XML namespace set: Use the value of that attribute.
  - • If the node is an HTML element or an element in the SVG namespace, and it has a lang in no namespace attribute set: Use the value of that attribute.
  - • If the node's parent is a shadow root: Use the language of that shadow root's host.
  - • If the node's parent element is not null: Use the language of that parent element.
  - • Otherwise: If there is a pragma-set default language set, then that is the language of the node. If there is no pragma-set default language set, then language information from a higher-level protocol (such as HTTP), if any, must be used as the final fallback language instead. In the absence of any such language information, and in cases where the higher-level protocol reports multiple languages,…

##### [3.2.6.3 The translate attribute](https://html.spec.whatwg.org/#the-translate-attribute)

Status: reviewed (2026-11-01)
> `HTMLElement::Translate` and `IsTranslateEnabled` (translation mode). Fixed: `translate` was false without a `translate=yes` rather than inheriting the parent's translation mode. Translatable attributes are n/a: nothing translates pages. Removed `TranslateState.hpp`, an unused copy of `CORSSettingsState.hpp`.
<!-- definitions: attr-translate translate-enabled no-translate -->

- [x] [Inherit](https://html.spec.whatwg.org/#attr-translate-inherit-state) — The attribute's missing value default and invalid value default are both the Inherit state, and its empty value default is the Yes state.

- [x] [translation mode](https://html.spec.whatwg.org/#translation-mode) — Each element (even non-HTML elements) has a translation mode, which is in either the translate-enabled state or the no-translate state. If an HTML element's translate attribute is in the Yes state, then the element's translation mode is in the translate-enabled state; otherwise, if the element's translate attribute is in the No state, then the element's translation mode is in the no-translate state. Otherwise, either the element's translate attribute is in the Inherit state, or the element is not an HTML element and thus does not have a translate attribute; in either case, the element's translation mode is in the same state as its parent element's, if any, or in the translate-enabled state, if the element's parent element is null.

- [x] [translatable attributes](https://html.spec.whatwg.org/#translatable-attributes) — The following attributes are translatable attributes:
  > n/a: nothing translates pages.

- [x] [HTMLElement.translate](https://html.spec.whatwg.org/#dom-translate) — The translate IDL attribute must, on getting, return true if the element's translation mode is translate-enabled, and false otherwise. On setting, it must set the content attribute's value to "yes" if the new value is true, and set the content attribute's value to "no" otherwise.

##### [3.2.6.4 The dir attribute](https://html.spec.whatwg.org/#the-dir-attribute)

Status: reviewed (2026-11-01)
> `ElementTextDirection` (the directionality, auto directionality, contained text auto directionality, text node directionality and parent directionality), with `HTMLTextFormControlElement::DirAutoUsesValue` (auto-directionality form-associated elements); `HTMLElement::Dir` and `Document::Dir`.
> Fixed: a slot's assigned elements used their own auto directionality rather than their contained text with canExcludeRoot (DIFFERENCES.md, from upstream). The directionality of an attribute is used only for alt text, which takes the image's direction (KNOWN-ISSUES.md).
<!-- definitions: attr-dir concept-ltr concept-rtl auto-directionality-can-exclude-root -->

- [x] [Undefined](https://html.spec.whatwg.org/#attr-dir-undefined-state) — The attribute's missing value default and invalid value default are both the Undefined state.

- [x] [directionality](https://html.spec.whatwg.org/#the-directionality) — The directionality of an element (any element, not just an HTML element) is either 'ltr' or 'rtl'. To compute the directionality given an element element, switch on element's dir attribute state:
  - • LTR: Return 'ltr'.
  - • RTL: Return 'rtl'.
  - • Auto:
    - 1. Let result be the auto directionality of element.
    - 2. If result is null, then return 'ltr'.
    - 3. Return result.
  - • Undefined: If element is a bdi element If element is an input element whose type attribute is in the Telephone stateReturn 'ltr'.OtherwiseReturn the parent directionality of element.
    - 1. Let result be the auto directionality of element.
    - 2. If result is null, then return 'ltr'.
    - 3. Return result.

- [x] [auto-directionality form-associated elements](https://html.spec.whatwg.org/#auto-directionality-form-associated-elements) — The auto-directionality form-associated elements are:

- [x] [auto directionality](https://html.spec.whatwg.org/#auto-directionality) — To compute the auto directionality given an element element:
  - 1. If element is an auto-directionality form-associated element:
    - 1a. If element's value contains a character of bidirectional character type AL or R, and there is no character of bidirectional character type L anywhere before it in the element's value, then return 'rtl'. [BIDI]
    - 1b. If element's value is not the empty string, then return 'ltr'.
    - 1c. Return null.
  - 2. If element is a slot element whose root is a shadow root and element's assigned nodes are not empty:
    - 2a. For each node child of element's assigned nodes:
      - 2a.i. Let childDirection be null.
      - 2a.ii. If child is a Text node, then set childDirection to the text node directionality of child.
      - 2a.iii. Otherwise:
        - 2a.iii.A. Assert: child is an Element node.
        - 2a.iii.B. Set childDirection to the contained text auto directionality of child with canExcludeRoot set to true.
      - 2a.iv. If childDirection is not null, then return childDirection.
    - 2b. Return null.
  - 3. Return the contained text auto directionality of element with canExcludeRoot set to false.

- [x] [contained text auto directionality](https://html.spec.whatwg.org/#contained-text-auto-directionality) — To compute the contained text auto directionality of an element element with a boolean canExcludeRoot:
  - 1. For each node descendant of element's descendants, in tree order:
    - 1a. If any of descendant; any ancestor element of descendant that is a descendant of element; if canExcludeRoot is true, element is one of a bdi element; a script element; a style element; a textarea element; an element whose dir attribute is not in the Undefined state then continue.
    - 1b. If descendant is a slot element whose root is a shadow root, then return the directionality of that shadow root's host.
    - 1c. If descendant is not a Text node, then continue.
    - 1d. Let result be the text node directionality of descendant.
    - 1e. If result is not null, then return result.
  - 2. Return null.

- [x] [text node directionality](https://html.spec.whatwg.org/#text-node-directionality) — To compute the text node directionality given a Text node text:
  - 1. If text's data does not contain a code point whose bidirectional character type is L, AL, or R, then return null. [BIDI]
  - 2. Let codePoint be the first code point in text's data whose bidirectional character type is L, AL, or R.
  - 3. If codePoint is of bidirectional character type AL or R, then return 'rtl'.
  - 4. If codePoint is of bidirectional character type L, then return 'ltr'.

- [x] [parent directionality](https://html.spec.whatwg.org/#parent-directionality) — To compute the parent directionality given an element element:
  - 1. Let parentNode be element's parent node.
  - 2. If parentNode is a shadow root, then return the directionality of parentNode's host.
  - 3. If parentNode is an element, then return the directionality of parentNode.
  - 4. Return 'ltr'.

- [x] [directionality of an attribute](https://html.spec.whatwg.org/#directionality-of-the-attribute) — The directionality of an attribute of an HTML element, which is used when the text of that attribute is to be included in the rendering in some manner, is determined as per the first appropriate set of steps from the following list:
  - • If the attribute is a directionality-capable attribute and the element's dir attribute is in the Auto state: Find the first character (in logical order) of the attribute's value that is of bidirectional character type L, AL, or R. [BIDI] If such a character is found and it is of bidirectional character type AL or R, the directionality of the attribute is 'rtl'. Otherwise, the directionality of th…
  - • Otherwise: The directionality of the attribute is the same as the element's directionality.
  > Missing for alt text with dir=auto (KNOWN-ISSUES.md); nothing else renders attribute text.

- [x] [directionality-capable attributes](https://html.spec.whatwg.org/#directionality-capable-attribute) — The following attributes are directionality-capable attributes:
  > Only alt text is rendered (KNOWN-ISSUES.md).

- [x] [HTMLElement.dir](https://html.spec.whatwg.org/#dom-dir) — The dir IDL attribute on an element must reflect the dir content attribute of that element, limited to only known values.

- [x] [Document.dir](https://html.spec.whatwg.org/#dom-document-dir) — The dir IDL attribute on Document objects must reflect the dir content attribute of the html element, if any, limited to only known values. If there is no such element, then the attribute must return the empty string and do nothing on setting.

##### [3.2.6.5 The style attribute](https://html.spec.whatwg.org/#the-style-attribute)

Status: reviewed (2026-11-01)
> `StyledElement` parses it as a declaration block, and `style` is its CSSOM (reviewed with CSS Style Attributes and the CSSOM).
<!-- definitions: attr-style -->

##### [3.2.6.6 Embedding custom non-visible data with the data-* attributes](https://html.spec.whatwg.org/#embedding-custom-non-visible-data-with-the-data-*-attributes)

Status: reviewed (2026-11-01)
> `Element::Dataset` and `DatasetDOMStringMap`, which looks at attributes' local names, as upstream. A custom data attribute is checked by the sanitizer (`SanitizerAlgorithms`).
<!-- definitions: attr-data-* concept-domstringmap-element -->

- [x] [custom data attribute](https://html.spec.whatwg.org/#custom-data-attribute) — A custom data attribute is an attribute in no namespace whose name starts with the string "data-", has at least one character after the hyphen, is a valid attribute local name, and contains no ASCII upper alphas.

- [x] [HTMLOrSVGOrMathMLElement.dataset](https://html.spec.whatwg.org/#dom-dataset) — The dataset IDL attribute provides convenient accessors for all the data-* attributes on an element. On getting, the dataset IDL attribute must return a DOMStringMap whose associated element is this element.

- [x] [get a DOMStringMap's name-value pairs](https://html.spec.whatwg.org/#concept-domstringmap-pairs) — To get a DOMStringMap's name-value pairs, run the following algorithm:
  - 1. Let list be an empty list of name-value pairs.
  - 2. For each content attribute on the DOMStringMap's associated element whose first five characters are the string "data-" and whose remaining characters (if any) do not include any ASCII upper alphas, in the order that those attributes are listed in the element's attribute list, add a name-value pair to list whose name is the attribute's name with the first five characters removed and whose value is…
  - 3. For each name in list, for each U+002D HYPHEN-MINUS character (-) in the name that is followed by an ASCII lower alpha, remove the U+002D HYPHEN-MINUS character (-) and replace the character that followed it by the same character converted to ASCII uppercase.
  - 4. Return list.

- [x] [To set the value of a new named property or set the value of an existing named…](https://html.spec.whatwg.org/#concept-domstringmap-pairs) — To set the value of a new named property or set the value of an existing named property for a DOMStringMap, given a property name name and a new value value, run the following steps:
  - 1. If name contains a U+002D HYPHEN-MINUS character (-) followed by an ASCII lower alpha, then throw a "SyntaxError" DOMException.
  - 2. For each ASCII upper alpha in name, insert a U+002D HYPHEN-MINUS character (-) before the character and replace the character with the same character converted to ASCII lowercase.
  - 3. Insert the string data- at the front of name.
  - 4. If name is not a valid attribute local name, then throw an "InvalidCharacterError" DOMException.
  - 5. Set an attribute value for the DOMStringMap's associated element using name and value.

- [x] [To delete an existing named property name for a DOMStringMap, run the following…](https://html.spec.whatwg.org/#concept-domstringmap-pairs) — To delete an existing named property name for a DOMStringMap, run the following steps:
  - 1. For each ASCII upper alpha in name, insert a U+002D HYPHEN-MINUS character (-) before the character and replace the character with the same character converted to ASCII lowercase.
  - 2. Insert the string data- at the front of name.
  - 3. Remove an attribute by name given name and the DOMStringMap's associated element.

#### [3.2.7 The innerText and outerText properties](https://html.spec.whatwg.org/#the-innertext-idl-attribute)

Status: unreviewed

- [ ] [get the text steps](https://html.spec.whatwg.org/#get-the-text-steps) — The get the text steps, given an HTMLElement element, are:
  - 1. If element is not being rendered or if the user agent is a non-CSS user agent, then return element's descendant text content.
  - 2. Let results be a new empty list.
  - 3. For each child node node of element:
    - 3a. Let current be the list resulting from running the rendered text collection steps with node. Each item in results will either be a string or a positive integer (a required line break count).
    - 3b. For each item item in current, append item to results.
  - 4. Remove any items from results that are the empty string.
  - 5. Remove any runs of consecutive required line break count items at the start or end of results.
  - 6. Replace each remaining run of consecutive required line break count items with a string consisting of as many U+000A LF code points as the maximum of the values in the required line break count items.
  - 7. Return the concatenation of the string items in results.

- [ ] [HTMLElement.innerText](https://html.spec.whatwg.org/#dom-innertext) — The innerText and outerText getter steps are to return the result of running get the text steps with this.

- [ ] [rendered text collection steps](https://html.spec.whatwg.org/#rendered-text-collection-steps) — The rendered text collection steps, given a node node, are as follows:
  - 1. Let items be the result of running the rendered text collection steps with each child node of node in tree order, and then concatenating the results to a single list.
  - 2. If node's computed value of 'visibility' is not 'visible', then return items.
  - 3. If node is not being rendered, then return items. For the purpose of this step, the following elements must act as described if the computed value of the 'display' property is not 'none': select elements have an associated non-replaced inline CSS box whose child boxes include only those of optgroup and option element descendant nodes;; optgroup elements have an associated non-replaced block-level…
  - 4. If node is a Text node, then for each CSS text box produced by node, in content order, compute the text of the box after application of the CSS 'white-space' processing rules and 'text-transform' rules, set items to the list of the resulting strings, and return items. The CSS 'white-space' processing rules are slightly modified: collapsible spaces at the end of lines are always collapsed, but the…
  - 5. If node is a br element, then append a string containing a single U+000A LF code point to items.
  - 6. If node's computed value of 'display' is 'table-cell', and node's CSS box is not the last 'table-cell' box of its enclosing 'table-row' box, then append a string containing a single U+0009 TAB code point to items.
  - 7. If node's computed value of 'display' is 'table-row', and node's CSS box is not the last 'table-row' box of the nearest ancestor 'table' box, then append a string containing a single U+000A LF code point to items.
  - 8. If node is a p element, then append 2 (a required line break count) at the beginning and end of items.
  - 9. If node's used value of 'display' is block-level or 'table-caption', then append 1 (a required line break count) at the beginning and end of items. [CSSDISPLAY]
  - 10. Return items.

- [ ] [set the inner text steps](https://html.spec.whatwg.org/#set-the-inner-text-steps) — The set the inner text steps, given an HTMLElement element, and a string value are:
  - 1. Let fragment be the rendered text fragment for value given element's node document.
  - 2. Replace all with fragment within element.

- [ ] [The innerText setter steps](https://html.spec.whatwg.org/#dom-innertext) — The innerText setter steps are to run set the inner text steps with this and the given value.

- [ ] [The outerText setter steps](https://html.spec.whatwg.org/#dom-outertext) — The outerText setter steps are:
  - 1. If this's parent is null, then throw a "NoModificationAllowedError" DOMException.
  - 2. Let next be this's next sibling.
  - 3. Let previous be this's previous sibling.
  - 4. Let fragment be the rendered text fragment for the given value given this's node document.
  - 5. If fragment has no children, then append a new Text node whose data is the empty string and node document is this's node document to fragment.
  - 6. Replace this with fragment within this's parent.
  - 7. If next is non-null and next's previous sibling is a Text node, then merge with the next text node given next's previous sibling.
  - 8. If previous is a Text node, then merge with the next text node given previous.

- [ ] [rendered text fragment](https://html.spec.whatwg.org/#rendered-text-fragment) — The rendered text fragment for a string input given a Document document is the result of running the following steps:
  - 1. Let fragment be the result of creating a document fragment given document.
  - 2. Let position be a position variable for input, initially pointing at the start of input.
  - 3. Let text be the empty string.
  - 4. While position is not past the end of input:
    - 4a. Collect a sequence of code points that are not U+000A LF or U+000D CR from input given position, and set text to the result.
    - 4b. If text is not the empty string, then append a new Text node whose data is text and node document is document to fragment.
    - 4c. While position is not past the end of input, and the code point at position is either U+000A LF or U+000D CR:
      - 4c.i. If the code point at position is U+000D CR and the next code point is U+000A LF, then advance position to the next code point in input.
      - 4c.ii. Advance position to the next code point in input.
      - 4c.iii. Append the result of creating an element given document, "br", and the HTML namespace to fragment.
  - 5. Return fragment.

- [ ] [merge with the next text node](https://html.spec.whatwg.org/#merge-with-the-next-text-node) — To merge with the next text node given a Text node node:
  - 1. Let next be node's next sibling.
  - 2. If next is not a Text node, then return.
  - 3. Replace data with node, node's data's length, 0, and next's data.
  - 4. Remove next.

#### [3.2.8 Requirements relating to the bidirectional algorithm](https://html.spec.whatwg.org/#requirements-relating-to-the-bidirectional-algorithm)

Status: unreviewed

##### [3.2.8.1 Authoring conformance criteria for bidirectional-algorithm formatting characters](https://html.spec.whatwg.org/#authoring-conformance-criteria-for-bidirectional-algorithm-formatting-characters)

Status: unreviewed

##### [3.2.8.2 User agent conformance criteria](https://html.spec.whatwg.org/#user-agent-conformance-criteria)

Status: unreviewed

#### [3.2.9 Requirements related to ARIA and to platform accessibility APIs](https://html.spec.whatwg.org/#wai-aria)

Status: unreviewed

- [ ] [Similarly, for a custom element element, the default ARIA state and property se…](https://html.spec.whatwg.org/#wai-aria) — Similarly, for a custom element element, the default ARIA state and property semantics, for a state or property named stateOrProperty, are determined as follows:
  - 1. If element's attached internals is non-null:
    - 1a. If element's attached internals's get the stateOrProperty-associated element exists, then return the result of running it.
    - 1b. If element's attached internals's get the stateOrProperty-associated elements exists, then return the result of running it.
  - 2. If element's internal content attribute map[stateOrProperty] exists, then return it.
  - 3. Return the default value for stateOrProperty.
