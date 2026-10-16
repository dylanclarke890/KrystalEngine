# Krystal.Booey TODO

Open work outside the WebKit port's current slice. Port-specific detail (verdicts, differences, FIXMEs per file) stays in `PORTING.md`; this is the short list. Paths are relative to `Krystal.Booey/`.

## Infrastructure

- **Event loop and rendering loop** — nothing schedules style updates or layout; the host must call `Document::UpdateStyleIfNeeded()` or `Document::UpdateLayout()`, and `Document::EvaluateMediaQueriesAndReportChanges()` after viewport or media environment changes (FIXMEs in `src/DOM/Document.cpp`, `src/Page/LocalFrameViewLayoutContext.cpp`). Needed for timers, microtasks and the "update the rendering" steps. https://html.spec.whatwg.org/#event-loops
- **Settings** — `Page/Settings.hpp` holds the experimental and embedder flags and the font size values; the other values upstream reads from `Settings` (`textAreasAreResizable`, `hiddenUntilFoundEnabled`, `coreMathMLEnabled`, ...) are upstream's defaults in code. The media text track display quirk is never set (no document quirks).
- **Resource loading** — `<link rel=stylesheet>`, `@import`, `@font-face` sources and images aren't fetched (`CSS/Rules/ImportRule.hpp`, `CSS/Values/ImageValue.hpp`, `src/HTML/HTMLLinkElement.cpp`). https://html.spec.whatwg.org/#link-type-stylesheet
- **Fonts** — fonts can be registered and measured (`Graphics/FontDatabase.hpp`, `Graphics/Font.hpp`), but `FontCascade` doesn't use them yet (font selection is layer t2b in `PORTING.md`), so font metrics (and units that depend on them) aren't real; `FontFace`/`FontFaceSet` and the `CSSFontFace*` headers are unbuilt (`CSS/Font/`). https://drafts.csswg.org/css-font-loading/
- **Frames** — `LocalFrame` has no page, loader, navigation, frame tree (no iframes), window, scripting or printing; `LocalFrameView` has no scrolling, scrollbars or visual viewport (`Page/LocalFrame.hpp`, `Page/LocalFrameView.hpp`).
- **Logging** — no channel for exceptions thrown by event listeners or for parse errors (`src/DOM/Algorithms/EventDispatcher.cpp`, `src/HTML/Parser/HTMLTreeBuilder.cpp`).

## Style

- **Hit testing and pointer events** — the host says which element is under the pointer (`Document::UpdateHoverActiveState`, `html::FocusAlgorithms::FocusForClick`); no hit testing, and no mouse, pointer or keyboard events are fired (`UIEvent` and `FocusEvent` only). https://w3c.github.io/uievents/
- **Sequential focus navigation** — no Tab order (tabindex-ordered focus navigation scopes), no focus navigation starting point. https://html.spec.whatwg.org/#sequential-focus-navigation
- **Autofocus and focus fixup** — autofocus candidates aren't flushed and the focused element isn't refocused when it stops being focusable (both run in the rendering update) (`src/HTML/Algorithms/FocusAlgorithms.cpp`). https://html.spec.whatwg.org/#flush-autofocus-candidates
- **Animations and transitions** — keyframe styles, interpolation and the tree resolver's animated updates aren't ported; `animation`/`transition` compute but nothing animates (`PORTING.md`, *Open*).
- **Other pseudo-classes** — `:target`, `:autofill`, validity (`:valid`, `:invalid`, `:in-range`, `:user-*`), `:state()`, `:popover-open`, `:modal`, `:fullscreen`, media playback states and view transitions return false (`src/CSS/Selectors/SelectorChecker.cpp`).
- **Form owners** — the form owner is the nearest ancestor form (no `form` attribute), which `:default` and the default button rely on (`src/CSS/Selectors/SelectorCheckerTestFunctions.cpp`). https://html.spec.whatwg.org/#form-owner
- **Auto directionality** — `dir=auto` and `<bdi>` are always LTR (no bidi data) (`src/CSS/Selectors/SelectorCheckerTestFunctions.cpp`). https://html.spec.whatwg.org/#the-directionality
- **Document language** — no default language from a `Content-Language` pragma or header (`EffectiveLang`). https://html.spec.whatwg.org/#pragma-set-default-language
- **Typed OM** — `styleMap`, `attributeStyleMap`, `computedStyleMap()` and `css/typedom` aren't ported; needs `DOMMatrix` and `ParseTransformRaw` first (`CSS/Properties/Consumers/Transform.hpp`). https://drafts.css-houdini.org/css-typed-om/
- **CSSOM** — async `CSSStyleSheet.replace()` (promises); `getComputedStyle` is a free function until there's a window (`CSS/CSSComputedStyleDeclaration.hpp`).
- **Render tree** — `RenderTreeUpdater` and the seam's `RenderView` are stand-ins; no layout, so no used values, size container queries, container units, `Style::Difference` or anchor positioning (`Style/Seam.hpp`, `PORTING.md` *Open*).

## DOM

