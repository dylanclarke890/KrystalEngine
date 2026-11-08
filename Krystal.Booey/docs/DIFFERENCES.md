# Differences

Where Krystal.Booey differs from the specifications and from Chrome (the reference the goldens compare against), by area, with the reason when it's known. Behaviour that's simply missing is in `KNOWN-ISSUES.md`, and conventions are in `OVERVIEW.md`.

Record a difference here when you make one, and remove it when Booey comes back in line.

## DOM

### From the specs

- There's no console, so what would be reported to it goes unreported: failed custom element constructions and upgrades, exceptions thrown by listeners, parse errors, SVG attribute parse errors (`ReportAttributeParsingError`) and filter warnings, failed canvas reads and canvas saves past the state stack limit.
- There are no parallel queues or threads: steps the specification runs in parallel run on the event loop's thread, waiting on timers or the host's loader strategy where they wait, or synchronously where they don't, and queue their tasks back as the specification's would.
- With no garbage collector, `AbortSignal`'s garbage collection rule is kept with references: a source holds a dependent signal strongly while that signal has `abort` listeners or abort algorithms, and weakly otherwise, and an aborted signal lets go of its sources and dependents. `AbortSignal.timeout()`'s task keeps its signal alive whether or not it has listeners.
- Callbacks are C++ `Func`s rather than callback interfaces, so there's no `this` value and no exceptions to report: `ResizeObserver` and `IntersectionObserver` take the entries and the observer. Promises are replaced the same way: `CustomElementRegistry::WhenDefined` takes a callback and `Element::RequestPointerLock` returns its result through one. Animation `ready`/`finished` are `WebAnimation::WhenReady`/`WhenFinished`, which take a callback and an error callback, run as microtasks once the current ready or finished promise settles.
- `CustomEvent`'s `detail` is typed by the host rather than `any`: `CustomEvent<T>` holds a `Maybe<T>`, empty for null.
- There are no JavaScript values (`any`), structured serialization or `structuredClone()`. `document.createEvent("customevent")` makes a `CustomEvent<EmptyTag>`, as there's no detail type to give it.
- `IntersectionObserver` is notified directly in the rendering update, where the specification queues a task.
- A task's document isn't checked for being fully active, as in Chrome and WebKit: a document that was never in a frame (`createHTMLDocument()`, `DOMParser`) runs its tasks, where the specification never runs them. A document's tasks wait while it's suspended and are dropped once it's stopped, except that a subresource load finishes regardless (`StoppedDocumentTask::Run`), as other documents may share it through the memory cache.
- `DOMContentLoaded` fires synchronously at the end of parsing (`FinishedParsing`), where the specification queues a task. `load` fires from a later task (`CheckCompleted`, then `ImplicitClose`).
- Event handler content attributes (`onclick="..."`) are scripts and stay inert: setting one doesn't replace a C++ handler as the specification's would. Handlers are set from C++ with `On<Name>()`/`SetOn<Name>()`. They're `Func<void(dom::Event &)>` and cancel with `PreventDefault()`, so `onerror`'s arguments don't exist.
- A listener's global is the current target's window (`EventDispatcher::RelevantWindow`), since listeners are host code without realms. There's no `window.event` (the window's current event).
- The no-op legacy members are left out: `captureEvents()`/`releaseEvents()` on the window and the document, `document.clear()`, `Range.detach()`, `NodeIterator.detach()` and `window.blur()`.
- Nodes are equal when their node types match, where the specification asks for the same interfaces: an `HTMLDocument` equals an `XMLDocument` with equal children.
- Documents have no origin and no base URL override, so cloning a document copies neither: the copy's base URL comes from its own URL and `<base>` element. `new Document()` skips setting its origin, and getting the base URL is the document base URL.
- With no origins, there's no `document.domain` and nothing is refused as cross-origin: a linked style sheet's rules can always be read, a canvas and its patterns are always origin-clean (`getImageData()` and `toDataURL()` throw no `SecurityError`), `<use>` can load another file with no same-origin fetch mode, changing `crossorigin` reloads nothing, and a dragged image can always be dropped as a file (`EventHandler::CanDropCurrentlyDraggedImageAsFile`).
- Adopt sets the node document and registry of every node in the subtree first, then enqueues the `adoptedCallback` reactions, then runs the adopting steps, where the specification does all three for each node in turn; so a node's adopting steps see the whole subtree moved.
- There are no global objects, so `DocumentFragment`, `Text` and `Comment` are constructed with a document, as is a `ProcessingInstruction` (`ProcessingInstruction::Create`). `new Range()` is `Document::CreateRange`.
- There's no XPath (`XPathEvaluator`, `XPathExpression`, `XPathResult`, `Document.evaluate()` and `createExpression()`) and no XSLT (`XSLTProcessor`).
- A `NodeFilter` is a C++ subclass overriding `AcceptNode` rather than a callback interface or function, and returns a `FilterResult`, so it can't return a value other than `FILTER_ACCEPT`, `FILTER_REJECT` and `FILTER_SKIP`.
- Traversing a `NodeIterator` from inside its own filter throws an `InvalidStateError` before changing its candidate reference (`IteratorAlgorithms::Traverse`). The specification's traverse sets the candidate reference first, and when the filter throws, the inner traversal sets it to null, leaving the outer traversal with no candidate to continue from.
- Cloning a `ProcessingInstruction` updates the copy's attributes from its data, so its attribute map matches its data; the specification's clone sets only the target and data, leaving the copy's attribute map empty. `Document()` takes no arguments.
- The fallback base URL is always the document's URL: documents have no about base URL, which an iframe's `srcdoc` document or a document navigated to `about:blank` takes from its parent or initiator (skipped in `Document::FallbackBaseURL`).
- There's no Content Security Policy: every `<base>` is allowed, and inline `<style>` sheets and `style` attributes are parsed unchecked (`SPEC-VIOLATION(CSP)`).
- Elements have no `nonce`: it serves only Content Security Policy, so there's no `[[CryptographicNonce]]`, no `nonce` IDL attribute, and the content attribute isn't hidden when an element is connected. The `nonce` attribute stays an ordinary attribute.
- `data:` and `javascript:` base URLs are always refused.
- `HTMLTemplateElement`'s content is a `DocumentFragment` in the element's own document; there's no inert template contents owner document. So a shadow root's adopted sheets are always cleared when it moves to another document (`ShadowRoot::OnAdopt`).
- Customized built-in elements aren't supported: `is` is ignored, so an element created with an `is` value and no matching autonomous definition is uncustomized where the specification makes it `undefined` (an upgrade candidate).
- A document's `querySelector()` and `querySelectorAll()` match `:scope` as `:root`, its document element, as browsers do. Scope-match gives the document itself as the scoping root, which Selectors would make a featureless virtual root that matches nothing.
- After dispatch, nodes' default event handlers run for the nodes on the event's path, beginning with the target, unless the event was canceled or handled; the specification has only activation behaviour.
- Strings are UTF-8 (`DOMString`), so lengths and offsets into them count UTF-8 code units where the specification counts UTF-16 code units: a node's length, `CharacterData`'s `length` and offsets, and boundary point offsets into character data.
- Elements' user agent shadow roots (form controls, `<details>` and so on) are shadow roots whose mode is a third value, `UserAgent`: they're treated as closed (closed-shadow-hidden, and not returned by `shadowRoot`).
- Find a slot gives the children of a `<details>` element its user agent shadow root's slots by HTML's rendering rules: the first `<summary>` child goes to the summary slot and the other children to the content slot (`HTMLDetailsElement::SlotForHostChild`).
- Assign slottables clears the assigned slot of a slottable the slot no longer finds, so a slottable that's removed from its host or renamed away from its slot isn't assigned any more, as in browsers. The specification only ever sets the assigned slot, so it would keep pointing at the old slot (and the event path would still go through it).
- Inserting or moving a slottable into the host of a manual shadow root assigns a slot for it, as browsers do and as `imperative-slot-api.html` in the web platform tests expects, so a node given to `assign()` before it was the host's child is assigned once it is. The specification assigns a slot on insertion and move only in named shadow roots.
- `Document::Body()` is the first `<body>` child of the html element, for code that wants a body element. The specification's body element, which can also be a `<frameset>`, and `document.body` are `BodyOrFrameset()` and `SetBodyOrFrameset()`.
- The `[SameObject]` collections of tables (`rows`, `tBodies`, a section's `rows`, a row's `cells`), forms (`elements`) and selects (`options`, `selectedOptions`), and a labelable element's `labels`, are held weakly by their element, since a collection holds its element: they're the same object while something holds them, and a new one after.
- Markup is given as strings: there are no Trusted Types, so `innerHTML`, `outerHTML`, `insertAdjacentHTML()`, the Unsafe HTML insertion methods and `parseHTMLUnsafe()` take no `TrustedHTML` (nor `TrustedHTMLParserOptions`), and the Unsafe methods have no `runScripts` option, as there's no scripting. `Document::ParseHTMLUnsafe` and `Document::ParseHTML` take the document whose settings the new one gets, which the specification finds from the current global object.
- Attribute values are strings too: without Trusted Types, set an attribute, set an existing attribute value (`Attr`'s `value`, `nodeValue` and `textContent`), `setAttribute()` and `setAttributeNS()` skip getting a trusted type compliant attribute value and use the value as it is.
- `insertAdjacentElement()`, `insertAdjacentText()` and `insertAdjacentHTML()` take an `InsertAdjacentWhere` rather than a string, so matching the string ASCII case-insensitively is left to the caller; insert adjacent throws a `SyntaxError` for a value outside the enum.
- Creating a custom element synchronously calls its definition's factory with an element already created (custom, with the definition), rather than constructing a JavaScript constructor: there's no active custom element constructor map, and the factory's checks on the result (no attributes, children or parent, and the right document) stand in for create an element's. A failure gives an `HTMLUnknownElement` whose state is `failed`.
- The sanitizer's IDL unions of a string and a dictionary (`SanitizerElement`, `SanitizerAttribute`, `SanitizerPI` and the like) are the dictionaries, which a string converts to as canonicalizing it would (`u8"div"` is a `SanitizerElementNamespace` in the HTML namespace). The options' `(Sanitizer or SanitizerConfig or SanitizerPresets)` is `html::SanitizerInput`. A configuration's lists hold an element or attribute once, by name and namespace, so an element list's "contains" and "remove" ignore the attribute lists of a `SanitizerElementNamespaceWithAttributes`.
- `innerHTML`, `outerHTML` and `insertAdjacentHTML()` keep their own fragment parsing (`CreateFragmentForInnerOuterHTML`) rather than the specification's Legacy fragment parser mode; the HTML insertion methods (`setHTML()` and the rest, and their Unsafe counterparts) follow the specification's algorithms in `HTMLInsertionAlgorithms`.
- Setting `outerHTML` on an element whose parent is a `DocumentFragment` throws `NoModificationAllowedError`, as Chrome does; the specification parses the markup as if in a `<body>`. Setting it also merges the text nodes that end up next to each other where the element was, which the specification doesn't, as Chrome does.
- `innerText` (and `outerText`) is the text iterator's plain text of the element's contents, which approximates the rendered text collection steps rather than following them.
- A `ResizeObserver` or `IntersectionObserver` belongs to whoever created it, since there's no garbage collector, and disconnects when it's destroyed.
- Element reflection covers only `popovertarget` and `commandfor`, one element per attribute (`ExplicitlySetAttrElementsMap`): there are no reflected arrays of elements.
- `Document::CreateEvent` throws `NotSupportedError` for `devicemotionevent` and `deviceorientationevent`, whose interfaces don't exist, and `storageevent`, as there's no Web storage. A `customevent` is a `CustomEvent<RefPtr<SerializedScriptValue>>`, its detail standing for `any`.

## HTML parser

### From the specs

- Parse errors aren't reported: there's no console, so the parser recovers from each one as specified and says nothing (the `xmlns` attribute checks in create an element for a token are skipped).
- A `script` end tag in the "text" insertion mode only pops the script and restores the original insertion mode: with no scripting there's no microtask checkpoint, insertion point or script nesting level, the script isn't prepared, and there's never a pending parsing-blocking script, so the parser never pauses or spins the event loop. An SVG `script` end tag in foreign content likewise only pops it, as the SVG script isn't processed.

## HTML elements and forms

### From the specs

- A file input's files come from the host: a user's click (on the input or its button) or `showPicker()` asks it with a `FileChooser` (`ChromeClient::RunOpenPanel`, which cancels without a host picker), and its answer fires `input` and `change`, or `cancel`, as it comes rather than in a queued task. There's no file icon, as in Chrome, and no directory uploads (`webkitdirectory` is non-standard). Files can also be dropped, or set from C++ (`FileInputType::SetFiles`).
- A click opens a date, time or colour picker, or a text field's datalist suggestions, only while the window has transient activation, as in Chrome (`InputType::HasTransientActivation`), and doesn't consume it. So the host's input opens them, but `Click()` from C++ without user input doesn't; changes to a field's text or its datalist still update shown suggestions.
- A colour input's value is parsed and serialised as WebKit does it (`ColorInputType`): `currentcolor` parses as black, and sRGB components (alpha too) round to the nearest of 0 to 255, where the specification rounds towards +∞.
- The user agent style sheets (`html.css`, `quirks.css`) and the elements' presentational hints (`CollectPresentationalHintsForAttribute`) are close to Chrome's rather than the rendering section's rules. Where they differ: `align` is a hint on every HTML element, and `div`, `p` and table parts align their descendants too (the internal `-internal-left`, `-internal-center` and `-internal-right`); a table's `frame` gives thin solid borders rather than outset ones, and a table's `valign` is mapped; an `hr`'s `color` sets its border and background colours, and its `size` gives a height of the size less two, or no bottom border when it's 1 or not a number, even with `color` or `noshade` (where the specification gives half the size as border widths); a hidden table part isn't rendered rather than being collapsed; `ol`'s `start` and `reversed` and the lists' `list-item` counter resets are applied by list item numbering rather than as `counter-reset` hints; only `bdi`, `output`, `hr` and elements with a `dir` are `unicode-bidi: isolate`, not every block-level element; `sub` and `sup` keep the inherited `line-height`; `nobr wbr` isn't given back normal white space; and `mark` is `Mark` on `MarkText`.
- Selection offsets (`selectionStart`, `selectionEnd`, `setSelectionRange()`) are UTF-8 byte offsets, snapped to the start of a character, where the specification counts UTF-16 code units. `maxlength` and `minlength` count UTF-16 code units, as the specification does.
- There are no nested browsing contexts: a page has its main frame only, and `<iframe>`, `<object>` and `<embed>` reflect their attributes but load no document, so there's no sandboxing and no `sandbox` token list.
- `<frame>` is an `HTMLUnknownElement`, as there's no frame tree; the specification gives it `HTMLFrameElement`. `<frameset>` is an `HTMLFrameSetElement` (it can be the body element, and forwards window event handlers) but isn't laid out as frames.
- Microdata (`itemscope`, `itemprop`, `itemtype`, `itemid` and `itemref`) isn't processed, as in browsers: there's no way to read a document's items, and no conversion to vCard, iCalendar or JSON.
- The rules for parsing integers give an error outside `int64`'s range (`Numbers::ParseInteger`); the specification's have no limit.
- Dates and times later than 275760-09-13T00:00, the latest a script `Date` can be, aren't parsed, and nor are times with more than three digits after the seconds' full stop, since `DateComponents` keeps seconds to the millisecond; the specification parses both. Chrome does the same.
- The machine-readable values of `<time>` (its `datetime` attribute or contents) and of `<ins>` and `<del>`'s `datetime` attribute aren't parsed, as in browsers: nothing reads them. So there are no parsers for yearless dates, time-zone offsets, global dates and times, durations or dates with optional times.
- Setting an options collection's indexed property (`HTMLSelectElement::SetItem`) at index 100,000 or more does nothing, as in Chrome; the specification limits only the length setter.
- Moving a `<source>` into a `<picture>` (`moveBefore()`) selects its images' sources again, as inserting one does (`HTMLSourceElement::OnMove`), and so does moving an `<img>` into one; the specification's moving steps count only a move out of, or within, a picture.
- Updating an image's data follows `ImageLoader` rather than the specification's steps: there's one image request, not a current and a pending one; the memory cache (`CachedResourceLoader`) stands in for the list of available images; load and error events go through an event sender rather than element tasks; and a document without a living render tree loads no images, where the specification waits until it's fully active. The default source size, `100vw`, is truncated to whole pixels, and a `srcset` width or height descriptor over 2,147,483,647 is an error.
- An image map's areas with a shape are hit before its default areas, whatever their order (`HTMLMapElement::MapMouseEvent`), as in Chrome; the specification layers the areas in tree order, so a default area earlier in the map covers the later ones.
- A list item's list owner and ordinal value come from its enclosing list (`RenderListItem.cpp`'s `EnclosingList`: the closest `ol`, `ul` or element with style containment, else its parent) and the `list-item` counter, as in Chrome, rather than the specification's steps. A `menu` isn't an enclosing list, but its `list-item` counter reset numbers its items as the specification does, and an item under an element without a box is still numbered in its owner's sequence.
- There are no openers, as the host opens new windows: links and forms ignore `rel=noopener` and `rel=opener` (the tokens are still supported), and following a link skips getting its noopener. There are no referrers either (no `Referer` header or `document.referrer`), so `rel=noreferrer` changes nothing: form submission skips its referrer policy (`FrameLoader::SubmitForm`), and the one a link computes isn't read.
- `select()` on a text control focuses it, as browsers do; the specification only selects its text. Resetting a form from its own `reset` event does nothing (a reset is locked while it runs), as in Chrome; the specification would reset again.
- Activating interactive content inside a summary, such as a link or a text field, doesn't toggle its details (`HTMLSummaryElement::ActivationBehavior`), as in Chrome; the specification's activation behaviour toggles it whatever the event's target.
- A details element closes for its name group only once it's connected (`HTMLDetailsElement::OnPostConnection`): open details with the same name stay open in a disconnected tree, and when that tree is connected the earlier ones close rather than the later. The specification's insertion steps run in any tree, as each is inserted.
- A script's `src`, `text`, `innerText` and `textContent` setters take plain strings, as there are no Trusted Types, so they keep no script text. `HTMLScriptElement::Supports` gives the specification's answer although no script runs.
- A legacy colour value that starts with `#` keeps the 128 code points after it (`HTMLElement::ParseLegacyColorValue`), as Chrome does; the specification truncates to 128 code points before removing the `#`.
- In design mode the body takes the focus when its content is pressed, as in Chrome and WebKit, though the specification's editing host is the document element.
- A pragma (`<meta http-equiv>`) runs again when the element's `http-equiv` or `content` changes, as in Chrome; the specification runs it only when the element is inserted.
- An option the parser pops (`OpenElementStack::Pop`) updates its select's `<selectedcontent>` elements again, as its children are parsed after its post-connection steps; the specification updates them only when it's inserted, so a parsed selected option would leave them empty.
- `:optional` matches every input that isn't required, including those `required` doesn't apply to (such as range and colour inputs), as in Chrome; the specification matches only inputs to which `required` applies.
- A customizable select's picker icon has the `content` `\25BE` where the specification has `counter(fake-counter-name, disclosure-open)`, and, with no `field-sizing` or `calc-size()`, its minimum sizes are plain lengths. An option makes its internal shadow tree only once it has a label.
- A select's size of 0 is the default display size, as in Chrome; the specification makes it a display size of 0, which selects nothing.
- The topmost popover ancestor counts a popover's source when it's an open popover itself, as Chrome does (`Element::TopmostPopoverAncestor`); the specification looks only for popovers the source is a descendant of.
- `DataTransfer` departs from the specification: a data transfer made with `new DataTransfer()` isn't a drag's, so its `dropEffect` and `effectAllowed` can't be set; types are trimmed, and `text/plain;…`, `text/html;…` and `text/uri-list;…` are normalised to the bare type; `getData("url")` returns the whole URI list rather than its first URL; HTML and URI list data are sanitised for other applications; `setDragImage()` with an `img` in a document snapshots the element rather than taking its image; and `getAsFile()` returns the same `File` each time. Drags are processed by `DragController` and `EventHandler`, not by the specification's processing model.
- `<marquee>`'s `behavior`, `direction`, `scrollamount`, `scrolldelay` and `loop` aren't presentational hints, because the `-webkit-marquee-*` properties are dropped. A marquee moves as Chrome moves it: a Web Animation of its shadow tree's mover's `transform`, one per loop, from the marquee's content box and the contents' max-content width (or height).
- Form control state isn't saved or restored, as there's no session history: form-associated custom elements have no `formStateRestoreCallback`, and `ElementInternals.setFormValue()`'s state is kept but never used.
- The `width` and `height` of `<img>`, `<col>` and `<colgroup>` ignore a relative length (`5*`, `Dimensions::ParseHTMLMultiLength`); the specification's rules for parsing dimension values read it as 5 pixels.
- A `<style>` element unblocks rendering as soon as its sheet and critical subresources are processed (`HTMLStyleElement::NotifyLoadedSheetAndAllCriticalSubresources`), rather than in the task that fires its `load` event, so a sheet without imports never holds back a rendering update.
- The number-like input types' values (`ParseToDoubleForNumberType`, `ParseToDecimalForNumberType`) accept a full stop followed directly by an exponent (`1.e5`), which isn't a valid floating-point number.
- A `pattern` attribute is an ICU regular expression rather than an ECMAScript one with the `v` flag (`BaseTextInputType::PatternMismatch`). They mostly agree (classes, set operations, `\p{...}` properties, matching by code point, unknown escapes being errors), but exotic syntax can differ: ICU has possessive quantifiers and `(?i)`-style flags, and lacks the `v` flag's string literals in classes (`\q{...}`).

