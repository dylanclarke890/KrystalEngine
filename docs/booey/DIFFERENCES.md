# Differences

Where Krystal.Booey differs from the specs, from WebKit (whose structure it mirrors; see `OVERVIEW.md`) and from Chrome (the reference the goldens compare against), by area, with the reason when it's known. Behaviour that's simply missing is in `KNOWN-ISSUES.md`, and naming and translation conventions are in `OVERVIEW.md`.

Record a difference here when you make one, and remove it when Booey comes back in line.

## DOM

### From the specs

- There are no parallel queues or threads: steps the specification runs in parallel run on the event loop's thread, waiting on timers or the host's loader strategy where they wait, or synchronously where they don't (such as `toBlob()`'s serialisation), and queue their tasks back as the specification's would.
- With no garbage collector, `AbortSignal`'s garbage collection rule is kept with references: a source holds a dependent signal strongly while that signal has `abort` listeners or abort algorithms, and weakly otherwise, and an aborted signal lets go of its sources and dependents. `AbortSignal.timeout()`'s task keeps its signal alive whether or not it has listeners.
- Callbacks are C++ `Func`s rather than callback interfaces, so there's no `this` value and no exceptions to report: `ResizeObserver` and `IntersectionObserver` take the entries and the observer. Promises are replaced the same way: `CustomElementRegistry::WhenDefined` takes a callback and `Element::RequestPointerLock` returns its result through one. Animation `ready`/`finished` are `AnimationPromise`s and Navigation API results are `NavigationPromise`s, settled once, with their callbacks run as microtasks.
- `CustomEvent`'s `detail` is typed by the host rather than `any`: `CustomEvent<T>` holds a `Maybe<T>`, empty for null.
- JavaScript values (`any`) are `SerializedScriptValue`, a reference-counted base class the host derives from. There is no structured serialization, and so no `structuredClone()`.
- `IntersectionObserver` is notified directly in the rendering update, as upstream, where the specification queues a task.
- `DOMContentLoaded` fires synchronously at the end of parsing (`FinishedParsing`), as upstream, where the specification queues a task. `load` fires from a later task (`CheckCompleted`, then `ImplicitClose`), and `hashchange` is queued on the window.
- Event handler content attributes (`onclick="..."`) are scripts and stay inert: setting one doesn't replace a C++ handler as the specification's would. Handlers are set from C++ with `On<Name>()`/`SetOn<Name>()`. They're `Func<void(dom::Event &)>` and cancel with `PreventDefault()`, so `onerror`'s arguments and `onbeforeunload`'s return value don't exist.
- A listener's global, whose current event `event` is, is the current target's window (`EventDispatcher::RelevantWindow`), since listeners are host code without realms. Other targets, such as signals and media query lists, set no current event (FIXME).
- Nodes are equal when their node types match, as upstream, where the specification asks for the same interfaces: an `HTMLDocument` equals an `XMLDocument` with equal children.
- Documents have no origin and no base URL override, so cloning a document copies neither: the copy's base URL comes from its own URL and `<base>` element. `new Document()` skips setting its origin, and getting the base URL is the document base URL.
- Adopt sets the node document and registry of every node in the subtree first, then enqueues the `adoptedCallback` reactions, then runs the adopting steps, where the specification does all three for each node in turn; so a node's adopting steps see the whole subtree moved, as upstream's (which moves the tree before notifying it).
- There are no global objects, so `DocumentFragment`, `Text` and `Comment` are constructed with a document, as is a `ProcessingInstruction` (`ProcessingInstruction::Create`). `new Range()` is `Document::CreateRange`.
- There's no XPath (`XPathEvaluator`, `XPathExpression`, `XPathResult`, `Document.evaluate()` and `createExpression()`) and no XSLT (`XSLTProcessor`).
- A `NodeFilter` is a C++ subclass overriding `AcceptNode` rather than a callback interface or function, and returns a `FilterResult`, so it can't return a value other than `FILTER_ACCEPT`, `FILTER_REJECT` and `FILTER_SKIP`.
- Traversing a `NodeIterator` from inside its own filter throws an `InvalidStateError` before changing its candidate reference (`IteratorAlgorithms::Traverse`). The specification's traverse sets the candidate reference first, and when the filter throws, the inner traversal sets it to null, leaving the outer traversal with no candidate to continue from.
- Cloning a `ProcessingInstruction` updates the copy's attributes from its data, so its attribute map matches its data; the specification's clone sets only the target and data, leaving the copy's attribute map empty. `Document()` takes no arguments.
- The fallback base URL is always the document's URL: documents have no about base URL, which an iframe's `srcdoc` document or a document navigated to `about:blank` takes from its parent or initiator (skipped in `Document::FallbackBaseURL`).
- There's no Content Security Policy: every `<base>` is allowed, and inline `<style>` sheets and `style` attributes are parsed unchecked (`SPEC-VIOLATION(CSP)`).
- A `<base>` whose URL fails to parse falls back to the fallback base URL, as the specification says; upstream sets the document's base URL to null. `data:` and `javascript:` bases are always refused (upstream refuses them only with the `ShouldRestrictBaseURLSchemes` setting).
- `HTMLTemplateElement`'s content is a `DocumentFragment` in the element's own document; there's no inert template contents owner document. So a shadow root's adopted sheets are always cleared when it moves to another document (`ShadowRoot::OnAdopt`), where upstream keeps them for hosts in a template document.
- The agent's mutation observer flag, pending observers and signal slots are process-wide (`MutationObserverAlgorithms`; FIXME: agents).
- Customized built-in elements aren't supported, as upstream: `is` is ignored, so an element created with an `is` value and no matching autonomous definition is uncustomized where the specification makes it `undefined` (an upgrade candidate).
- A document's `querySelector()` and `querySelectorAll()` match `:scope` as `:root`, its document element, as browsers do. Scope-match gives the document itself as the scoping root, which Selectors would make a featureless virtual root that matches nothing.
- After dispatch, nodes' default event handlers run, as upstream, for the nodes on the event's path, beginning with the target, unless the event was canceled or handled; the specification has only activation behaviour.
- Strings are UTF-8 (`DOMString`), so lengths and offsets into them count UTF-8 code units where the specification counts UTF-16 code units: a node's length, `CharacterData`'s `length` and offsets, and boundary point offsets into character data.
- Elements' user agent shadow roots (form controls, `<details>` and so on) are shadow roots whose mode is a third value, `UserAgent`, as upstream: they're treated as closed (closed-shadow-hidden, and not returned by `shadowRoot`).
- Find a slot gives the children of a `<details>` element its user agent shadow root's slots by HTML's rendering rules, as upstream: the first `<summary>` child goes to the summary slot and the other children to the content slot (`HTMLDetailsElement::SlotForHostChild`).
- Assign slottables clears the assigned slot of a slottable the slot no longer finds, so a slottable that's removed from its host or renamed away from its slot isn't assigned any more, as in browsers. The specification only ever sets the assigned slot, so it would keep pointing at the old slot (and the event path would still go through it).
- Inserting or moving a slottable into the host of a manual shadow root assigns a slot for it, as browsers do and as `imperative-slot-api.html` in the web platform tests expects, so a node given to `assign()` before it was the host's child is assigned once it is. The specification assigns a slot on insertion and move only in named shadow roots.
- `Document::Body()` is the first `<body>` child of the html element, for code that wants a body element. The specification's body element, which can also be a `<frameset>`, and `document.body` are `BodyOrFrameset()` and `SetBodyOrFrameset()`.
- The `[SameObject]` collections of tables (`rows`, `tBodies`, a section's `rows`, a row's `cells`), forms (`elements`) and selects (`options`, `selectedOptions`), and a labelable element's `labels`, are held weakly by their element, as upstream caches collections, since a collection holds its element: they're the same object while something holds them, and a new one after.
- Markup is given as strings: there are no Trusted Types, so `innerHTML`, `outerHTML`, `insertAdjacentHTML()`, the Unsafe HTML insertion methods and `parseHTMLUnsafe()` take no `TrustedHTML` (nor `TrustedHTMLParserOptions`), and the Unsafe methods have no `runScripts` option, as there's no scripting. `Document::ParseHTMLUnsafe` and `Document::ParseHTML` take the document whose settings the new one gets, which the specification finds from the current global object.
- Attribute values are strings too: without Trusted Types, set an attribute, set an existing attribute value (`Attr`'s `value`, `nodeValue` and `textContent`), `setAttribute()` and `setAttributeNS()` skip getting a trusted type compliant attribute value and use the value as it is.
- `insertAdjacentElement()`, `insertAdjacentText()` and `insertAdjacentHTML()` take an `InsertAdjacentWhere` rather than a string, so matching the string ASCII case-insensitively is left to the caller; insert adjacent throws a `SyntaxError` for a value outside the enum.
- Creating a custom element synchronously calls its definition's factory with an element already created (custom, with the definition), rather than constructing a JavaScript constructor: there's no active custom element constructor map, and the factory's checks on the result (no attributes, children or parent, and the right document) stand in for create an element's. A failure gives an `HTMLUnknownElement` whose state is `failed`.
- The sanitizer's IDL unions of a string and a dictionary (`SanitizerElement`, `SanitizerAttribute`, `SanitizerPI` and the like) are the dictionaries, which a string converts to as canonicalizing it would (`u8"div"` is a `SanitizerElementNamespace` in the HTML namespace). The options' `(Sanitizer or SanitizerConfig or SanitizerPresets)` is `html::SanitizerInput`. A configuration's lists hold an element or attribute once, by name and namespace, so an element list's "contains" and "remove" ignore the attribute lists of a `SanitizerElementNamespaceWithAttributes`.
- `innerHTML`, `outerHTML` and `insertAdjacentHTML()` keep their own fragment parsing (`CreateFragmentForInnerOuterHTML`) rather than the specification's Legacy fragment parser mode; the HTML insertion methods (`setHTML()` and the rest, and their Unsafe counterparts) follow the specification's algorithms in `HTMLInsertionAlgorithms`.
- Setting `outerHTML` on an element whose parent is a `DocumentFragment` throws `NoModificationAllowedError`, as upstream and Chrome do; the specification parses the markup as if in a `<body>`. Setting it also merges the text nodes that end up next to each other where the element was, which the specification doesn't, as upstream and Chrome do.

### From WebKit

- Directionality (`dir`) is kept in three node flags (`SelfOrPrecedingNodesAffectDirAuto`, `UsesEffectiveTextDirection`, `EffectiveTextDirectionIsRTL`) rather than in rare data. A node with no directionality of its own reads the document element's current direction; upstream caches it in `documentElementTextDirection` when the document element changes. Children changes are reported once for everything inserted, so an insertion counts as replaced-elements-only (`onlyReplacedElementsInserted`) only when every inserted node is a replaced element. A slot updates its directionality in its insertion steps.
- A slot's auto directionality skips assigned elements that contained text auto directionality excludes (`<bdi>`, `<script>`, `<style>`, `<textarea>` and elements with a valid `dir`) and otherwise looks only at their text, as the specification says; upstream uses each assigned element's own auto directionality, so an assigned `<span dir=ltr>` or a text field's value decided it.
- `innerText` (and `outerText`) is the text iterator's plain text of the element's contents, as upstream, which approximates the rendered text collection steps rather than following them.
- An element's language counts `lang` in no namespace only on HTML and SVG elements, as the specification says (`xml:lang` counts on any element); upstream counts `lang` on every element, MathML included.
- `ProcessingInstruction` has the specification's attribute map and its methods (`getAttribute()`, `setAttribute()` and the rest), which upstream doesn't. An `xml-stylesheet` instruction reads its pseudo-attributes from the map, parsed by the rules for parsing pseudo-attributes (`xml::ParsePseudoAttributes`), where upstream parses its data as an element's attributes with libxml2.
- `NamedNodeMap`'s supported property names leave out duplicate qualified names, as the specification says; upstream keeps them.
- A `NodeIterator`'s reference and candidate reference are moved by the specification's adjust a node pointer when a node is removed (`IteratorAlgorithms::PreRemove`). Upstream's `updateForNodeRemoval` moves them differently in places: with the pointer after a removed first child, it moves to the node following the child rather than to the child's parent.
- `moveBefore()`, which upstream doesn't have, resets and then updates a moved element's directionality, as a removal plus an insertion would.
- `MutationObserver::Observe` implies `attributes` (or `characterData`) when `attributeOldValue` (or `characterDataOldValue`) is given at all, as the specification has it, so `{attributeOldValue: false}` observes attributes; upstream implies them only when the old value option is true, and throws a `TypeError` for it.
- A `ResizeObserver` or `IntersectionObserver` belongs to whoever created it, since there's no garbage collector, and disconnects when it's destroyed.
- `File` has no `webkitRelativePath` (directory uploads), file IDs or replacement paths.
- `Document::OriginIdentifierForPasteboard` always returns an opaque-origin identifier (`null:` plus a UUID, per document), so a page reads back custom pasteboard types it wrote but no other document does.
- The document title is computed when it's asked for, so nothing is notified as a `<title>` is inserted, removed or changed.
- Touch pointer ids start at 3 (`PenPointerID + 1`); upstream's GLib ones start at 2 (`mousePointerID + 1`). Touch objects have the standard's `RadiusX()`/`RadiusY()`/`RotationAngle()`/`Force()` in place of the `webkit*` names.
- Children are inserted one at a time, so the `AllChildrenRemoved`/`AllChildrenReplaced` change types are never produced: `replaceChildren` and `textContent` go through individual changes. Moves are a removal plus an insertion.
- There's no `SpaceSplitString`: the class invalidation parses the classes from the old and new values each time.
- A frame without a page gets a shown view, as nothing else would show it; upstream shows a new view only when its page is visible.
- Every connected node is considered to be in a rendered document, so documents without a frame or view are styled too; upstream's `resolveStyle` returns without a render view.
- `Document::NodeWillBeRemoved` runs before the removal and moves the hovered element and the active chain out of the removed subtree. Removing an element clears its pseudo-elements in the `ExtensibilityHooks::NodeRemoved` hook rather than in `removedFromAncestor`.
- Custom elements are C++ `CustomElement` subclasses with virtual callbacks, so every callback reaction is enqueued (upstream skips callbacks a definition lacks). `ConnectedMoveCallback` (for `moveBefore()`) is enqueued as the specification has it, and defaults to `DisconnectedCallback` then `ConnectedCallback`.
- Dialogs and popovers close through close watchers, as the specification has it: each registers a watcher with a cancel and a close action in its window's `CloseWatcherManager`. Upstream's `CloseWatcher` is only the page's API: Escape runs its watchers and then queues the modal dialog's cancel task and hides the topmost auto popover directly. `requestClose()` can always cancel (`RequestToClose(false)`, as the current specification has it); upstream's always needs history-action activation. Watchers are always on (no `CloseWatcherEnabled` setting) and aren't active DOM objects: the document destroys its window's watchers when it stops its active objects (`CloseWatcherManager::DestroyAll`).
- `ToggleEvent` has the specification's `source`, which upstream lacks, retargeted against the current target as `CommandEvent`'s is. `ToggleEventTask::Queue` keeps the latest toggle's source.
- Element reflection holds one element per attribute, keyed by local name (`ExplicitlySetAttrElementsMap`), for `popovertarget` and `commandfor` only. There are no arrays of elements.
- A node's rare data is a plain `UniquePtr<NodeRareData>` (an `ElementRareData` for an element, with a virtual destructor) rather than a compact pointer tuple with a type flag and custom deleter, and it's destroyed at the start of `Node::~Node`, before the node releases its document; upstream's goes with the node's members, after.
- Maps that upstream keeps weakly (such as `AnchorPositionedToAnchorMap`) are keyed by address, with a weak pointer in each entry to tell a destroyed key from a new one at the same address.
- `Document::CreateEvent` takes only the specification's names: upstream's extra `keyboardevents`, `mutationevent(s)`, `popstateevent` and `wheelevent` throw `NotSupportedError`, as do `devicemotionevent` and `deviceorientationevent`, whose interfaces aren't ported, and `storageevent`, as there's no Web storage. A `customevent` is a `CustomEvent<RefPtr<SerializedScriptValue>>`, its detail standing for `any`.
- `dataset` finds a `data-*` attribute only by its own property name, as the specification says. Upstream converts the property name to an attribute name and compares that, so `dataset["foo-bar"]` finds `data-foo-bar`; with a single attribute it compares the attribute's name after its first five characters without checking for `data-`.
- Markup set with `innerHTML`, `outerHTML` and `insertAdjacentHTML()` is always parsed with the full fragment parser: there's no fast path for simple markup, no cache of the last `innerHTML` set, and no reused fragment. Replacing the children queues one mutation record through the specification's "replace all", where upstream gathers the removals and the insertion with a `ChildListMutationScope`.

## HTML parser

### From the specs

- A `script` end tag in the "text" insertion mode only pops the script and restores the original insertion mode: with no scripting there's no microtask checkpoint, insertion point or script nesting level, the script isn't prepared, and there's never a pending parsing-blocking script, so the parser never pauses or spins the event loop. An SVG `script` end tag in foreign content likewise only pops it, as the SVG script isn't processed.

### From WebKit

- A `<link>` counts as created by the parser (and so is implicitly render-blocking) when it's inserted before it has finished parsing its children; upstream passes a flag to its constructor.
- `<?xml-stylesheet?>` loads only CSS sheets: there's no XSLT.
- Parsing an XML fragment (`innerHTML` in an XML document) follows the specification where upstream doesn't: a script's or style's markup is parsed like any other (upstream inserts it as text), and a namespace prefix declared on the context element hides an ancestor's declaration of it. Serializing an element's children declares the empty default namespace for those in no namespace when the element has a default namespace in scope.
- Serializing HTML follows the specification where upstream doesn't: HTML, SVG and MathML elements are written with their local names rather than their qualified names, an element's `is` value is written when it has no `is` attribute, a shadow root with manual slot assignment gets `shadowrootslotassignment="manual"`, and a processing instruction ends with `>` rather than `?>`.

## HTML elements and forms

### From the specs

- The user agent style sheets (`html.css`, `quirks.css`) and the elements' presentational hints (`CollectPresentationalHintsForAttribute`) are upstream's, which Chrome mostly shares, rather than the rendering section's rules. Where they differ: `align` is a hint on every HTML element, and `div`, `p` and table parts align their descendants too (`-webkit-left`, `-webkit-center`, `-webkit-right`); a table's `frame` gives thin solid borders rather than outset ones, and a table's `valign` is mapped; an `hr`'s `color` sets its border and background colours, and its `size` gives a height of the size less two, or no bottom border when it's 1 or not a number, even with `color` or `noshade` (where the specification gives half the size as border widths); a hidden table part isn't rendered rather than being collapsed; `ol`'s `start` and `reversed` and the lists' `list-item` counter resets are applied by list item numbering rather than as `counter-reset` hints; only `bdi`, `output`, `hr` and elements with a `dir` are `unicode-bidi: isolate`, not every block-level element; `sub` and `sup` keep the inherited `line-height`; `nobr wbr` isn't given back normal white space; and `mark` is `Mark` on `MarkText`.
- Selection offsets (`selectionStart`, `selectionEnd`, `setSelectionRange()`) are UTF-8 byte offsets, snapped to the start of a character, where the specification counts UTF-16 code units. `maxlength` and `minlength` count UTF-16 code units, as the specification does.
- `navigator.cookieEnabled` is false (there are no cookies without a network), `platform` is always `Win32` and `vendor` is empty (upstream's is `Apple Computer, Inc.`, Chrome's `Google Inc.`). `languages` has only the most preferred language and `hardwareConcurrency` is 4 or 8, as upstream (4 even with fewer logical processors, where the specification caps it at their number).
- `isSecureContext` checks the document's URL, since there are no origins: `https:`, `wss:`, `file:`, the local host, `about:` and a null URL are trustworthy, `data:` isn't. `crossOriginIsolated` and `originAgentCluster` are always false. Members restricted to secure contexts (`[SecureContext]`, such as `registerProtocolHandler()`) are available in every context.
- `<frame>` is an `HTMLUnknownElement`, as there's no frame tree; the specification gives it `HTMLFrameElement`. `<frameset>` is an `HTMLFrameSetElement` (it can be the body element, and forwards window event handlers) but isn't laid out as frames.
- Microdata (`itemscope`, `itemprop`, `itemtype`, `itemid` and `itemref`) isn't processed, as in browsers: there's no way to read a document's items, and no conversion to vCard, iCalendar or JSON.
- The rules for parsing integers give an error outside `int64`'s range (`Numbers::ParseInteger`); the specification's have no limit.
- Dates and times later than 275760-09-13T00:00, the latest a script `Date` can be, aren't parsed, and nor are times with more than three digits after the seconds' full stop, since `DateComponents` keeps seconds to the millisecond; the specification parses both. Upstream and Chrome do the same.
- The machine-readable values of `<time>` (its `datetime` attribute or contents) and of `<ins>` and `<del>`'s `datetime` attribute aren't parsed, as in browsers: nothing reads them. So there are no parsers for yearless dates, time-zone offsets, global dates and times, durations or dates with optional times.
- Setting an options collection's indexed property (`HTMLSelectElement::SetItem`) at index 100,000 or more does nothing, as in upstream and Chrome; the specification limits only the length setter.
- Moving a `<source>` into a `<picture>` (`moveBefore()`) selects its images' sources again, as inserting one does (`HTMLSourceElement::OnMove`), and so does moving an `<img>` into one; the specification's moving steps count only a move out of, or within, a picture.
- Updating an image's data follows upstream's `ImageLoader` rather than the specification's steps: there's one image request, not a current and a pending one; the memory cache (`CachedResourceLoader`) stands in for the list of available images; load and error events go through an event sender rather than element tasks; and a document without a living render tree loads no images, where the specification waits until it's fully active. The default source size, `100vw`, is truncated to whole pixels, and a `srcset` width or height descriptor over 2,147,483,647 is an error, as upstream.
- An image map's areas with a shape are hit before its default areas, whatever their order (`HTMLMapElement::MapMouseEvent`), as in upstream and Chrome; the specification layers the areas in tree order, so a default area earlier in the map covers the later ones.
- A list item's list owner and ordinal value come from its enclosing list (`RenderListItem.cpp`'s `EnclosingList`: the closest `ol`, `ul` or element with style containment, else its parent) and the `list-item` counter, as in upstream and Chrome, rather than the specification's steps. A `menu` isn't an enclosing list, but its `list-item` counter reset numbers its items as the specification does, and an item under an element without a box is still numbered in its owner's sequence.
- A link to a `javascript:` URL with a `_blank` target keeps its opener (`HyperlinkAlgorithms::GetNoopener`), as upstream; the specification gives it none. It opens no document, as there's no scripting.
- `select()` on a text control focuses it, as browsers do; the specification only selects its text. Resetting a form from its own `reset` event does nothing (a reset is locked while it runs), as in upstream and Chrome; the specification would reset again.
- Activating interactive content inside a summary, such as a link or a text field, doesn't toggle its details (`HTMLSummaryElement::ActivationBehavior`), as in upstream and Chrome; the specification's activation behaviour toggles it whatever the event's target.
- A details element closes for its name group only once it's connected (`HTMLDetailsElement::OnPostConnection`), as upstream: open details with the same name stay open in a disconnected tree, and when that tree is connected the earlier ones close rather than the later. The specification's insertion steps run in any tree, as each is inserted.
- A script's `src`, `text`, `innerText` and `textContent` setters take plain strings, as there are no Trusted Types, so they keep no script text. `HTMLScriptElement::Supports` gives the specification's answer although no script runs.
- A legacy colour value that starts with `#` keeps the 128 code points after it (`HTMLElement::ParseLegacyColorValue`), as upstream and Chrome do; the specification truncates to 128 code points before removing the `#`.

