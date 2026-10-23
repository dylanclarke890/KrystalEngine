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