### From Chrome

- Text fields have no browser extras: no caps lock indicator on password fields (WebKit's on macOS), no generated strong passwords (Safari's), and no recent searches on search fields (WebKit's non-standard `results` attribute).
- `inputmode`, `enterkeyhint` and `autocapitalize` reflect but change nothing, as there's no on-screen keyboard.
- There are no interest invokers (`interestfor`, the `interest` and `loseinterest` events, `interest-delay` and `:interest-source`/`:interest-target`): Chrome ships them, but they're an Open UI proposal that isn't in the HTML standard yet.
- Access keys follow the specification where Chrome doesn't: `accessKeyLabel` exists (Chrome has no such attribute), `accesskey="x y"` assigns the first token that's one code point (Chrome needs the whole value to match the key), and a legend's access key runs its accesskey delegatee's command (Chrome clicks the legend).
- A textarea's `wrap` has no legacy `physical` and `on` aliases of `hard`, which Chrome keeps.
- A marquee follows the specification where Chrome doesn't: setting `loop` to 0 or a negative number other than -1 is ignored (Chrome throws an `IndexSizeError`), and a `scrolldelay` under 60 without `truespeed` is 60 (Chrome uses 85).
- Textareas aren't resizable by default (`Settings::TextAreasAreResizable` is false); Chrome's are.
- A search field's clear button shows whenever the field has a value, as the Adwaita theme does; Chrome shows it only while the field is focused or hovered.
- Booey breaks lines between adjacent atomic inlines (inline-block controls) with no space between them, where Chrome keeps them together (seen in the `ranges` golden).

## CSS parsing and CSSOM

### From the specs

- Style `Number` leaves in calc are multiplied by the used zoom like dimensions (`style::calc` `Evaluate(const Number &)`).
- A quoted generic family (`"serif"`) serialises unquoted (`SerialiseFontFamily` quotes only what isn't an identifier), so it reads back as the generic.
- Some shorthand serialisations differ from other browsers': `background-size: 10px` is `10px auto`, `animation: none` serialises as `auto`, `outline: none` as `currentcolor`, `offset: none` as `normal`.
- `attr()` lowercases its attribute name when parsed in an HTML document (`ConsumeAttr`), so it no longer matches a camel-case attribute on an SVG element there; the specification lowercases it only to compare with the attributes of HTML elements.
- An empty string keyframes name (`animation-name: ""`) becomes an empty `<custom-ident>` that serialises as nothing.
- `@supports` condition text is trimmed of ASCII whitespace and the vertical tab.
- `CSS.registerProperty()` (`DOMCSSNamespace::RegisterProperty`) throws a `SyntaxError` for a syntax other than `*` without an initial value, as Chrome and WebKit do and as `@property` requires, where the specification now gives it the guaranteed-invalid value. `CSS.supports()` and `CSS.registerProperty()` take the document that stands in for the global.
- `CSSStyleSheet` has no `replace()`: it's `replaceSync()` (`ReplaceSync`) with a promise, as both drop `@import` rules, and there are no promises.
- Whitespace tokens serialise compactly: in normal mode a run of spaces and tabs is one space and each newline another (`a \n b` becomes `a   b`); in custom property mode whitespace keeps its length and its newlines, with tabs as spaces. In custom property mode a dimension keeps its original text, escapes included (`10\70 x`).
- Media features read the frame's `MediaEnvironment`, which the host fills in. `hover`/`any-hover` and the pointer features default to a mouse (hover, fine pointer).
- There are no view transitions: `@view-transition` is an unknown at-rule, `view-transition-name`/`-class` are unknown properties, and `:active-view-transition(-type)` and the `::view-transition*` pseudo-elements are invalid selectors.
- There's no paged media: `@page` is an unknown at-rule (no `CSSPageRule`, page selectors, `size` descriptor or `page` property), and `overflow-block: paged` matches only the `print` media type.
- Non-standard properties are unknown properties: `buffered-rendering` (SVG Tiny 1.2's rendering hint, which Chrome parses), `-webkit-text-fill-color` (Compatibility; text is filled with `color`), `-webkit-font-smoothing`, `-webkit-user-modify`, `-webkit-user-drag`, `-webkit-tap-highlight-color`, `-webkit-background-size`, `-webkit-text-combine`, `-webkit-ruby-position`, `-webkit-line-align`, `-webkit-column-progression`, `-webkit-mask-position` and the `-apple-*` properties. Others are internal properties (`-internal-*`) that only the user agent's style sheets and C++ set: `-webkit-user-modify` and `-webkit-user-drag` (from `contenteditable` and `draggable`), `-webkit-locale` (from `lang`), `-webkit-text-security` (password fields, which mask with discs; the circle and square masks are gone), `-webkit-text-decorations-in-effect`, `-webkit-rtl-ordering`, `-webkit-line-box-contain` and the `border-spacing` and `mask-position` longhands (`-webkit-border-horizontal-spacing` and so on), so only the shorthands can be set.
- Legacy and prefixed aliases of standard properties are unknown properties (`word-wrap`, `grid-gap`, `font-stretch`, `-webkit-animation`, `-webkit-border-before`, `-epub-writing-mode` and the rest): use the standard names. So there are no webkit-cased CSSOM attributes, and `CSSFontFaceDescriptors` has no `fontStretch`.
- Prefixed keywords are unknown values: `-webkit-flex`, `-webkit-min-content` and the other sizing spellings (`-webkit-fill-available` is the standard `stretch`), `-webkit-sticky`, `-webkit-isolate`, `-webkit-grab`, `-webkit-text`, `-webkit-body`, `-webkit-pictograph`, `-webkit-xxx-large` and the rest, and `-apple-` keywords aren't read as `-webkit-` ones. So are WebKit's legacy `intrinsic` and `min-intrinsic` sizing keywords: use `fit-content` and `min-content`. What the user agent needs is internal: `<center>` and `align` use `-internal-left`, `-internal-center` and `-internal-right`, `<img align=middle>` uses `-internal-baseline-middle`, and `<rt>` uses `font-size: -internal-ruby-text`. The `-webkit-link`, `-webkit-activelink` and `-webkit-focus-ring-color` colour keywords are kept.
- Prefixed functions are unknown: `-webkit-linear-gradient()`, `-webkit-radial-gradient()`, their repeating forms and `-webkit-gradient()` (Compatibility), `-webkit-cross-fade()`, `-webkit-image-set()`, `-webkit-filter()` and `-webkit-calc()`; use the standard ones. The non-standard `-webkit-canvas()` and `-webkit-named-image()` images are gone.
- `initial-letter` takes one or two non-negative numbers (a size, and a sink that defaults to the size), as WebKit's `-webkit-initial-letter` did: the sink isn't required to be an integer, zero is allowed, and the `drop` and `raise` keywords aren't supported.
- `user-select`'s initial value is `text`, not `auto`.
- `hyphenate-limit-chars` and `hyphenate-limit-lines` animate discretely, not by computed value type.
- An unknown `::-webkit-*` pseudo-element is a parse error, where Selectors keeps it valid (matching nothing) for compatibility.
- `CSSOMString` can't be null, so nullable attributes are empty for null (`Href()`, `Title()`), and the rule model can't tell an empty `supports()` from none (so `@import "a" supports()` serialises without it); `CSSPropertyRule` omits an empty `syntax`.
- Style sheets from `<link>`, `@import` and processing instructions are always strict about their type: they must be `text/css` or have no type, where the specifications take a same-origin sheet of any type in quirks mode.

## Style (cascade and computed values)

### From the specs

- There's no history, so every link is unvisited: `:visited` never matches and there are no visited link styles (no per-link-state cascade, visited colours or `<body vlink>` colour). `:link` matches every link.

### From Chrome

- A computed `letter-spacing` of zero serialises as `normal`, as the computed value can't tell `normal` from `0`; Chrome serialises `letter-spacing: 0` as `0px`.

## Animations

### From the specs

- Keyframes are given from C++ as `KeyframesInput` (null, a list of `InputKeyframe`s or `InputPropertyIndexedKeyframes`), with properties named by their IDL attributes and values as CSS text.
- A transition that finishes naturally is removed from the running transitions before it's completed (`DocumentTimeline::TransitionDidComplete` only records transitions that are still running), so it isn't recorded in the completed transitions.

## Layout

### From the specs

- `display: -webkit-box` and `-webkit-inline-box` (Compatibility) are parse errors: there's no legacy box layout or `-webkit-line-clamp`; use flex and `line-clamp`.

### From Chrome

- Ordered list markers sit a little further from their text than Chrome's (the `decorations` golden); this is marker layout, not painting.

## Painting

### From the specs

- PDF isn't an image format: `Image::Create` makes SVG images from `image/svg+xml` and bitmaps from everything else, so a PDF is a broken image. A PDF renderer is too heavy for now; Chrome draws a PDF's first page.
- Dashes follow SVG and canvas (an odd-length pattern is repeated, each contour restarts the pattern, a zero-length dash with round or square caps draws a dot or square, and an empty, negative, non-finite or zero-sum pattern strokes solid), except that more than 100,000 dashes on a contour strokes it solid (`detail::MaxDashes`). `StrokeStyle::MiterLimit` defaults to 10 (canvas's default; SVG's is 4), and a width of 0 strokes a hairline one target pixel wide, as Skia does (canvas callers skip such strokes).
- Degenerate gradients (a line of zero length, or circles that coincide) are solid fills, as Skia's, where canvas paints nothing: the last colour when padded and the average colour when reflected or repeated. A padded radial gradient whose circles coincide but have a radius is the first colour inside the circle and the last outside.
- Compositing operators on draws are bounded, as Skia's: only what a draw covers changes, and antialiased edges blend over. A layer composited with an operator covers its whole clip.
- A pattern that doesn't repeat along an axis (canvas's `repeat-x`, `repeat-y` and `no-repeat`) extends its edge texels along that axis (the shader clamps the coordinate); the canvas specification and Skia leave it transparent there.
- `clipToImageBuffer` is drawn by Krystal.UI as a layer that the mask's texture masks (`DestinationIn`) at the `Restore` or `EndTransparencyLayer` that ends it, where Skia and Core Graphics clip to the image's alpha directly. A buffer with no texture clips everything out.
- Where a shape's edge lies on a clip's edge, both partly cover the edge pixels and the product is fainter (half times half is a quarter, not a half), the same seam Skia-based browsers have.

### From Chrome

- Chrome decodes images off the main thread as it rasterises, so a decode never stalls the page. Booey decodes a large still image (over 500KB decoded) on the host's work queue the first time an element paints it, even where it's visible, drawing nothing until it's decoded; WebKit does that only in a layer's first paint, which would stall the page for an image that loads later. Later decodes of a visible image, small images and animated images decode as they're drawn, unless `decoding=async` asks otherwise, and `decoding=sync` always does.
- Text is heavier in Chrome on Windows (DirectWrite hinting and contrast) than in Render2D (FreeType greyscale, hinted), with a mean difference of about 7 to 8, largest on glyph edges; glyph and box positions match. So the `text`, `decorations`, `selection` and the other text-heavy goldens are report-only against Chrome. Booey's synthetic bold is also drawn a little heavier than Chrome's (a 24th of the em against about a 25th at 14px).
- The theme is Adwaita's (`ControlFactory::Type` has only `Adwaita`), so the `controls`, `ranges`, `selects`, `textfields`, `dates` and picker goldens are report-only against Chrome (the date goldens also because Chrome on Windows uses the system's regional date format). Selection colours are Adwaita's (the accent colour at 30%), not Chrome's blue, and the focus ring is Adwaita's 2-unit ring where Chrome draws its own.
- Chrome fills selection only over text and line ends; Booey fills selection gaps in blocks.
- Chrome's wavy underline is taller and its skip-ink gaps a little wider.
- Chrome tiles `border-image-repeat: repeat` and `space` its own way, so the `border-images` golden is report-only. Stripes sliced at a third of a non-square box skew at tile joins in both.
- Chrome draws an SVG `<pattern>`'s content at its final transform; Booey draws the tile into an image buffer at the scale it's painted at and uses it as a brush, so a pattern with a rotating transform has differing tile edges.
- Chrome antialiases twice where its rounded border is a clip plus a fill (an edge pixel covered 0.38 shows 0.38 squared); Render2D's single coverage is closer to the true area, so golden comparisons differ there.
- A `box-shadow` is painted by clipping the shadow out of the box a pixel inside an opaque box's edge; without that, a light seam shows along every shadowed box's antialiased edge.
- Chrome's backdrop-filter blur mirrors its edges; Render2D duplicates the nearest edge texel (`EdgeMode2D::Duplicate`), so the `backdrop` golden differs a little along blurred glass edges.
- Chrome's `mask-border` golden was captured with the prefixed `-webkit-mask-box-image`, which always keeps the middle, so it's report-only.

## Compositing and scrolling

### From the specs

- `clientLeft` is the left border only (`RenderBox::ClientLeft`), even when a right-to-left scroller's vertical scrollbar is on the left; CSSOM View adds the scrollbar's width then.

### From Chrome

- The compositor has no damage tracking: each painted frame draws the whole layer tree into the host's target, as a game engine redraws its frames in full rather than presenting changed rects of a buffer it keeps. Painting is still damage-driven: the view repaints only damaged tiles, and `View::NeedsPaint()` lets a host skip frames where nothing changed.
- Scrollbars are classic, not overlay (`ScrollbarThemeAdwaita::UsesOverlayScrollbars` is false; FIXME): they always show and take layout space, 21 px for `auto` and `thin` and 0 for `none`, with a rounded thumb of at least 40 px and no buttons. Chrome's are 15 px, and overlays that fade on some platforms.
- Wheel scrolling is immediate outside a scroll snap container, where Chrome animates it. A notch is the system's lines (or a page) at 100/3 px per line, and Shift turns the vertical wheel horizontal. Keyboard scrolling moves 40 px for an arrow, a page for Page Up and Page Down, Space and Shift+Space, and to the ends for Home and End, animated unless `Settings::ScrollAnimatorEnabled` is off, and with the timing of a smooth scroll, which isn't measured against Chrome's keyboard.
- A smooth scroll takes Chrome's time, the square root of the distance in 60ths of a second up to 1.5 s, on a `cubic-bezier(0.4, 0, 0, 1)` fitted to Chrome's. A smooth scroll that's retargeted starts again from where it has got to, easing out, and its speed isn't carried over.
- A snap area larger than its snapport snaps to its aligned position when a scroll lands outside it, where Chrome goes to the closest position in which it covers the snapport: both are valid snap positions. A snap position is within 30% of the snapport for `proximity`, which is the user agent's to choose.
- Wheel events made by script don't scroll, as in Chrome.

## Text and fonts

### From the specs

- Password fields mask one character per code point (`RenderText::SecureText`), so a character outside the BMP gets one mask.
- Fonts come only from the host (`FontDatabase`, `Page::AddFontDirectory`) and the bundled faces: there's no lookup of the operating system's fonts and no fallback to them, so text renders the same on every machine. The system font keywords (`caption`, `icon`, `menu`, `message-box`, `small-caption`, `status-bar`) are the host's too (`SettingsValues::SystemFonts`): by default the standard family, at Chrome's sizes on Windows (12px for `menu`, `small-caption` and `status-bar`, 16px for the others), where Chrome uses the system's fonts (Segoe UI for those three).
- There's no CSS Font Loading API (`FontFace`, `FontFaceSet`, `document.fonts`), nor its string parsers for descriptors: it's for scripts, and a host adds fonts with `FontDatabase` (`AddFontFile`, `AddFontData`) or `Page::AddFontDirectory`. Web fonts come only from `@font-face` rules.

### From Chrome

- `hyphens: auto` never hyphenates, as there are no hyphenation dictionaries (`CanHyphenate` in `Text/Hyphenation.hpp` is false for every language), which css-text allows; Chrome hyphenates the languages it has downloaded dictionaries for.
- `text-transform: full-width` is supported, as in Firefox and Safari: a character's full-width form is what its `<narrow>` decomposition maps it to, or the character whose `<wide>` decomposition maps to it. Chrome doesn't parse the value.
- Hinting defaults to slight (`FontHinting::Slight`, vertical snapping only), and is off under a transform or synthetic oblique. Pens snap to a quarter pixel horizontally (four glyph variants, `GlyphAtlas::SubpixelPositions`) and to a whole pixel vertically. There's no LCD antialiasing. See *Painting* for how this compares with Chrome's text.
- Chrome hints Ahem's boxes differently, leaving seams between them, so the `webfonts` and `textfields` goldens are report-only.

## SVG

### From the specs

- The deprecated `zoomAndPan` attribute and `SVGZoomAndPan` interface aren't supported; `zoomAndPan()` in an `svgView(...)` fragment is parsed and ignored. So, as in Chrome and Firefox, a standalone SVG document isn't zoomed or panned by the user (WebKit pans it with an Alt-drag); the host can set `currentScale` and `currentTranslate`.
- `getComputedTextLength` sums the fragments' unstretched widths, so `lengthAdjust="spacingAndGlyphs"` doesn't change it.
- An `<svg>` element doesn't expose the window's event handlers, where SVG 2 includes `WindowEventHandlers` on it: as in Chrome and HTML's target algorithm, only the body and frameset elements reach the window, and `SetOnResize()` and the like on an `<svg>` are the element's own. Chrome forwards `onunload`, `onresize` and `onscroll` content attributes of an outermost `<svg>` to the window, but event handler content attributes are inert here. Set these on the window instead.
- A `mask-image: url()` that references a `<mask>` draws the mask's content as an image (`SVGResourceImage`), so it masks by alpha rather than by luminance.

### From Chrome

- Nothing inside a `<foreignObject>` is composited: its positioned boxes are painted into the foreign object's layer, in SVG coordinates, whatever their styles ask for (the layer lists that compositing goes by leave it out; `RenderLayer::PaintForeignObjectLayers`). So `backdrop-filter`, which only composited layers draw, isn't drawn there, and `transform` and `opacity` animations there run on the page rather than on layers. Chrome composites them; doing so would take the SVG's transforms and viewport clip into the graphics layer tree.
- Filters run on the GPU in float textures: a Gaussian blur is a true Gaussian rather than three box blurs, and nothing is quantised between effects, so an edge-detecting convolution of a smooth gradient stays smooth where Chrome amplifies the banding.

## MathML

### From the specs

- `<menclose>` is kept, though MathML Core doesn't have it.
- MathML elements take no `href`, so there are no MathML links.

### From Chrome

- Three behaviours differ from Chrome's: token heights are rounded, `<mtd>`'s padding differs, and `<mi>` has no italic correction. So the `mathml` golden is report-only against Chrome, for these and for DirectWrite's glyphs.
- Chrome draws no `<menclose>`; the golden uses a stretched brace instead.
- `autofocus` works on MathML elements, as the specification says (`MathMLElement::OnInsert`). Chrome only runs it for HTML and SVG elements, although `focus()` works on a MathML element with a `tabindex`.

## Canvas

### From the specs

- `toDataURL` encodes the pixels read back from the buffer (PNG, JPEG and WebP, with libpng, libjpeg-turbo and libwebp). A quality outside 0 to 1, or none, encodes at the encoders' best (100); Chrome's default JPEG quality is 0.92.
- A filter needs the canvas's computed style (`SetFilterStringWithoutUpdatingStyle`).
- While the current transform can't be inverted, the transform methods, the path methods and drawing do nothing, as in Chrome; the specification multiplies the matrix and adds the points regardless, so `scale(0, 1)` then `scale(1, 2)` leaves the vertical scale at 1 here, where the specification's is 2.
- `letterSpacing` and `wordSpacing` reject the units whose resolution the specification leaves undefined (`lh`, `rlh`, the viewport units and the container query units; whatwg/html#10893).
- `letterSpacing` and `wordSpacing` reject `calc()` lengths (`calc(1em + 2px)`), as Chrome and WebKit do: the specification parses a CSS `<length>`, but only a number with a unit is converted to pixels against the canvas's font (`ParseSpacing`).
- The text preparation algorithm replaces vertical tabs with spaces too; the specification replaces only ASCII whitespace.
- `arcTo()` with a negative radius throws before it ensures there's a subpath, so the path is unchanged, and `createPattern()` checks its repetition before the image's usability, so an unusable image with a bad repetition throws rather than returning null.
- There's no `toBlob()`: `toDataURL()` encodes the pixels synchronously.
- A canvas always gets a `RenderHTMLCanvas` and is always replaced, so its fallback content is never rendered; the specification renders the fallback content when scripting is disabled, which it always is here.

### From Chrome

- Stroked canvas text differs slightly, so the `canvas-text` golden is report-only (DirectWrite leaves seams between Ahem's boxes).

## Editing and selection

### From Chrome

- Turning `designMode` on puts a caret at the start of the document and focuses the document element, as the specification says, so typing can start straight away. Chrome leaves the selection and the focus where they are.
- In design mode, `isContentEditable` and `:read-write` say what can be edited: they're false inside `contenteditable=false` and in shadow trees, where Chrome reports true for both although typing there does nothing.
- There's no find in page: no text match markers or scrollbar tick marks for matches, no forward-only scrolling to them, and a match in a closed `<details>` or `hidden=until-found` content doesn't reveal it.
- There's no spelling or grammar checking: no text checker, autocorrection or document markers, so `::spelling-error` and `::grammar-error` highlights are never painted. Nor are the platform's text services built in: dictation, Writing Tools, text extraction, app highlights, image overlays (text recognised in images) and the kill ring and mark.
- There are no mail blockquotes: a `<blockquote type=cite>` is edited like any other block (no breaking out of quotes on Enter or paste, no `InsertNewlineInQuotedContent` command), and pasted `Apple-style-span` and `Apple-paste-as-quotation` markup gets no special treatment.
- Chrome's `getClientRects` reports an inline box's pieces separately around bidi-reordered content; Booey reports one rect where they're on one line.

## Input and events

### From the specs

- While the window doesn't have system focus, moving the focus fires no `focus`, `blur`, `focusin` or `focusout` events at elements, as in Chrome (`Element::DispatchFocusEvent`): the focused element gets `focus` and `focusin` when the window gets system focus. The focus update steps fire them regardless, as WebKit does.
- `window.focus()` asks the host (`ChromeClient::Focus`) and keeps the focused element, as browsers do, rather than running the specification's focusing steps on the viewport.
- The old focused element loses focus before the `blur` events fire, as in browsers (the specification designates the new area only before `focus`), so blur handlers see the viewport focused.
- `:focus-visible` uses a heuristic: `focus()` shows it unless the latest focus change was a click or `focusVisible` says otherwise, and text fields, selects and editable elements always show it.
- Transient user activation lasts 5 s (`LocalDOMWindow::TransientActivationDuration`) of the host's time, so it pauses with the engine.
- `pointerrawupdate` fires in every document, where it's only for secure contexts, as there are no origins. It fires just before the `pointermove` of each move, not sooner or more often, since the host's moves aren't delayed or coalesced into `pointermove`s.
- A pointer lock request needs the window's transient activation, so it may come from a timer up to five seconds after a press or key.
- Event timestamps aren't coarsened, where High Resolution Time coarsens them (Chrome to 100 µs): without script there are no timing attacks to guard against.

### From Chrome

- A cursor image (`cursor: url()`, up to 128 CSS pixels each way, as in Chrome) is shown wherever the mouse is. Chrome falls back to the next cursor when a large one would reach out of the page's visible content, so as not to cover its own UI; a C++ application has no browser UI to protect, and a game's cursor shouldn't change near the window's edge.
- A range takes a touch that first moves along it (`SliderContainerElement::HandleTouchEvent`), as Chrome does, but a slider with any transform (or `rotate`, `scale`, `translate` or `offset-path`) takes moves in every direction, where Chrome does so only for a rotation.
- The `focus` and `blur` events at the window are composed `FocusEvent`s with a `view`, as the specification says. Chrome fires a plain `Event` that isn't composed.
- `beforeinput` and `input` have the window as their `view`, as UI Events says; Chrome's is null.
- After a scroll, the hovered element follows the content under a still mouse when a fake mouse move fires, 100 ms later (250 ms once handling a mouse move has taken over 10 ms), as in WebKit; Chrome updates hover in the next frames. Fake moves carry the modifier keys of the latest mouse or keyboard event and are skipped while the page is hidden, but a page whose view isn't focused still gets them, as it does real mouse moves.

## Loading, navigation and history

### From the specs

- Everything is decoded as UTF-8: documents (through `DocumentWriter`), style sheets and linked text, with BOMs removed, sequences split across chunks kept and invalid bytes replaced. Encoding sniffing and the prescan, a `<meta charset>` or `http-equiv="Content-Type"` met while parsing, the request's and response's encodings, `@charset`, `<link charset>` and an XML document's encoding declaration (recorded but not followed) are all ignored, and form submissions are UTF-8 whatever their `accept-charset`. A host with text in another encoding converts it to UTF-8 before handing it to Booey. So no document is visually ordered (the ISO-8859-8 encoding's visual Hebrew): text is always in logical order, and there's no internal property or bidi override for visual order.
- Booey doesn't navigate by itself: following a link and submitting a form hand the navigation to the host, which loads what it wants. A form's entries reach the host unencoded, with the form's encoding type: Booey runs none of the encoding algorithms, so a `GET` submission's URL is the action without the entries as its query, and a `mailto:` action isn't given a body.
- Dropping a URL on a page that doesn't take it loads nothing, where browsers load the dropped URL.
- The CSS `crossorigin()` URL modifier is parsed but sets no request mode or credentials, as there's no network. Only a link to a fragment of the document in its own browsing context navigates, within the document.
- There is no session history: no `History`, `Navigation` or `Location` (`window.location`, `document.location`), no back/forward cache (`pageshow` and `pagehide` are never persisted), and no `popstate`, `hashchange`, `beforeunload` or `BeforeUnloadEvent`. `<meta http-equiv=refresh>` is ignored.
- A response of a type that's neither HTML, XML nor text (one the specification would hand off to other software, or download) is loaded as an HTML document of that type.
- In quirks mode, a fragment finds an `a` element by its `name` ASCII case-insensitively (`Document::FindAnchor`).
- The window has only what a C++ application uses: no `navigator` (nor `clientInformation`, plugins or protocol handlers), `screen`, bars (`locationbar` and the others), `status`, `external`, `name`, `open()`, `close()`, `closed`, `stop()`, `opener`, `top`, `parent`, `frames`, `length`, `frameElement`, `window`, `self`, user prompts (`alert()`, `confirm()`, `prompt()`), `print()` (nor `beforeprint` and `afterprint`), window geometry (`moveTo()`, `resizeTo()`, `screenX`, `outerWidth` and the others), `postMessage()` (nor `MessageEvent`), `reportError()` (nor `ErrorEvent`), `atob()`/`btoa()`, `origin`, `isSecureContext`, `crossOriginIsolated`, `originAgentCluster` or `navigator.userActivation`. Nor are there `online`, `offline`, `languagechange`, `message`, `messageerror`, `pageswap`, `pagereveal`, `rejectionhandled`, `unhandledrejection` or the gamepad events, or their handlers. The user activation itself is kept.
- A `ResizeObserver` loop that leaves notifications undelivered reports no error, as there's no script error reporting.
- There's no Gamepad API: the host reads pads itself (Krystal.UI's `View::GamepadInput` drives focus navigation with them).
- A document's visibility state is its page's (hidden without one), so `visibilityState` changes as soon as the host shows or hides the page and `visibilitychange` follows in a task (`Document::VisibilityStateChanged`); the specification sets the state in that task. An unloaded document is hidden for good, and its `visibilitychange` fires straight away (`Document::SetVisibilityHiddenDueToDismissal`). There is no `VisibilityStateEntry`, as there's no performance timeline, and no screen orientation change steps, as there's no screen orientation.
- There's no `setTimeout()` or `setInterval()`: C++ code starts document timers (`Document::StartTimer` and `StartRepeatingTimer`), whose handle stops the timer when it's destroyed. They run as tasks on the timer task source and wait while the document is suspended, but have no IDs or nesting clamp, and a repeating timer fires at a fixed rate.
- Animation frame callbacks are given the frame's timestamp rounded to whole milliseconds.
- DOM timestamps are the host's time since the window's time origin, so they pause and scale with the engine.
- There's no `requestIdleCallback()`: the host runs its own idle work between frames.
- There are no text fragments: a URL's `:~:` is part of its fragment like any other text, so `#:~:text=` links don't find, highlight or scroll to text. `::target-text` styles the text the host marks in `Document::FragmentHighlightRegistry()`.
- `url::URL` has no encoding override, so queries are always UTF-8 percent-encoded. IDNA uses `uidna_nameToASCII_UTF8` from vcpkg's ICU (Unicode 17), so `tests/URL/WPT.cpp` expects the `IdnaTestV2.json` cases with U+32931 and U+32B9A to be valid where the data expects a V7 failure, because Unicode 17 assigned those code points.

### From Chrome

- A file's type comes from its extension through a built-in table of common types (`MIMETypeRegistry::MIMETypeForExtension`, after Chrome's own mappings), for `File.type` and for files Krystal.UI loads. Chrome also asks the operating system (the registry's content types on Windows) for extensions its table lacks; Booey gives those no type.
- Layout is scheduled as soon as the document has a body and the style sheets before it have loaded (`Document::ShouldScheduleLayout`). Browsers also hold scheduled layouts back until the page has enough text or image pixels to show (WebKit's visually non-empty state), so as not to paint a half-loaded page; the host pushes Booey its content, so that would only delay the first paint.
