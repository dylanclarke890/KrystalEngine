# Krystal.Booey roadmap

Decided 2026-10-23, after the graphics redesign (`docs/gfx/DESIGN.md`, G1 to G14) finished. Booey is to be the engine's UI for everything: menus and HUDs, in-world surfaces (pages on textures), editor tooling, and shipped tools such as launchers. The focus now is finishing Booey's HTML, CSS and DOM; an MVC layer (C++ controllers over the DOM, components as custom elements, routing within a document and between documents) comes afterwards, on top of what these slices build.

Each slice is planned here before it starts (a *B<n> plan* section: parts, decisions, what's out), ported with the per-file checklist in `CLAUDE.md`, recorded in `docs/booey/PORTING.md`, and summarised here when done (a *B<n> as built* section). Slices go in the order below; small items from `docs/booey/TODO.md` are picked up by the slices they touch. Sizes are upstream's, as a rough guide.

## What's needed

The user's answers (2026-10-23):
- **Routing:** both within a document (views swapped by C++) and between documents (each route its own page, with history).
- **Surfaces:** menus and HUDs, in-world surfaces, editor tooling, shipped tools.
- **Input:** mouse and keyboard, touch, gamepads, text entry through IMEs.
- **Content:** SVG, MathML, engine textures in pages, canvas 2D, video and audio.
- **Components:** custom elements defined in C++.
- **Sources:** local files and asset packs, and data the host fetches and pushes in; remote pages later.
- **Also:** accessibility, drag and drop, right-to-left and bidi text, resize and intersection observers, the Web Animations API, dialogs, popovers and `<details>`.
- **Not needed:** iframes and a frame tree, printing.

## Slices

| Slice | Content | Upstream size | Done when |
|---|---|---|---|
| **Tier 1: the document model** | | | |
| B1 | Custom elements: the registry, creation, upgrades, callback reactions (the `CUSTOM-ELEMENTS` TODOs in the DOM algorithms and the parser), defined from C++; event listeners that are easy to add and remove from C++. | about 1.3k lines, plus the TODOs | A C++ class registered as `<my-widget>` is created by the parser and `createElement()`, upgraded when defined late, and gets its lifecycle callbacks. |
| B2 | Hyperlinks: `<a>` and `<area>` activation, `HTMLHyperlinkElementUtils`, fragment navigation and `:target`, image maps; the host decides what a link does (as upstream's navigation policy does). | small | Clicking a link reaches the host, fragments scroll to their targets and match `:target`, and `<map>` areas activate. |
| B3 | Document navigation, one frame: a slimmed `FrameLoader` and `DocumentLoader`, the load and unload lifecycle, session history (back and forward), `<meta http-equiv>` (refresh, default language), document readiness. | about 14k lines (much less without frames, scripting and origins) | A page navigates to another through a link or the host, goes back and forward, and fires its load events. |
| B4 | Interactive elements: `<dialog>` (modal, the top layer, `method=dialog`), popovers, `<details>` and `<summary>`, the command API (finishing `<button>`), `:modal` and `:popover-open`. | about 2k lines | Dialogs and popovers open, close, take focus and paint in the top layer; `<details>` toggles; buttons invoke commands. |
| B5 | Observers: `ResizeObserver` and `IntersectionObserver` on the rendering update. | about 1.9k lines | Observers report size and visibility changes from the host's frame. |
| B6 | The Web Animations API: `Element.animate()` and `getAnimations()`, the `Animation`, `KeyframeEffect` and timeline interfaces used from C++ (the model already runs CSS animations, G9), scroll and view timelines (named timelines, `scroll()`, `view()`, animation ranges), and the frame rate options. | about 17k lines (`animation/`; most of the model is ported) | A C++ caller animates an element with keyframes, pauses, seeks and reverses it, and scroll-driven animations follow their scrollers. |
| **Tier 2: input** | | | |
| B7 | Pointer events (`PointerEvent`, pointer capture), then touch: touch events, touch scrolling, `touch-action`, gestures. | about 1.4k lines, plus touch | Mouse, pen and touch input reach pages as pointer events; touch scrolls. |
| B8 | Gamepads: spatial navigation (focus moved by direction, upstream's `SpatialNavigation`) and gamepad input from the host. | about 2.5k lines | Focus moves between controls with the D-pad or a stick, and they activate with a button. |
| B9 | IME composition: composition text with its underlines and caret in `Editor`, composition events, the host's text input (Windows' IME) in Krystal.UI. | part of `Editor` | Chinese, Japanese and Korean text can be typed into fields and editable content. |
| B10 | Drag and drop: `DragController`, `DataTransfer`, drag events, within a page first, then to and from the system. | about 3.3k lines | Elements drag and drop within a page with their events; text and files drag in from the system. |
| **Tier 3: content** | | | |
| B11 | SVG: the render tree (`rendering/svg/`), shapes and paths first, SVG as an image (icons in `<img>` and CSS), then gradients, patterns, `<use>`, clipping, masking, markers, text and filters; the element factory and the remaining SVG DOM. | about 27k lines of rendering, plus the remaining DOM (35k lines) | Inline SVG and SVG images paint as Chrome does, including the theme's search field icons. |
| B12 | MathML: the elements (`mathml/`, the element factory, the `MATHML-ELEMENTS` TODOs) and their renderers (`rendering/mathml/`: fractions, scripts, roots, operators, tables), with the OpenType MATH table. | about 11k lines | MathML formulas lay out and paint as Chrome does. |
| B13 | Engine textures in pages: an element (or a `<canvas>` context) that paints a GPU texture the host supplies by handle, composited, with damage when it changes. Krystal-specific. | new | A live engine view (a 3D preview) shows inside a page, laid out and clipped like an image. |
| B14 | Canvas 2D: `<canvas>` and `CanvasRenderingContext2D` (paths, fills, strokes, text, images, `ImageData`, gradients, patterns, transforms, compositing) drawn through Render2D, used from C++. No WebGL. | about 10k to 15k lines | Canvas drawing from C++ matches Chrome's for the common calls. |
| B15 | Video and audio: `HTMLMediaElement`, `<video>` and `<audio>`, a host media player (decoding outside Booey) handing frames to compositing, the default controls. | about 15k lines, plus the host player | A video plays in a page, with sound, controls and its events. |
| **Tier 4: international and accessible** | | | |
| B16 | Right-to-left and bidi: `dir=auto` and `<bdi>` (bidi data), caret movement and selection in bidi text, checking inline layout and editing against Chrome in Arabic and Hebrew. | moderate, spread out | Mixed-direction text lays out, selects and edits as Chrome does. |
| B17 | Accessibility: the accessibility object tree (`AXObjectCache` and its objects, roles, names), the controls' accessibility FIXMEs, and a Windows UI Automation bridge (new work: upstream's Windows side is nearly empty; a library such as AccessKit could provide it). | about 38k lines | A screen reader reads and moves through pages and their controls. |

Afterwards: the MVC layer, on top of B1 to B3.

## Later

- **Remote pages:** a network loader (HTTP, caching, cookies), with origins and CORS (out by choice so far, `docs/booey/TODO.md`). Until then, the host fetches remote data and pushes it into pages.
- **The inspector:** see `docs/booey/TODO.md`.

## Not planned

From upstream, these stay out: JavaScript bindings, the JIT and scripting in general (`TODO.md`'s *Not supported*), workers, the APIs in `Modules/` (WebRTC, IndexedDB, WebAuthn, payments and the like), WebGL and WebGPU, content extensions and plugins, iframes and a frame tree, printing and pagination, and the iOS and macOS code.

## B1 plan (2026-10-23)

Custom elements, defined from C++. Upstream's part is `dom/CustomElementRegistry`, `dom/CustomElementReactionQueue` (with its element queues, `CustomElementReactionStack` and the backup queue), `bindings/js/JSCustomElementInterface` (the definition: constructor, callbacks, observed attributes), `dom/ElementInternals`, `dom/CustomStateSet` and `html/FormAssociatedCustomElement`: about 2.1k lines. Booey's DOM follows the spec's algorithms, which already have the registries' storage (documents, shadow roots and elements each hold one, with the scoped and global distinction), the elements' custom element state, `:defined`, and a `TODO(impl): CUSTOM-ELEMENTS` or `TODO(CUSTOMELEMENTS)` wherever a reaction is enqueued or a definition is looked up.

**Decisions** (the user's, 2026-10-23):
1. **Behaviour objects.** A custom element stays an `HTMLElement`; its definition creates a `CustomElement` (a C++ class the host derives from), which the element holds. It gets the lifecycle callbacks as virtual methods (`ConnectedCallback`, `DisconnectedCallback`, `ConnectedMoveCallback`, `AdoptedCallback`, `AttributeChangedCallback`, and the form callbacks) and keeps the component's state. The definition creates it when the element is created and when an existing element is upgraded, so a late definition keeps the element's identity, as the spec requires. `CustomElementInterface` takes the place of `JSCustomElementInterface`: the name, the behaviour's factory, the observed attributes, and `disabledFeatures` (`internals`, `shadow`) and `formAssociated` as plain options rather than static JavaScript properties.
2. **No customized built-ins**, as upstream: definitions have no `extends`, and `is` doesn't look up definitions. Booey's `is` value stays (it's serialised and cloned).
3. **Internals in B1:** `attachInternals()`, `ElementInternals` (its form value, validity, labels and form; ARIA reflection waits for B17 with `CustomElementDefaultARIA`), form-associated custom elements (taking part in form submission, reset, `disabled` and `<fieldset>`, validation, with `formAssociatedCallback`, `formResetCallback`, `formDisabledCallback`; `formStateRestoreCallback` waits for form state restoration, a FIXME today), and `CustomStateSet` with `:state()`.
4. **Reactions run before a DOM method returns:** upstream's `CustomElementReactionStack` (a scope; upstream's generated bindings open one for each `[CEReactions]` member) goes in the C++ methods the IDL marks `[CEReactions]`, with reflected attributes covered once in `Reflection`. Reactions enqueued outside any scope (from the host's own algorithms, or the editor's commands) go to the backup element queue, processed at the next microtask checkpoint as upstream does.

**Parts:**
- **B1a, the registry and reactions.** `CustomElementRegistry` (`Define`, `Get`, `GetName`, `WhenDefined` with a callback in place of the promise, `Upgrade`, `Initialize`, scoped registries), `CustomElementInterface`, `CustomElement`, `CustomElementReactionQueue` and its element queues, `CustomElementReactionStack`, `CustomElementReactionDisallowedScope`; the global registry, created with a document that has a browsing context (a `Page`'s, as upstream's comes with the window); creation through the element factory (synchronous and as an upgrade candidate), `createElement()`, the parser (the tree builder's element queues and *will execute script*, which is true for a definition outside fragment parsing), cloning, `attachShadow()`'s disabled shadow, and every `CUSTOM-ELEMENTS` TODO in the mutation, attribute, adoption and parser algorithms (the style attribute's mutation scope too). Upgrades in shadow-including tree order when a definition comes late; failed upgrades (a factory that returns null) leave the element `failed`.
- **B1b, internals and states.** `ElementInternals`, `FormAssociatedCustomElement` (a `ValidatedFormListedElement`, as upstream), the form callbacks, `CustomStateSet`, `:state()` in the selector checker (its FIXME) with style invalidation when a state changes.
- **B1c, C++ ergonomics and the sample.** Event listeners from C++: `AddEventListener` overloads taking a `Func<void(Event &)>`, returning a handle that removes the listener when it's destroyed (Krystal-specific; the callback classes stay for the listeners that need identity). The editor gains a custom element (a form-associated rating control, with `:state()` styles and a late definition), and a page section showing it.

**Bugs to fix on the way:** the tree builder reads an `is` attribute's name rather than its value; `CustomElementFactory.hpp` forward-declares `Document` in a namespace `boo::dom::dom`.

**Out:** customized built-ins; JavaScript; `formStateRestoreCallback` and the `restore` reason (with form state restoration); `CustomElementDefaultARIA` and the internals' ARIA (B17); the `extends-li` quirk. How a page's definitions carry over to the next document when it navigates is B3's (a host hook when a document is created).

**Tests:** under `tests/HTML/CustomElement/` and `tests/DOM/`: defining, names and errors; creation by `createElement()`, the parser (including fragments, whose elements upgrade when inserted) and cloning; upgrades on definition and on insertion, in tree order, and failures; each callback's order and arguments, observed attributes only, `[CEReactions]` timing (a callback has run when the method returns) and the backup queue; adoption between documents; scoped registries; `attachShadow()` with shadow disabled; internals (form values in submission, reset, `disabled` through a fieldset, validity, labels); `:state()` matching and invalidation; listener handles.

### B1a as built (2026-10-23)

- **Booey** (the ledger entry is *Custom elements* in `docs/booey/PORTING.md`): `CustomElementRegistry` (`Define`, `Get`, `GetName`, `WhenDefined`, `Upgrade`, `Initialize`, scoped registries), `CustomElementInterface` (the definition: a `CustomElementFactory` and `CustomElementOptions`), `CustomElement` (the behaviour, with the callbacks as virtual methods), the reaction queues and stacks, `HTMLMaybeFormAssociatedCustomElement` (every element with a valid custom element name). Elements are created synchronously by the parser (outside fragment parsing) and `createElement()`, and as upgrade candidates when cloned or created before their definition; definitions upgrade the document's elements in tree order, and insertion upgrades the rest. Every `CUSTOM-ELEMENTS` TODO is done: connected, disconnected, moved (`moveBefore()`), adopted and attribute changes (the inline style's too), disabled shadow roots. A document gets its global registry when it becomes a frame's.
- **Timing** (decision 4): `CEReactionsScope` in the `[CEReactions]` members upstream marks as needed (about 100 methods, and `Define`, `Upgrade` and `Initialize`); host code that Booey calls (event listeners, callbacks) runs with the stack suspended, so its calls get their own; editing commands scope their changes; the rest goes to the backup element queue, at the next microtask checkpoint.
- **Bugs fixed on the way:** the parser's `is` value, `createElement()`'s null registry, the parser's null registry, `NodeFlags::IsUnknownElement` never set.
- **Tests:** 7 cases (`tests/HTML/CustomElement/CustomElementRegistry.cpp`). No editor sample yet: B1c adds one, with the C++ listener handles.
- **Out (FIXMEs):** reporting failures (no console); the scopes of the `[CEReactions]` members Booey doesn't implement yet.

### B1b as built (2026-10-23)

- **Booey** (the ledger entry is *Element internals and custom states* in `docs/booey/PORTING.md`): `HTMLElement::AttachInternals` and `ElementInternals` (the shadow root, the form owner, `SetFormValue` with a string or entries, `SetValidity` with `ValidityStateFlags`, a message and an anchor, `WillValidate`, `Validity`, `ValidationMessage`, `CheckValidity`, `ReportValidity`, `Labels`, `States`); `FormAssociatedCustomElement`, the listed, validated half of an `HTMLMaybeFormAssociatedCustomElement` whose definition is form-associated: it joins its form and fieldsets, submits its value, takes part in validation (`:valid`, `:invalid`, `:disabled`), resets and is labelable; the `FormAssociatedCallback`, `FormResetCallback` and `FormDisabledCallback` reactions; `CustomStateSet` and `:state()`, with style invalidation.
- **Tests:** 4 cases (`tests/HTML/ElementInternals.cpp`).
- **Out (FIXMEs):** `formStateRestoreCallback` (no form state restoration), `File` form values, the internals' ARIA members (B17).

### B1c plan (2026-10-23)

C++ ergonomics for pages and components, and the editor sample. Krystal-specific: upstream's listeners are script functions.

**Decisions** (the user's, 2026-10-23):
1. **Listener handles.** `EventTarget::AddEventListener(type, Func<void(Event &)>, options)` registers a lambda and returns an `EventListenerHandle`: move-only and `[[nodiscard]]`, it removes its listener when it's destroyed (or by `Remove()`), and `Release()` gives the listener up to the target, which keeps it for as long as it lives. A typed overload, `AddEventListener<MouseEvent>(type, Func<void(MouseEvent &)>, options)`, calls back only for events of that type (via the event classes' type cast traits). The handle holds weak pointers to the target and to the registered `EventListener` and removes that listener by identity, so removing one that's already gone (by `RemoveEventListener`, `once`, an aborted signal, or the target going away) does nothing, as the spec's *remove an event listener* does. Released listeners can still be removed together through `AddEventListenerOptions::Signal` (an `AbortController`), as the spec has it.
2. **View-level definitions.** `View::DefineCustomElement(name, factory, options)` keeps the definition and defines it in each document the view loads before parsing (and in the current document, which upgrades its elements), until B3's navigation gives the host a hook when a document is created.
3. **The sample:** a star rating in the editor's coffee order form: form-associated, with a shadow root of five marks and its own styles, rated by a click or the arrow keys, styled through `:state(rated)`, submitting its rating, invalid until rated, cleared by Reset and disabled with its fieldset. (Built as `<bean-rating>`, five coffee beans drawn with CSS: the bundled Liberation Sans has no star.)

**Parts:**
- **Booey:** `DOM/EventListenerHandle.hpp` (the handle and the function callback); the `EventTarget` overloads; `EventTargetAlgorithms::AddEventListener` reports whether it added the listener (a duplicate or an aborted signal adds none, and the handle is then empty); type cast traits for the event classes that have predicates but lack them (`WheelEvent`, `ClipboardEvent`).
- **Krystal.UI:** `View::DefineCustomElement`, applied in `LoadHTML` between creating the document and parsing it.
- **Editor:** `StarRating` (a `CustomElement` in `Krystal.Editor`), defined on the page view; the star rating in `pages/index.html` with its styles; the page's `CardPicker` and `KeyLogger` callback classes become lambdas kept by handles.

**Tests:** `tests/DOM/EventListenerHandle.cpp`: a lambda's calls, the typed overload's filtering, removal by the handle's destruction, `Remove()` and assignment, `Release()`, moves, removal by other means first (`RemoveEventListener` is unreachable for a lambda, so `once`, an aborted signal, a dropped target), a handle destroyed during its own dispatch, an already aborted signal giving an empty handle. Krystal.UI's `tests/View.cpp`: a view's definitions apply to the pages it loads (created by the parser) and upgrade the current page. No golden: Chrome can't run the C++ element.

**Out:** inferring the typed overload's event type from the lambda (the type is given explicitly); listener groups (handles can be kept in a list, or listeners share a signal).

### B1c as built (2026-10-23)

- **Booey** (the ledger entry is *Listener handles and the custom element sample* in `docs/booey/PORTING.md`): `EventListenerHandle` and the `EventTarget::AddEventListener` overloads for functions, plain and typed (`AddEventListener<MouseEvent>`); `WheelEvent` and `ClipboardEvent` type traits. Fixed on the way: `AbortController::Abort` never aborted its signal, and a signal with two listeners overflowed the stack (`AbortAlgorithmCallback`'s constructor template), so signals now remove listeners as the spec has it.
- **Krystal.UI:** `View::DefineCustomElement`, given to each document the view loads before it's parsed.
- **Editor:** `<bean-rating name="rating" required>` in the coffee order form (`Krystal.Editor/include/Krystal.Editor/BeanRating.hpp`): five beans in a closed shadow root with its own styles (`:host(:focus-visible)`, `:host(:disabled)`), rated by a click on a bean, the arrow keys or a digit; its internals give the form its value (`rating=N`), its validity (required: the order form stays `:invalid` until it's rated) and the custom state `rated`, which the page's styles use (`bean-rating:state(rated) ~ .rated`); reset clears it. The page's card picker and key logger are lambdas kept by handles.
- **Tests:** 3 cases (`tests/DOM/EventListenerHandle.cpp`); Krystal.UI's `tests/View.cpp` defines and drives the editor page's rating (Krystal.UI.Tests includes `Krystal.Editor/include` for it).
- **Out:** inferring the typed overload's event type from the lambda; listener groups.

B1 is done: custom elements are defined from C++, created by the parser and `createElement()`, upgraded when defined late, get their lifecycle and form callbacks, have internals and custom states, and take part in forms.