- **Mutation observers** — records are queued but never delivered (no microtasks); transient registered observers aren't added on removal (`src/DOM/Algorithms/MutationObserverAlgorithms.cpp`, `src/DOM/Algorithms/MutationAlgorithms.cpp`). https://dom.spec.whatwg.org/#notify-mutation-observers
- **Custom elements** — creation, upgrades and callback reactions are TODOs throughout the mutation, attribute and parser algorithms (`TODO(impl): CUSTOM-ELEMENTS`). https://html.spec.whatwg.org/#custom-elements
- **Event handler attributes** — no `EventHandler` IDL attributes (`onabort`, `onslotchange`, the global `on*` handlers) (`DOM/AbortSignal.hpp`, `DOM/ShadowRoot.hpp`). https://html.spec.whatwg.org/#event-handler-attributes
- **Event listeners** — awkward to add and remove with lambdas (need a comparable wrapper like `AbortAlgorithmCallback`). https://dom.spec.whatwg.org/#add-an-event-listener
- **`createEvent()`** — always returns a plain `Event` (no event factory) (`src/DOM/Document.cpp`). https://dom.spec.whatwg.org/#dom-document-createevent
- **`AbortSignal.timeout()`** — needs timers (`src/DOM/AbortSignal.cpp`). https://dom.spec.whatwg.org/#dom-abortsignal-timeout
- **Dependent abort signals** — `CreateDependentAbortSignal` keeps its source signals in a vector with messy de-duplication, and the garbage collection rule (keep a dependent signal alive while it has sources and listeners) isn't implemented (`src/DOM/Algorithms/AbortAlgorithms.cpp`). https://dom.spec.whatwg.org/#abortsignal-garbage-collection
- **`NodeFilter`** — works but is clunky to use from C++ (`DOM/NodeIterator.hpp`, `DOM/TreeWalker.hpp`). https://dom.spec.whatwg.org/#interface-nodefilter
- **Slot change signalling** — signal slots and the mutation observer microtask aren't implemented; moved nodes aren't assigned a slot (`TODO(impl): SLOTTABLES` in `src/DOM/Algorithms/MutationAlgorithms.cpp`). https://dom.spec.whatwg.org/#signaling-slot-change
- **Document encoding** — not set on creation or passed to clones; `characterSet` is always UTF-8 (`src/DOM/Document.cpp`, `src/DOM/Algorithms/NodeAlgorithms.cpp`). https://dom.spec.whatwg.org/#concept-document-encoding
- **Memory** — documents are never freed (nodes hold their document; containers don't release children). (`PORTING.md`, *Open*)
- **ID lookups** — `getElementById` and the selector ID fast paths walk the tree (no ID map per tree scope) (`DOM/Mixins/NonElementParentNode.hpp`, `src/DOM/SelectorQuery.cpp`).
- **Live collections** — `LiveNodeList`/`LiveHTMLCollection` re-walk the tree on every access (no caching) (`DOM/Internals/`).
- **Public tree accessors** — `FirstChild()`, `ParentNode()` and friends return raw pointers; the public API should return `RefPtr`. https://dom.spec.whatwg.org/#interface-node
- **Token lists** — only `classList` follows its attribute; `relList`, `sandbox`, `sizes`, `blocking`, `htmlFor` should update from `Element::OnAttributeChanged`.
- **Header weight** — `Node.hpp` includes the registered observer and mutation record types; forward declare them if possible.
- **Small fixes** — `NodeAlgorithms::Equals` compares attributes by position; `TreeQueries::NextShadowIncluding` visits a host's children before its shadow root; `Node.cpp:212` doesn't use `element1`; `NameValidation.cpp` decodes whole strings and assumes valid UTF-8.

## HTML

- **`innerText`/`outerText`** — getters and setters are TODOs (`src/HTML/HTMLElement.cpp`). https://html.spec.whatwg.org/#the-innertext-idl-attribute
- **`HTMLElement` API** — `click()`, `accessKeyLabel`, `draggable`/`spellcheck` defaults, `writingSuggestions`, `autocapitalize`, `autocorrect`, `attachInternals()`, removal and move steps (`src/HTML/HTMLElement.cpp`, `HTML/HTMLElement.hpp`).
- **`dataset`** — no `DOMStringMap` for `data-*` attributes. https://html.spec.whatwg.org/#dom-dataset
- **`title` advisory information** — the "get advisory information" algorithm. https://html.spec.whatwg.org/#the-title-attribute
- **`document.lastModified`** — not implemented. https://html.spec.whatwg.org/#dom-document-lastmodified
- **Document collections** — `images`, `embeds`, `plugins`, `links`, `forms`, `scripts`, `getElementsByName()`; `HTMLAllCollection`, `HTMLFormControlsCollection`, `HTMLOptionsCollection` (`src/DOM/Document.cpp`). https://html.spec.whatwg.org/#dom-tree-accessors
- **Microsyntaxes** — lists of floating-point numbers, lists of dimensions, dates and times (`HTML/MicroParsers/`). https://html.spec.whatwg.org/#common-microsyntaxes
- **Enumerated attributes** — several IDL attributes aren't limited to known values (`crossOrigin`, `referrerPolicy`, `decoding`, `loading`, `fetchPriority`, `as`, `http-equiv`, `<ol type>`) (`src/HTML/HTMLImageElement.cpp`, `HTMLLinkElement.cpp`, `HTMLAnchorElement.cpp`, `HTMLMetaElement.cpp`, `HTMLOListElement.cpp`).
- **`ElementInternals` reflection** — reflected attributes on `ElementInternals` (`HTML/Attributes/Reflection.hpp`). https://html.spec.whatwg.org/#reflecting-content-attributes-in-idl-attributes
- **Forms** — form owners, the form API on controls (`form`, `labels`, validity, selection, `valueAsNumber`/`valueAsDate`, `stepUp()`), `<select>`/`<option>` APIs, `<label>` control, submission (`SubmitEvent`, `FormDataEvent`) (`HTML/HTMLInputElement.hpp`, `HTMLSelectElement.hpp`, `HTMLFormElement.hpp` and the other control headers). https://html.spec.whatwg.org/#forms
- **Tables** — `caption`/`tHead`/`tBodies`/`rows` and the row, section and cell APIs (`HTML/HTMLTable*Element.hpp`). https://html.spec.whatwg.org/#the-table-element
- **Lists** — `<ol>` starting value and `<li>` ordinal value. https://html.spec.whatwg.org/#ordinal-value
- **`<meta http-equiv>`** — the pragma algorithms. https://html.spec.whatwg.org/#pragma-directives
- **`<link>`** — explicitly enabled flag on `disabled` changes, `LinkStyle` loading (`src/HTML/HTMLLinkElement.cpp`). https://html.spec.whatwg.org/#the-link-element
- **`<template>`** — constructor and parser steps aren't spec compliant (`src/HTML/HTMLTemplateElement.cpp`, `src/HTML/Parser/HTMLTreeBuilder.cpp`). https://html.spec.whatwg.org/#the-template-element
- **Render blocking** — unblock rendering when a render-blocking element disconnects or stops being render-blocking (`src/HTML/Algorithms/RenderBlocking.cpp`). https://html.spec.whatwg.org/#render-blocking-mechanism
- **Document readiness** — load timing info for parser documents; `about:srcdoc` (`src/DOM/Algorithms/DocumentAlgorithms.cpp`). https://html.spec.whatwg.org/#current-document-readiness
- **Interactive elements** — `<details>`, `<dialog>`, popovers and the command API. https://html.spec.whatwg.org/#interactive-elements
- **Hyperlinks** — `HyperlinkElementUtils`/`HTMLHyperlinkElementUtils` and activation behaviour for `<a>`/`<area>`; `<map>` `areas`. https://html.spec.whatwg.org/#api-for-hyperlink-elements
- **Embedded content** — image `naturalWidth`/`naturalHeight`, `complete`, `currentSrc`, `decode()`, source selection in `<picture>`; media, video, track, iframe and object APIs (`src/HTML/HTMLImageElement.cpp`, `HTML/HTMLMediaElement.hpp`). https://html.spec.whatwg.org/#embedded-content
- **`<script>`** — `text` attribute and `HTMLScriptElement.supports()` (`src/HTML/HTMLScriptElement.cpp`).

## HTML parser

- **Parse errors** — not reported (`src/HTML/Parser/HTMLTreeBuilder.cpp`).
- **Unimplemented steps** — `<template>` start tags in "in head", script end tags in "text", stop parsing, root insertion target, resettable and form-associated elements on creation, the `xmlns` parse error, SVG adjustments (`src/HTML/Parser/HTMLTreeBuilder.cpp`).
- **Stack of open elements** — the steps when the current node is popped (`src/HTML/Parser/HTMLElementStack.cpp`).
- **Encoding sniffer and input stream** — a TODO in `HTML/Parser/HTMLEncodingSniffer.hpp`; ASCII fast path in `HTML/Parser/InputStream.hpp`.

## SVG and MathML

- **SVG elements** — `SVGElementFactory` returns null for unknown elements (`SVG/Internals/SVGElementFactory.hpp`); document `title` for SVG documents (`src/DOM/Document.cpp`); `<use>` instances.
- **MathML elements** — `TODO(impl): MATHML-ELEMENTS`.

## Krystal.Core

- `IntrusivePtr` `Downcast`/`DynamicDowncast` can't take an explicit target and `Downcast` rejects null; `StringBuilder` and `DefaultHash<StringAtom>` aren't ported (`PORTING.md`, *Open*).

## Not supported (by choice)

Window and global objects (including `window.event`), realms, origins and `document.domain`, ancestor origins, service workers, trusted types, Content Security Policy and `nonce`, policy-controlled features, the JavaScript-only parts of the specs (`handleEvent`, IDL callbacks, HTML element constructors), scripting in the parser. Constructors that need a global object (`Document()`, `DocumentFragment()`) take a document instead. These are marked `SPEC-VIOLATION` at the call sites.