### From WebKit

- Enumerated attributes are read through their definitions: each attribute's keywords, missing, invalid and empty value defaults, and canonical keywords are its `EnumeratedAttributeTraits`, with a states enum of its own, and `EnumeratedAttribute::ResolveState` gives its state. IDL attributes limited to only known values reflect it with `ReflectEnumerated`. Upstream matches keywords wherever it reads an attribute, including `contentEditableType`, `isHiddenUntilFound`, `parseTextDirectionState`, `directionForFormData` and the `contenteditable`, `hidden`, `draggable` and `dir` presentational hints; the states they find are the same.
- A `ul` or `li` with `type=round` gets no list style: upstream maps it to a non-standard `round` style.
- `<meta name>` is only the standard `color-scheme` for the page's colour scheme; upstream also takes the legacy `supported-color-schemes`. A meta refresh to a `javascript:` URL is ignored, as the specification says, so a later refresh still counts; upstream schedules it.
- A pragma (`<meta http-equiv>`) runs again when the element's `http-equiv` or `content` changes, as upstream and Chrome; the specification runs it only when the element is inserted.
- An element's target is its `target` attribute even when it's empty; upstream's links fall back to the base element's target for an empty one. A form's target from the base element is checked for dangling markup (a tab or newline and a `<` make it `_blank`) like its own; upstream returned it as it is.
- `spellcheck` inherits from the parent element, as the specification says; upstream's `isSpellCheckingEnabled` continues from a shadow root to its host. `writingSuggestions`, `autocapitalize`, `autocorrect`, `inputMode` and `enterKeyHint` follow the specification; upstream ties some of them to platform features Booey doesn't have.
- Focus on click follows the specification (focusable areas that are click focusable, `FocusAlgorithms::ClickFocusableInclusiveAncestor`), not upstream's `isMouseFocusable`.
- Selects always use the current specification's parsing (customizable selects): list items are found among all of a select's descendants. Upstream does this only with the `HTMLEnhancedSelectParsingEnabled` setting, and has no `<selectedcontent>`. An option the parser pops (`HTMLElementStack::Pop`) updates its select's `<selectedcontent>` elements again, as its children are parsed after its post-connection steps; the specification updates them only when it's inserted, so a parsed selected option would leave them empty. An inserted selected option is selected from its own insertion steps (`OptionInserted`, `OptGroupInserted`) wherever it is in the select; upstream works it out from the select's children change.
- An option or optgroup whose nearest ancestor select is disabled is actually disabled, so it matches `:disabled`, as the specification says; upstream disables only the select. `:optional` matches every input that isn't required, including those `required` doesn't apply to (such as range and colour inputs), as in upstream and Chrome; the specification matches only inputs to which `required` applies.
- A select's selectedness setting algorithm selects only its first enabled option, as the specification says; upstream selects its first option when none is enabled, so a disabled first option inserted alone (as the parser does) stays selected. A size of 0 is the default display size, as in upstream and Chrome; the specification makes it a display size of 0, which selects nothing. Setting `selectedIndex` or `value`, or picking an option, makes it dirty, and adding or removing `multiple` with nothing selected runs the selectedness setting algorithm rather than resetting the select. The placeholder label option is the first option outside an optgroup even after an `<hr>`; upstream needs it to be the first list item. An option with an empty `label` attribute shows its text, as the specification's label and Chrome do; upstream shows nothing.
- A textarea's `wrap` is only `soft` or `hard` (and `off`, from the rendering section's style sheet); the legacy `physical` and `on` aliases of `hard`, which upstream and Chrome keep, are dropped.
- The date and time fields' user agent shadow root delegates focus, so focusing the input focuses its first field and sequential navigation moves between the fields. `<input type=week>` has editable fields (a week field and the layout `'Week' ww, yyyy`), as Chrome does; upstream's week inputs have none.
- A symbolic date field (month name, AM/PM) restarts type-ahead with the pressed key when the typed text matches none of its names (`TypeAhead::ResetSession`, as Chromium); upstream finds nothing until the keys pause.
- Asking to stay in `beforeunload` needs the window's sticky activation; upstream asks whether the user interacted with the page.
- Dialogs have the specification's `closedby` (upstream has none). `requestClose()` goes through the dialog's close watcher (upstream fires `cancel` and closes directly). Closing a dialog restores focus only if focus was in the dialog or the dialog was modal; upstream always restores it.
- Activating a summary is its activation behaviour, not upstream's `DOMActivate` default handler.
- Links follow the specification's activation behaviour (`HyperlinkAlgorithms::ActivationBehavior`): a click on an `<img ismap>` in an `<a>` adds `?x,y` even when the image has a `usemap`, `?0,0` for an untrusted click, after the URL is parsed and serialized (so after any fragment); upstream adds it only for a server-side map, before parsing. A link whose URL doesn't parse isn't followed. The `HTMLHyperlinkElementUtils` setters (`URLDecomposition`) do nothing for a null URL (no `href`, or one that doesn't parse); upstream's set an `href` from the document's URL.
- Popovers follow the specification's algorithms. `popover=hint` is supported, with its own stack and a hint stack parent (`Document::HintStackParent`), and the showing auto and hint lists are computed from the top layer (`Document::ShowingAutoPopoverList`, `ShowingHintPopoverList`); upstream keeps one auto popover list.
- Booey has the `overlay` property, which upstream doesn't. It's set only by the user agent: `auto` while an element is in the top layer and not pending removal (`style::Adjuster::Adjust`), so `transition: overlay allow-discrete` keeps an element rendered while it animates out.
- The topmost popover ancestor counts a popover's source when it's an open popover itself, as upstream and Chrome do (`Element::TopmostPopoverAncestor`); the specification looks only for popovers the source is a descendant of. The popover a click targets (`Document::TopmostClickedPopover`) follows the specification: a button's `commandfor` popover only with a popover command, and not with a form owner unless its type is `button`.
- Popover focus follows the specification's focus navigation scopes (`FocusAlgorithms::IsFocusNavigationScopeOwner`, `AssociatedFocusNavigationOwner`), not upstream's invoker links. `FocusAlgorithms::IsInert` walks the flat tree for `inert` and the active modal dialog; upstream reads the style's effective inertness.
- Commands (`command`, `commandfor`) are always on: there's no `CommandAttributesEnabled` setting.
- Lazy image loading is always on (no `LazyImageLoadingEnabled` setting), and whether scripting is enabled comes from the host (`MediaEnvironment::CanExecuteScripts`) rather than a script controller. Without scripting or a frame, images load eagerly, as upstream.
- Password fields don't use input methods (`ShouldUseInputMethod` is false), as on every upstream port but GTK and WPE.
- `DataTransfer` follows upstream where it departs from the specification: a data transfer made with `new DataTransfer()` isn't a drag's, so its `dropEffect` and `effectAllowed` can't be set; types are trimmed, and `text/plain;…`, `text/html;…` and `text/uri-list;…` are normalised to the bare type; `getData("url")` returns the whole URI list rather than its first URL; HTML and URI list data are sanitised for other applications; `setDragImage()` with an `img` in a document snapshots the element rather than taking its image; and `getAsFile()` returns the same `File` each time. Drags are processed by `DragController` and `EventHandler` as upstream's are, not by the specification's processing model. Unlike upstream, `items.clear()` does nothing unless the data can be written, and a removed item's `kind` is empty, as the specification and Chrome have it.
- Files are readable from pastes and file drags but not from drags of page content (`DataTransfer::AllowsFileAccess`, Cocoa's rule); upstream's other ports allow no file access.
- SVG `<style>` updates its sheet when `media` or `type` changes, as `HTMLStyleElement` does; upstream only stores the value.
- `<marquee>`'s `behavior`, `direction`, `scrollamount`, `scrolldelay` and `loop` aren't presentational hints, because the `-webkit-marquee-*` properties are dropped.
- The `-webkit-` user agent parts (`-webkit-inner-spin-button`, `-webkit-search-cancel-button`, the slider, progress, meter and date field parts and so on) parse only in user agent sheets (`user-agent-sheet-only`), so pages can't style them; upstream lets pages style them.
- The resizer grip is drawn by `RenderTheme::PaintPlatformResizer` rather than from a platform image. Textareas aren't resizable by default (`Settings::TextAreasAreResizable` is false, as upstream's WebCore default; WebKit's own default is true off iOS).
- `RenderThemeAdwaita::AnimationRepeatIntervalForProgressBar` is a fixed 16 ms; upstream uses the page's preferred rendering update interval (FIXME).
- The `width` and `height` of `<img>`, `<col>` and `<colgroup>` ignore a relative length (`5*`, `Dimensions::ParseHTMLMultiLength`), as upstream; the specification's rules for parsing dimension values read it as 5 pixels.
- `HTMLSelectElement::Add` follows the specification: it throws `NotFoundError` for an element to insert before that isn't in the select, does nothing to insert an option before itself, and inserts into that element's parent (an `<optgroup>`); upstream inserts into its parent only for an index. Setting an options collection's indexed property replaces the option in its parent, and growing the length appends the new options in one fragment; upstream removes and re-inserts, and appends them one at a time.
- `RadioNodeList`'s `value` follows the specification for radio buttons with an empty `value`: the getter returns a checked one's empty value and the setter can check one; upstream skips them.
- The `autocomplete` IDL attribute follows the current specification's processing model (`AutofillData::Create`): `on` and `off` alongside other tokens are no value, the field keeps the case it was written in, and the non-standard `device-eid` and `device-imei` fields are dropped; upstream follows an older version, with a maximum number of tokens per category. Its autofill hint set and scope are kept too, though nothing uses them.
- A GET submission is URL-encoded and a POST one uses the enctype by the submitter's method (its `formmethod`, if it has one); upstream goes by the form's method.
- The number-like input types' values (`ParseToDoubleForNumberType`, `ParseToDecimalForNumberType`) accept a full stop followed directly by an exponent (`1.e5`), which isn't a valid floating-point number, as upstream.

### From Chrome

- A search field's clear button shows whenever the field has a value, as upstream's Adwaita theme does; Chrome shows it only while the field is focused or hovered.
- Booey breaks lines between adjacent atomic inlines (inline-block controls) with no space between them, where Chrome keeps them together (seen in the `ranges` golden).

## CSS parsing and CSSOM

### From the specs

- Style `Number` leaves in calc are multiplied by the used zoom like dimensions (`style::calc` `Evaluate(const Number &)`), as upstream.
- A quoted generic family (`"serif"`) serialises unquoted (`SerialiseFontFamily` quotes only what isn't an identifier), as upstream, so it reads back as the generic.
- Shorthand serialisations follow upstream where browsers differ: `background-size: 10px` is `10px auto`, `animation: none` serialises as `auto`, `outline: none` as `currentcolor`, `offset: none` as `normal`.
- `attr()` lowercases its attribute name when parsed in an HTML document (`ConsumeAttr`), as upstream, so it no longer matches a camel-case attribute on an SVG element there; the specification lowercases it only to compare with the attributes of HTML elements.
- An empty string keyframes name (`animation-name: ""`) becomes an empty `<custom-ident>` that serialises as nothing, as upstream.
- `@supports` condition text is trimmed of ASCII whitespace and the vertical tab only; upstream's `deprecatedIsSpaceOrNewline` also trims non-ASCII spaces.
- Whitespace tokens serialise as upstream's do: in normal mode a run of spaces and tabs is one space and each newline another (`a \n b` becomes `a   b`); in custom property mode whitespace keeps its length and its newlines, with tabs as spaces. In custom property mode a dimension keeps its original text, escapes included (`10\70 x`); upstream serialises the value when it has no original prefix length (a unit with escapes, or a long number).
- Media features read the frame's `MediaEnvironment`, which the host fills in and which stands in for upstream's page, chrome client and screen. `hover`/`any-hover` and the pointer features default to a mouse (hover, fine pointer); upstream asks the chrome client, and says `none` without a page. Site quirks (`shouldSupportHoverMediaQueries` and the like) are dropped.

### From WebKit

- Legacy and WebKit-specific features are dropped on purpose. 42 of upstream's 70 `-webkit-` properties are gone: `-webkit-box-*`, `-webkit-border-radius`, `-webkit-border-image`, `-webkit-box-shadow`, `-webkit-mask`, `-webkit-mask-box-image`, `-webkit-text-stroke*`, `-webkit-line-clamp`, `-webkit-marquee-*`, `-webkit-perspective`, `-webkit-text-size-adjust`, `-webkit-text-zoom` and the rest, with their parsers and serialiser cases. The 28 non-standard ones the engine uses are kept (`-webkit-user-modify`, `-webkit-text-security`, `-webkit-background-size`, `-webkit-mask-position` and so on), and so are the 114 `-webkit-` and 9 `-epub-` aliases of standard properties (`-webkit-animation`, `-webkit-border-bottom-left-radius`, `-epub-writing-mode`). Also dropped: `@-webkit-keyframes`, `@-internal-base-appearance`, `DeprecatedCSSOM` (`getPropertyCSSValue`) and the epub-cased IDL names. Standard features are kept even when nothing supports them.
- 63 of upstream's 86 pseudo-classes are kept: the `-webkit-`, `-apple-` and `-internal-` ones, the scrollbar ones (`:horizontal`, `:decrement` and so on), `:window-inactive` and every legacy alias (`:matches`, `:-webkit-any-link`, `:-webkit-full-screen`) are dropped. 57 of upstream's 89 pseudo-elements are kept, of which 31 are `-webkit-` form control parts that parse only in user agent sheets (plus `-webkit-datetime-edit-week-field`, which upstream lacks); the scrollbar, resizer, media control, validation bubble and autofill button parts and the legacy aliases (`::-webkit-input-placeholder`, `::-webkit-file-upload-button`) are dropped. An unknown `::-webkit-*` pseudo-element is a parse error (upstream accepts it as `WebKitUnknown`, matching nothing).
- Non-standard media features (`-webkit-device-pixel-ratio` and its `min-`/`max-` forms, `-webkit-transform-2d`/`3d`, `-webkit-animation`, `-webkit-transition`, `-webkit-video-playable-inline`, `prefers-dark-interface`) are dropped and parse as `<general-enclosed>` (valid, evaluating to unknown).
- The tokenizer is Booey's own and has the specification's token set: there are no comment, match (`~=`, `|=` and so on) or column tokens, and one whitespace token type, plus `UnicodeRange`. Attribute selectors read the match operators as two delimiters, with no whitespace allowed between them. Tokens own their text.
- Upstream bugs not reproduced: `VisitSimpleSelectors` with `VisitOnlySubject::Yes`, `specificityForPage`, `serializeAnimationRange` with mismatched list lengths, `StyleSheetContents::wrapperInsertRule` for `@namespace`, `CSSViewTransitionRule::cssText`, `CSSCounterStyleRule::setName`/`setSystem`, the single type-selector fast path for `ns|*`, `linear()` accepting a trailing comma (`linear(0, 1,)` is rejected, as the grammar requires), `consumeStringOrURI` taking the value of any token inside `url()`, and in calc: `random()` evaluation and simplification element checks, negating a product negating every factor, `IsKnownZero` reporting non-zero values as zero, `anchor-size()` accepting a single non-value token as its fallback, `containsCurrentColor(RelativeColor)`, `toStyleColor(ColorLayers)` keeping layers unresolved, `vmax` serialisation priority and `CanonicalizeResolution` converting to dpi instead of dppx.
- `ComputeDeltaEOK` (Krystal.Core) uses OKLab lightness directly, as it's already in [0, 1]; upstream divides it by 100, making lightness differences 100 times too small. The unclamped ProPhotoRGB and Rec2020 transfer functions pass `abs(c)` to `pow`; upstream passes the signed value, so negative extended components become NaN.
- An+B integers are clamped to `int32` (upstream casts a `double`, undefined when out of range).
- `CSSOMString` can't be null: `Href()` and `Title()` are empty for none, and the rule model can't tell an empty `supports()` from none (so `@import "a" supports()` serialises without it); `CSSPropertyRule` omits an empty `syntax`. `SerialiseShorthandValue` returns `Maybe<CSSOMString>` (null means the shorthand can't represent the value).
- `random()`'s caching keys and `RuleSetBuilder`'s anonymous layer names (`anon_...`) use a `std::mt19937_64` rather than a cryptographic generator (FIXME for `random()`).
- `StyleSheetContentsCache`, the immutable declaration deduplication map and the `SelectorQuery` cache evict their first entry when full, where upstream evicts a random one. The `ValuePool` colour (512), font face (128) and font family (128) caches evict at random, as upstream.
- `RuleData`'s property allowlist has 3 bits, so `CueBackground` fits; upstream's has 2 bits and reads it back as `None`.

## Style (cascade and computed values)

### From WebKit

- MathML follows MathML Core as Chrome has it (upstream's `CoreMathMLEnabled` and `CSSMathDepthEnabled` on, in code): `mi { text-transform: math-auto; }` is in `mathml.css`, `mathvariant` is only `mathvariant="normal"` on `<mi>` (as `text-transform: none`), `mathsize` takes no named sizes, and the legacy presentational attributes (`fontsize`, `color`, `background`, `fontstyle`, `fontweight`, `fontfamily`) are dropped.
- `Settings` holds only the experimental and embedder flags and the font size values; standard features are always on, and values upstream reads from `Settings` that Booey doesn't model use upstream's defaults, hard-coded. `-apple-color-filter` is kept behind `Settings::ColorFilterEnabled`, off by default.
- A dark appearance is the main frame's `MediaEnvironment::UseDarkAppearance`: `Page::UseDarkAppearance` has no inspector override or per-load colour scheme preference, and `Document::UseDarkAppearance` has no elevated user interface level (macOS only).
- The properties dropped from the JSON (see *CSS parsing*) have no storage, no `Style::Difference` fields and no builder or rendering branches: legacy line clamp (`-webkit-line-clamp`), `-webkit-nbsp-mode`, the line grid, box reflections, `-webkit-text-size-adjust` and the rest. Site-specific quirks, text autosizing and Cocoa's intrinsic form control margins aren't ported either.
- There's no selector JIT; selectors are always interpreted by `SelectorChecker`.
- Documents without a frame or view are still styled, but their viewport units resolve to zero. Registering fonts (`FontDatabase`) invalidates every font cache, which restyles every document that has a font selector.
- There's no `UserInterfaceDirectionPolicy` setting: the side of a vertical scrollbar always follows the content's style (`shouldPlaceVerticalScrollbarOnLeft`).
- The shared shadow tree resolvers are capped at 256, and the first entry is evicted when the cap is passed, where upstream evicts a random one.

### From Chrome

- A computed `letter-spacing` of zero serialises as `normal`, as upstream, which can't tell `normal` from `0`; Chrome serialises `letter-spacing: 0` as `0px`.

## Animations

### From the specs

- Keyframes are given from C++ as `KeyframesInput` (null, a list of `InputKeyframe`s or `InputPropertyIndexedKeyframes`), with properties named by their IDL attributes and values as CSS text.
- A transition that finishes naturally is removed from the running transitions before it's completed (`DocumentTimeline::TransitionDidComplete` only records transitions that are still running), so it isn't recorded in the completed transitions, as upstream.

### From WebKit

- A replaced animation's `remove` event is scheduled at its document timeline's current time, converted to origin-relative time, as the specification has it; upstream converts the monotonic clock's time, which isn't on the timeline's clock.
- SMIL frames are always 1/60 s apart; upstream throttles them in low power mode and under thermal mitigation.
- `<svg>` begins its time container when it's inserted after the load event has finished (`LoadEventFinished`); there's no `processingLoadEvent` check.
- `SVGAnimateMotionElement` rebuilds its motion path on every children change, where upstream rebuilds it only on removals.
- `d` animates through `SVGAnimatedPathSegList`, and `SVGPathSegList` holds only the path byte stream: there are no `SVGPathSeg` items (SVG 2 removed them).
- Anonymous keyframes identifiers count up from 1; upstream starts from a random number so that accelerated animation names can't collide with authors'.

## Layout

### From WebKit

- Layout settings are fixed at upstream's defaults, and the paths behind the other value aren't ported. Continuations aren't built (`blocksInInlineLayoutEnabled` is on upstream), so a block inside an inline stays the inline's child, and `RenderTreeBuilderContinuation`, `splitFlow` and `splitInlines` don't exist. Text shaping across inline boxes and the flex and grid formatting contexts are always on, and subpixel inline layout is always off.
- `CanUseForLineLayout` is true for every block container but SVG text: the legacy line layout is kept only for SVG text, behind `Settings::UseIFCForSVGText` (off, as upstream). `RenderBlockFlow` computes preferred widths with the inline formatting context when `CanUseForPreferredWidthComputation` allows it, and with the legacy walker otherwise.
- The table formatting context (`layout/formattingContexts/table/`) isn't ported, since it only runs from `LayoutContext` on a whole layout tree, which the render tree never reaches; tables use the `RenderTable*` classes with auto and fixed table layout. Nothing builds a full formatting context tree, and there's no `LayoutContext`; the integration builds layout boxes with `BoxTreeUpdater`.
- `display: -webkit-box` is kept (it's in the Compat standard), but its `-webkit-box-*` properties are dropped, so it's always horizontal and stretching. There's no line grid, no legacy line clamp and no reflection layers.
- Inline layout works on UTF-8 offsets where upstream steps by UTF-16 code units. Text is trimmed, hyphenated and truncated by code points, and `TextUtil::BreakWord`'s estimates (fixed pitch character count, average width) are per byte, so they hit less often on non-ASCII text; the bisection still finds the position.
- `TextBreakingPositionCache` has no timers: it's evicted when it passes its hard cap rather than shortly after the soft threshold, and eviction removes entries in map order rather than at random.
- `LayoutUnit` has `operator=(int32)`; upstream assigns integers through `float`, losing precision above 2^24.

### From Chrome

- Ordered list markers sit a little further from their text than Chrome's (the `decorations` golden); this is marker layout, not painting.

## Painting

### From the specs

- Dashes follow SVG and canvas (an odd-length pattern is repeated, each contour restarts the pattern, a zero-length dash with round or square caps draws a dot or square, and an empty, negative, non-finite or zero-sum pattern strokes solid), except that more than 100,000 dashes on a contour strokes it solid (`detail::MaxDashes`). `StrokeStyle::MiterLimit` defaults to 10 (canvas's default; SVG's is 4), and a width of 0 strokes a hairline one target pixel wide, as Skia does (canvas callers skip such strokes).
- Degenerate gradients (a line of zero length, or circles that coincide) are solid fills, as Skia's, where canvas paints nothing: the last colour when padded and the average colour when reflected or repeated. A padded radial gradient whose circles coincide but have a radius is the first colour inside the circle and the last outside.
- Compositing operators on draws are bounded, as Skia's: only what a draw covers changes, and antialiased edges blend over. A layer composited with an operator covers its whole clip.
- A pattern that doesn't repeat along an axis (canvas's `repeat-x`, `repeat-y` and `no-repeat`) extends its edge texels along that axis (the shader clamps the coordinate); the canvas specification and Skia leave it transparent there.
- `clipToImageBuffer` is drawn by Krystal.UI as a layer that the mask's texture masks (`DestinationIn`) at the `Restore` or `EndTransparencyLayer` that ends it, where Skia and Core Graphics clip to the image's alpha directly. A buffer with no texture clips everything out.
- Where a shape's edge lies on a clip's edge, both partly cover the edge pixels and the product is fainter (half times half is a quarter, not a half), the same seam Skia-based browsers have.

### From WebKit

- A hidden (off-screen) view still collects damage; upstream drops repaints while a view is off screen (`shouldUpdate`) and relies on the platform's drawing area to repaint when it's shown, which Krystal.UI doesn't do.
- `GraphicsContext::DrawGlyphs` is pure virtual: the drawing backend draws glyphs itself. `GraphicsContextRender2D` fills them through `Renderer2D::DrawGlyphs` and strokes them as one path of their outlines.
- The base `GraphicsContext::DrawImageBuffer` and `DrawPattern(ImageBuffer &)` draw nothing (FIXME); display list recorders and Krystal.UI's context override them.
- Image buffers record into a display list unless the host makes an accelerated buffer with its own backend (`GraphicsClient`), which the drawing backend renders; there's no bitmap backend. Colour spaces, pixel formats, volatility, cloning and serialisation are FIXMEs.
- Synthetic bold keeps the regular advances, as Chrome does (`Font`'s `_syntheticBoldOffset` is 0); upstream widens each glyph by its synthetic bold offset.
- Gradients are dithered on 8-bit targets (an 8-by-8 ordered Bayer pattern under half a colour step); solid colours are never dithered, and there's no switch to turn dithering off.
- There's no layer-based SVG engine, so every renderer is pixel snapped.
- Paint-time pagination (`usePrintRect`, `page-break-*` truncation in `paintChild`) is dropped; upstream keeps it only for embedded AppKit views.
- A page's scale is always applied by the render view: `delegatesScaling` is dropped.
- Drag images are image buffers (display lists, `DragImageRef = RefPtr<ImageBuffer>`), not bitmaps: scaling one redraws it at the new size rather than resampling, and fading draws it at the fraction's alpha into a new buffer.
- `StringTruncator` works over UTF-8: its lengths and its 2048 limit are bytes.
- Core Graphics' crisp-edges stroke inflation is dropped. `DrawRect` is a fill then four side rects in the stroke colour, and spelling and grammar underlines are a triangle wave.
- While a selection is being made, the cursor follows the content beneath it, as Firefox's does: Booey comments out upstream's rule in `EventHandler::SelectCursor` that keeps an I-beam over everything.

### From Chrome

- Text is heavier in Chrome on Windows (DirectWrite hinting and contrast) than in Render2D (FreeType greyscale, hinted), with a mean difference of about 7 to 8, largest on glyph edges; glyph and box positions match. So the `text`, `decorations`, `selection` and the other text-heavy goldens are report-only against Chrome. Booey's synthetic bold is also drawn a little heavier than Chrome's (a 24th of the em against about a 25th at 14px).
- The theme is Adwaita's (`ControlFactory::Type` has only `Adwaita`), so the `controls`, `ranges`, `selects`, `textfields`, `dates` and picker goldens are report-only against Chrome (the date goldens also because Chrome on Windows uses the system's regional date format). Selection colours are Adwaita's (the accent colour at 30%), not Chrome's blue, and the focus ring is Adwaita's 2-unit ring where Chrome draws its own.
- Chrome fills selection only over text and line ends; Booey fills selection gaps in blocks, as upstream does.
- Chrome's wavy underline is taller and its skip-ink gaps a little wider.
- Chrome tiles `border-image-repeat: repeat` and `space` its own way, so the `border-images` golden is report-only. Stripes sliced at a third of a non-square box skew at tile joins in both.
- Dashed collapsed table borders start their dashes slightly differently from Chrome's (the `tables` golden); the dash phase is the same as for dashed borders.
- Chrome draws an SVG `<pattern>`'s content at its final transform; Booey draws the tile into an image buffer at the scale it's painted at and uses it as a brush, so a pattern with a rotating transform has differing tile edges.
- Chrome antialiases twice where its rounded border is a clip plus a fill (an edge pixel covered 0.38 shows 0.38 squared); Render2D's single coverage is closer to the true area, so golden comparisons differ there.
- A `box-shadow` is painted by clipping the shadow out of the box a pixel inside an opaque box's edge (upstream's painter rule); without that, a light seam shows along every shadowed box's antialiased edge.
- Chrome's backdrop-filter blur mirrors its edges; Render2D duplicates the nearest edge texel (`EdgeMode2D::Duplicate`), so the `backdrop` golden differs a little along blurred glass edges.
- Chrome's `mask-border` golden was captured with the prefixed `-webkit-mask-box-image`, which always keeps the middle, so it's report-only.

## Compositing and scrolling

### From the specs

- `clientLeft` is the left border only (`RenderBox::ClientLeft`), as upstream, even when a right-to-left scroller's vertical scrollbar is on the left; CSSOM View adds the scrollbar's width then.

### From WebKit

- The layer tree is adapted from TextureMapper (GTK and WPE), drawing with Render2D, not from Core Animation or the coordinated graphics thread. Tiling follows the coordinated port's coverage rather than TextureMapper's tiling of the whole layer.
- Compositing is mandatory rather than a setting: when the host gives the page a graphics layer factory, compositing is forced (`forceCompositingMode`), with all triggers allowed.
- Accelerated animations follow TextureMapper: only `transform` and `opacity` run on layers. They sample the event loop's current time (`EventLoop::Current().Now()`) rather than `MonotonicTime::now()`.
- Backdrop painting in Krystal.UI stops once it reaches the layer the backdrop belongs to (`ReachedBackdropTarget`), so later siblings above it don't show in its backdrop.

### From Chrome

- Scrollbars are classic, not overlay (`ScrollbarThemeAdwaita::UsesOverlayScrollbars` is false; FIXME): they always show and take layout space, 21 px for `auto` and `thin` and 0 for `none`, with a rounded thumb of at least 40 px and no buttons, as upstream's Adwaita theme. Chrome's are 15 px, and overlays that fade on some platforms.
- Wheel scrolling is immediate, with no animation. A notch is the system's lines (or a page) at 100/3 px per line, and Shift turns the vertical wheel horizontal. Keyboard scrolling moves 40 px for an arrow, a page for Page Up and Page Down, Space and Shift+Space, and to the ends for Home and End.
- Wheel events made by script don't scroll, as in Chrome (upstream's `scrollViaNonPlatformEvent` scrolls them).

## Text and fonts

### From the specs

- `-webkit-text-security` masks one character per code point (`RenderText::SecureText`), so a character outside the BMP gets one mask; upstream masks one per UTF-16 code unit.
- Text fragments: `FragmentDirectiveRangeFinder` runs only the specification's search over the nodes' text, without upstream's first attempt through `findPlainText` (FIXME), so matching ignores ASCII case only. Quotation marks fold to straight forms on both sides (`FoldQuoteMarks`, with source offsets because folding changes UTF-8 lengths).

### From WebKit

- All text is UTF-8, and every offset into rendered text is a UTF-8 byte offset (runs, glyph buffers, break iterators, `RenderText`, selection rects, DOM and editing ranges, `TextIterator` character counts). A soft hyphen is two bytes. `FontCascade::CharacterSelectionRectsForText` returns one rect per byte. Composition offsets are UTF-8 code units; the host converts an input method's UTF-16 offsets. `WidthCache` keys words of up to 16 bytes rather than 16 UTF-16 code units, so fewer non-Latin words are cached.
- Characters above U+FFFF are classified in `BreakablePositions` as upstream classifies their surrogates: they go to ICU.
- Where upstream splits a glyph's advance between UTF-16 code units, `ComplexTextController` splits it between characters, so a position inside a character counts the whole character.
- SVG text metrics, fragments and `SVGTextQuery` character numbers are UTF-8 byte offsets (a character number inside a character answers for the whole character). The `x`, `y`, `dx`, `dy` and `rotate` lists have one value per code point.
- `TextRun` holds a non-owning `utf8_stringview`, so the text must outlive the run.
- Fonts come from the host rather than the OS. The host registers files, directories or in-memory data with `FontDatabase` (`AddFontFile`, `AddFontDirectory`, `AddFontData`), and Booey indexes them with FreeType; there's no fontconfig, Core Text or DirectWrite. Families match by name, then by CSS font matching of weight, width and slope. Only the Arial/Helvetica, Times/Times New Roman and Courier/Courier New aliases apply.
- Characters that no family has fall back to a registered face that has the whole cluster, then to the bundled face. `FontCache::LastResortFallbackFont` is the bundled face, not fontconfig's `serif`.
- Bold is synthesised when it's asked for and the matched face is lighter than 600, and oblique when a slope is asked for and the face is upright. Synthetic bold doesn't change advances (see *Painting*); Render2D emboldens and shears the outline.
- Shaping uses HarfBuzz on FreeType faces, as upstream's Skia port does, and the simple path is used only when kerning and shaping are off and the variant caps are normal (`ShouldUseComplexTextControllerForSimpleText`). Scripts are converted to HarfBuzz scripts through their ISO 15924 short names (`uscript_getShortName`), because vcpkg's HarfBuzz is built without ICU.
- Glyph advances and bounds use unhinted font units scaled by the size, with metrics following cairo's unhinted path (`hhea` metrics, or OS/2 typographic metrics when `USE_TYPO_METRICS` is set).
- `-webkit-nbsp-mode` is dropped, so the non-breaking space mode is always normal and the breaker's and line builder's quirks for it are off.
- In right-to-left text, the caret and selection extension move logically by character and word, as Chrome does, rather than visually through runs of the other direction as upstream does. A double click also takes the whitespace after the word (see *Editing*). Bidi text drawn outside the line layout (list box items, alt text, file names, ellipses, drag labels, canvas text) goes through `GraphicsContext::DrawBidiText` in visual order.

### From Chrome

- Hinting defaults to slight (`FontHinting::Slight`, vertical snapping only), and is off under a transform or synthetic oblique. Pens snap to a quarter pixel horizontally (four glyph variants, `GlyphAtlas::SubpixelPositions`) and to a whole pixel vertically. There's no LCD antialiasing. See *Painting* for how this compares with Chrome's text.
- Chrome hints Ahem's boxes differently, leaving seams between them, so the `webfonts` and `textfields` goldens are report-only.

## Images and resources

### From WebKit

- The platform image is Booey's own decoded pixels (`PixelData`) with its size and premultiplication, not Skia's `SkImage`. The PNG, JPEG and WebP decoders are upstream's GTK/WPE ones over vcpkg's `libpng`, `libjpeg-turbo` and `libwebp`; the GIF, BMP and ICO decoders are upstream's own. A `PlatformImage` can also be an `ExternalImage` (a texture the host holds).
- Images have no HDR or colour space variants.
- `ResourceRequest`, `ResourceResponse` and `ResourceError` carry no HTTP. Loads complete whole, never partially, through the host's `LoaderStrategy`; there are no subresource loaders, redirects, cancellation, pings or preconnects.
- `DataURLDecoder` decodes only synchronously, and parses the media type simply (FIXME: `ParsedContentType`). `data:` URLs are decoded by Booey, never by the host.
- `url()` always serialises the specified URL: there are no replacement URLs and no `shouldUseResolvedURLInCSSText`.

## SVG

### From the specs

- `getComputedTextLength` sums the fragments' unstretched widths, so `lengthAdjust="spacingAndGlyphs"` doesn't change it, as upstream.
- A `mask-image: url()` that references a `<mask>` draws the mask's content as an image (`SVGResourceImage`), so it masks by alpha rather than by luminance, as upstream.

### From WebKit

- Only the legacy SVG renderers exist: the layer-based SVG engine (`RenderSVGModelObject` and the rest) isn't ported.
- There's no software filter path: the drawing backend applies filters on the GPU (`Page::PreferredFilterRenderingModes` is `{Accelerated, GraphicsContext}`), in float textures rather than 8-bit buffers. A Gaussian blur is a true Gaussian rather than three box blurs, and nothing is quantised between effects, so an edge-detecting convolution of a smooth gradient stays smooth where Chrome amplifies the banding.
- SVG 1.1 features that SVG 2 removed are unknown elements or absent: SVG fonts and the `<font>` family, `<altGlyph>`, `<glyphRef>`, `<tref>`, `<cursor>`, `<animateColor>`, `SVGPathSeg`, and `SVGPathSegList`'s functions (`pathSegList`, `getPathSegAtLength`, `createSVGPathSeg*`).
- `Path::StrokeContains` and `StrokeBoundingRect` ask the host's `PathStrokeStrategy`; without one, nothing is inside a stroke, and a stroke's bounds are the path's grown by half the width.
- `LegacyRenderSVGForeignObject::Paint` paints from the block's location, so opacity, blending and isolation clip correctly; upstream paints nothing for a foreign object with opacity away from the origin.
- The SVG number parser rejects an exponent too large for a float (`1e39`); upstream's `genericParseNumber` returns `false` from a function returning `std::optional<float>`, which parses it as 0.

## MathML

### From the specs

- `<menclose>` is kept, though MathML Core doesn't have it, since upstream renders it either way.
- MathML elements take no `href`, so there are no MathML links.

### From WebKit

- MathML follows MathML Core (see *Style*): `<mfenced>` (with `RenderMathMLFenced` and `RenderMathMLFencedOperator`) is dropped, and `<mglyph>`, `<malignmark>`, `<mstack>` and the rest of `mathtags.in`'s second group are unknown elements laid out as rows, as in Chrome (upstream makes them presentation elements). `<semantics>` and `<maction>` are rows that show their first child, by the Core sheet. `mathmlCoreExtras.css` and `mathmlFontSizeMath.css` replace the legacy font size sheet.
- `RenderMathMLToken` has no single-glyph `mathvariant` mapping; italics in `<mi>` come from `text-transform: math-auto`.
- The operator dictionary's infix `^` and `_` entries are found; upstream's table is mis-sorted there, so its infix forms get postfix accent properties.

### From Chrome

- Three upstream behaviours differ from Chrome's: token heights are rounded, `<mtd>`'s padding differs, and `<mi>` has no italic correction. So the `mathml` golden is report-only against Chrome, for these and for DirectWrite's glyphs.
- Chrome draws no `<menclose>`; the golden uses a stretched brace instead.

## Canvas

### From the specs

- The specification's `fontKerning`, `fontStretch`, `fontVariantCaps`, `textRendering` and `lang` exist; upstream lacks them.
- `toDataURL` and `toBlob` encode the pixels read back from the buffer (PNG, JPEG and WebP, with libpng, libjpeg-turbo and libwebp). A quality outside 0 to 1, or none, encodes at the encoders' best (100); Chrome's default JPEG quality is 0.92.
- A filter needs the canvas's computed style (`SetFilterStringWithoutUpdatingStyle`); `"null"` and `"undefined"` aren't special, where upstream leaves the filter unchanged for them.
- While the current transform can't be inverted, the transform methods, the path methods and drawing do nothing, as in upstream and Chrome; the specification multiplies the matrix and adds the points regardless, so `scale(0, 1)` then `scale(1, 2)` leaves the vertical scale at 1 here, where the specification's is 2.
- `letterSpacing` and `wordSpacing` reject the units whose resolution the specification leaves undefined (`lh`, `rlh`, the viewport units and the container query units; whatwg/html#10893), as upstream does.
- The text preparation algorithm replaces vertical tabs with spaces too, as upstream does; the specification replaces only ASCII whitespace.
- `arcTo()` with a negative radius throws before it ensures there's a subpath, so the path is unchanged, and `createPattern()` checks its repetition before the image's usability, so an unusable image with a bad repetition throws rather than returning null; both as upstream.
- `toBlob()` serialises the bitmap before it queues the callback's task, rather than in parallel.

### From WebKit

- `globalCompositeOperation` doesn't accept the non-standard `darker`, and `difference` is only the blend mode, as in Chrome.
- Upstream's non-standard aliases (`setStrokeColor`, `setFillColor`, `setLineWidth`, `webkitLineDash`, `setShadow`, `clearShadow`, `setPath`, `getPath`, the string setters for line caps and joins), the testing API, `beginLayer`/`endLayer`, `CanvasNoiseInjection` and quirks are dropped, and filters are always on (no `CanvasFiltersEnabled` setting).
- `putImageData` on a display list buffer is drawn as an image (premultiplied pixels with the copy operator, under the base transform with no clip, alpha or shadow), so it keeps its place among the recorded draws.
- A canvas always gets a `RenderHTMLCanvas` and is always replaced, so its fallback content is never rendered; upstream does this only when scripting is enabled.

### From Chrome

- Stroked canvas text differs slightly, so the `canvas-text` golden is report-only (DirectWrite leaves seams between Ahem's boxes).

## Editing and selection

### From WebKit

- Where two text boxes meet at the caret's offset, the caret belongs to the line-leftmost of them, whichever way they run (`Position::GetInlineBoxAndOffset`); upstream's adjustments by bidi level are removed.
- A double click selects the word and the whitespace after it under the Windows editing behaviour (`EditingBehavior::ShouldSelectTrailingWhitespaceOnDoubleClick`); upstream asks the editor client, whose default is off.
- Ctrl with a Shift key pressed and released on its own sets the direction of the text being edited (right Shift right-to-left, left Shift left-to-right), as Chrome on Windows does; a key pressed in between cancels it. This lives in Krystal.UI's `View` rather than in Booey.
- `textInput`'s default action (`Editor::HandleTextEvent`) runs after dispatch when the event isn't cancelled or handled, where upstream runs it from the node's default event handler.
- The pasteboard follows the GLib port's model: the system clipboard is read and written by type through the host's `PasteboardStrategy`, with a `SelectionData` for drags. Pasteboard contexts, names other than `CLIPBOARD`, the primary selection and the async clipboard API's per-item strategy calls are absent. Pages' own clipboard types travel in a private clipboard format that carries the writer's origin, and HTML that a page writes, or reads from another origin, is sanitised in a staging page.
- An input method's result rides on the key press, as on GTK: the `keydown` goes out as key code 229 (key `Unidentified`) with no default handling and no `keypress`, and the editor client applies the result afterwards.
- `EditorClient::SetInputMethodState` is also called with null when the focused element is removed (`Document`); upstream calls it only from `FocusController::setFocusedElement`, so a removal left the input method on.

### From Chrome

- Chrome's `getClientRects` reports an inline box's pieces separately around bidi-reordered content; Booey reports one rect where they're on one line.

## Input and events

### From the specs

- `window.focus()` asks the host (`ChromeClient::Focus`) and keeps the focused element, as browsers do, rather than running the specification's focusing steps on the viewport. `window.blur()` does nothing.
- The old focused element loses focus before the `blur` events fire, as in WebKit and other engines (the specification designates the new area only before `focus`), so blur handlers see the viewport focused.
- `:focus-visible` follows WebKit's heuristic: `focus()` shows it unless the latest focus change was a click or `focusVisible` says otherwise, and text fields, selects and editable elements always show it.
- Transient user activation lasts 5 s (`LocalDOMWindow::TransientActivationDuration`) of the host's time, so it pauses with the engine.
- A pointer lock request needs the window's transient activation, so it may come from a timer up to five seconds after a press or key; upstream's must come while the input is being handled.

### From WebKit

- Sequential focus navigation and click focus are written from the HTML specification (`FocusAlgorithms`, with `FocusForClick`), and `FocusController` isn't ported. From a starting point that isn't in the sequential order (a pressed node, an element with a negative `tabindex`), navigation takes the next area in shadow-including tree order.
- A pen arrives as mouse events (pointer type `pen`, id 2) with tilt and twist; its eraser is the "other" button and its barrel button the right button, and a pen in contact with no pressure presses at 0.5.
- Touch panning calls `EventHandler::HandleTouchScroll()` (the wheel's default scrolling without the DOM `wheel` event), where upstream hosts send wheel events. Gestures (pan, fling, tap, long press, pinch) are recognised in Krystal.UI's `View`; Booey has none.
- `Settings::AutoscrollForDragAndDropEnabled` is on by default (upstream's is on only on iOS).
- `Gamepad.timestamp` is milliseconds from the window's time origin, not the monotonic clock.
- Events are stamped with the host's time (`html::EventLoop::Current().Now()`), and so are the platform events that Krystal.UI and Booey make; upstream uses the real monotonic clock. `Event::TimeStampForBindings` takes the document rather than a script execution context, and its resolution isn't reduced (upstream rounds to 1 ms and Chrome to 100 µs): without script there are no timing attacks to guard against.
- Links' drag images are a label (the text in bold over the URL in a rounded rectangle, truncated with `StringTruncator`), where the GLib port has none; Krystal.UI draws every drag image translucent (`DragController::DragImageAlpha`) and runs the drag loop itself rather than handing it to a system loop.
- `ColorChooserClient::IsElementRTL` is Booey's: the host's select popup, date picker and colour picker overlays are mirrored for right-to-left controls, as Chrome's are.

## Loading, navigation and history

### From the specs

- `popstate` fires as the frame goes to the entry (also for fragment navigations), as upstream does, not in one task with `hashchange` (FIXME).
- `Location`, `History` and `Navigation` use the specification's rule that a `file:` document's URL may be rewritten to another query but not another path; upstream compares ignoring only the fragment.
- `history.pushState()` and `replaceState()` throw `SecurityError` when called more than 100 times in 10 seconds, and `QuotaExceededError` past 64 MB of state, as upstream does. The specification's limit (allowed to perform a navigation or history update) returns without an exception. Script-initiated Navigation API navigations past upstream's rate limit reject their promises with `QuotaExceededError`.
- A response of a type that's neither HTML, XML nor text (one the specification would hand off to other software, or download) is loaded as an HTML document of that type.
- In quirks mode, a fragment finds an `a` element by its `name` ASCII case-insensitively (`Document::FindAnchor`), and `beforeunload` isn't fired at a document without a body, as upstream does.
- Session history is upstream's: `HistoryItem`s in the page's back/forward list, moved through by `FrameLoader`, `HistoryController` and `NavigationScheduler` tasks, rather than session history entries with steps applied by a traversable's session history traversal queue. Navigations, reloads and traversals follow upstream's order of events, which matches the specification's where tests check it.
- An intercepted `reload` navigation with no fragment, or one that matches nothing, restores the scroll position rather than scrolling to the beginning, and updates the Navigation API entries, as upstream and Chrome do.
- A `navigate` event's `userInitiated` is whether the window has transient activation, as there's no user involvement to pass, and a traversal's `cancelable` doesn't check for history-action activation. `hasUAVisualTransition` is always false.
- `postMessage()` only reaches the window itself, with the window as the incumbent; `reportError()` takes a message and an optional value, without a script location.
- A document's visibility state is its page's (hidden without one), so `visibilityState` changes as soon as the host shows or hides the page and `visibilitychange` follows in a task (`Document::VisibilityStateChanged`), as upstream; the specification sets the state in that task. There is no `VisibilityStateEntry`, as there's no performance timeline.
- `window.open()` navigates an existing browsing context, or asks the client's new-window policy and returns null for a new one (there are no other windows to return).
- `print()` fires `beforeprint` and `afterprint` around the host's print request, as the specification has it; upstream fires them from its print context. A document waits until after its load event to print, as the specification has it (upstream waits while its loader is loading), and the initial `about:blank` document, which never loads, prints at once.
- Timers follow upstream's clamping: a timeout is raised to 4 ms only once timers are nested 10 deep (5 for intervals), where the specification clamps beyond a nesting level of 5, and an interval repeats at a fixed rate rather than being set again after each run.
- Animation frame callbacks are given the frame's timestamp rounded to whole milliseconds, as upstream.
- DOM timestamps are the host's time since the window's time origin, so they pause and scale with the engine; idle deadlines are real time. Idle periods start when the host asks (`View::Update` gives the rest of its frame's budget, at most 50 ms), not when an event loop idles.
- `Document::SetURL` takes the fragment directive from every URL it's given (empty when there is none), so a text fragment link within the page works and a later plain fragment isn't sent to the old text; upstream keeps the page's main frame fragment, or the first directive.

### From WebKit

- Navigation goes through a slim `FrameLoader` that asks the host: `LocalFrameLoaderClient` answers policy decisions as they're asked (`PolicyAction`) rather than through callbacks, so the client is asked at once whether to go to a history item. There's no subresource loader.
- `FrameLoader::IsLoadingFromCachedPage` (not upstream) tells the client a commit is a restore, and `LoadEvent::FromBackForwardCache` passes it to the host.
- Documents given host markup (`SubstituteData`) get history items (upstream leaves substitute data out of `urlForHistory`); the item keeps the markup, which going back and reloading load again.
- The back/forward cache expires entries after `CachedPage::ExpirationInterval` of the host's time, holds one frame and keeps no script or platform data.
- `DOMWindow` is merged into `LocalDOMWindow` (there are no remote windows), and a frame is the only one in its tree, so `top` and `parent` are the window itself.
- `MessageEvent` data is a `SerializedScriptValue`, a reference-counted base the host derives from; `source` is a window, and there are no ports.
- `url::URL::IsAboutSrcDoc` is false for an `about:srcdoc` URL with a query, as the specification's matches about:srcdoc; upstream ignores the query.
- `url::URL` (the WHATWG URL, UTF-8 only) has no `URLTextEncoding`, so queries are always UTF-8 percent-encoded. IDNA uses `uidna_nameToASCII_UTF8` from vcpkg's ICU (Unicode 17); `tests/URL/WPT.cpp` expects the `IdnaTestV2.json` cases with U+32931 and U+32B9A to be valid where the data expects a V7 failure, because Unicode 17 assigned those code points. The scheme length field is 24 bits (upstream's is 26).
- `URLDecomposition::Origin` serialises the origin from the URL (`file://` for files, `null` when opaque), as there are no security origins.
