# Krystal.Booey

Booey is the engine's web engine: HTML, CSS, the DOM, SVG, MathML and canvas 2D, for everything the engine shows as UI (menus and HUDs, pages on in-world surfaces, editor tooling, and shipped tools such as launchers). Krystal.UI hosts it and draws it through Render2D (see `Krystal.UI/docs/OVERVIEW.md` and `Krystal.Gfx/docs/OVERVIEW.md`).

The specifications are the reference for behaviour, and Chrome is the reference for rendering (the goldens are Chrome's). Where Booey differs from either, it's recorded in `DIFFERENCES.md`; known bugs and gaps are in `KNOWN-ISSUES.md`. How each area is put together is in `ARCHITECTURE.md`. Fine-grained open work is marked in the code with `// FIXME:` comments.

## Scope

**In:** the DOM, HTML (elements, forms, the parser, dialogs, popovers, `<details>`, commands, custom elements defined in C++), CSS (parsing, the cascade, every standard property, container queries, anchor positioning, the CSSOM and CSSOM View), layout (block, inline, flex, grid, table, positioning, floats, ruby), painting and compositing, scrolling, transitions, animations and the Web Animations API, editing and selection, mouse, keyboard, pointer, touch, pen and gamepad input, IME composition, drag and drop, links, navigation and session history with the back/forward cache, resize and intersection observers, SVG (the legacy render tree), MathML Core, canvas 2D, engine textures in pages, and right-to-left and bidi text.

**Later:** remote pages (a network loader with HTTP, caching, cookies, origins and CORS; until then the host fetches remote data and pushes it into pages); video and audio (`HTMLMediaElement` with a host media player, the default controls and text tracks); an inspector, as a Krystal.UI panel over the DOM, CSS and layer tree (its hooks were left out: the CSS parser's observer, the resolver's CSSOM wrappers, forced pseudo-classes and the instrumentation points); the rest of the Typed OM.

**Not planned:** JavaScript and scripting in general (bindings, the JavaScript-only parts of the specs such as `handleEvent`, IDL callbacks, HTML element constructors and scripting in the parser), realms and global objects other than the window, origins and `document.domain`, trusted types, Content Security Policy and `nonce`, policy-controlled features, workers and service workers, message channels and broadcast channels (`MessageChannel`, `MessagePort`, `BroadcastChannel`), Web storage (`localStorage`, `sessionStorage` and the `storage` event; the engine can offer its own), standalone web APIs other than gamepads (WebRTC, IndexedDB, WebAuthn, payments and the like), WebGL and WebGPU, content extensions and plug-ins, iframes and a frame tree, printing and pagination, and accessibility. With no global objects, `DocumentFragment` is constructed with a document. Where a missing feature changes a specification algorithm, the call site is marked `SPEC-VIOLATION` (global objects, trusted types, service workers, origins, Content Security Policy, browsing contexts and Web IDL), or, in algorithms reviewed against a checklist in `Krystal.Booey/docs/specs/`, the step is labelled `// 3. (Skipped: <reason>.)`.

**Dropped on purpose:** `-webkit-`/`-apple-` features, other non-standard features and legacy aliases (properties, pseudo-classes and elements, at-rules, SVG 1.1 features SVG 2 removed). The exceptions are the non-standard properties the engine itself uses and the `-webkit-` and `-epub-` aliases of standard properties, which are still kept (see `DIFFERENCES.md`). Everything in a standard is kept, even where Booey has no use for it yet, so it's easy to wire up.

Without scripting, the DOM's callbacks are C++: listeners are `Func`s owned by `EventListenerHandle`s, observer callbacks (`MutationCallback`, `ResizeObserverCallback`, `IntersectionObserverCallback`) are `Func`s, and the window has the spec's `Window` interface for C++ callers.

## Working on Booey

### Conventions

- **UTF-8 throughout.** There's no 8-bit/16-bit split and no Latin-1 path, and offsets into text are bytes. CSS code uses the aliases in `CSS/Types/CSSOMString.hpp` (`CSSOMString`, `CSSOMStringView`, `CSSOMStringAtom`) rather than the Core or `dom::` names; literals are `u8"foo"sv`. Text helpers live in Krystal.Core's `Text/` (add missing ones there), apart from those that need ICU (`TextBreakIterator`, `CharacterProperties`), which are in `Krystal.Booey/Core/` because Core doesn't link ICU.
- **Serialisers** take a `StringBuilder &` (`Text/StringBuilder.hpp`), whose variadic `Append` writes floats in ECMAScript `Number.prototype.toString` form. `TextStream` writes doubles as `%.2f`, rounding ties away from zero.
- **No weak sets or maps.** Holders keep lists or hash sets of raw pointers, walk them from copies and clear themselves on destruction.
- **Type lists** use brigand (vcpkg) rather than hand-rolled lists. Core doesn't link it, so its `VariantOrSingle` takes a pack: from a list, write `brigand::wrap<List, VariantOrSingle>`.
- **Naming.** British spelling in identifiers (`Serialise`). When a member shares its type's name (`CompositeOperation CompositeOperation`), write the type as `boo::X` inside the struct. Class constants (`Color::red`) and container-protocol methods (`size`, `get<I>`, `at`, `data`) are lower case. A method named after a type would hide it, so it's named otherwise (`Space()`, `GetQuad()`). The HTML parser's classes have no `HTML` prefix, which the `html` namespace gives (`html::TreeBuilder`, `html::DocumentParser`). The typed colour maths lives in Krystal.Core's `Colour/`, spelled "Colour" (`ColourSpace`, `ConvertColour`), so Render2D can use it without Booey; the CSS `Color` class and CSS concepts (`css::ColorType`, `ResolvedColor`) keep the spec's spelling.

**MSVC.** `TimingFunction`, `TransformOperation`, `FilterOperation`, `PathOperation` and `StyleImage` compare through a non-virtual `operator==` over a virtual `Equals`, because MSVC finds C++20's reversed `operator==` candidates ambiguous.

### The build

Booey compiles as a unity build: CMake joins its sources into batches of 16. Without it, the Debug static library outgrows the 4 GB a static library can hold.

- **File-local names share a batch.** Anonymous-namespace and `static` helpers, constants and types, and `using namespace` directives at namespace scope, are visible to the files after them in the batch: two files defining the same helper fail to compile, and a directive can make a name ambiguous elsewhere. Look for a shared helper before writing one (`text::CodePointAt` and friends in `Text/CodePointIterator.hpp`, `infra::StringAlgorithms`, `css::SkipExactly` in `ParserIdioms.hpp`), and put a new shared helper beside the type it works on.
- **Qualify instead of `using namespace`** at namespace scope (`mq::Feature`); function-scope directives are fine.
- **Sources whose helpers clash** compile on their own (`SKIP_UNITY_BUILD_INCLUSION` in `Krystal.Booey/CMakeLists.txt`: `WidthIterator.cpp`, `ComplexTextController.cpp`, `TextBoxPainter.cpp`, `RenderListMarker.cpp`, and the generated sources).
- **Include what each file uses.** A file can compile only because an earlier file in its batch included something; building once with `-DCMAKE_UNITY_BUILD=OFF` shows what's missing.
- Release is supported: variables only debug assertions use are `KRYS_MAYBE_UNUSED`, and MSVC builds with `/bigobj` for `InterpolationWrapperMap.cpp`.
- Code generators in `Krystal.Booey/codegen/` (`css_properties.py`, `css_pseudo_selectors.py`, and the user agent style sheets in `codegen/user-agent-style-sheets/`) produce the property, pseudo-selector and interpolation tables from `css-properties.json` and `css-pseudo-selectors.json`.
- Every header and source is listed in `Krystal.Booey/CMakeLists.txt`; a file that isn't listed isn't built.
