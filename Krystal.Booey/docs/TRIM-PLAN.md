# Booey trim plan

> **Temporary.** The working plan for trimming Booey to what a game engine needs. Delete this file (and its line
> in `Krystal.Booey/CLAUDE.md`) once every step is done.

Booey keeps the DOM, HTML, CSS, SVG and MathML a C++ application declares and renders documents with. It drops
what only makes sense in a browser: script-facing window APIs, session history, navigation, cross-document
features, the network and origin model, printing, and the non-standard WebKit leftovers.

Each step is its own commit, built and tested. Delete the FIXMEs a removal makes moot as part of it, and add a
`DIFFERENCES.md` entry for each standard API dropped.

## Steps

- [x] **0. Scope docs.** Reword the scope in `Krystal.Booey/CLAUDE.md` and `docs/OVERVIEW.md` (no longer "keep
  everything in a standard"), with a "not planned" list: scripting, network loads and HTTP semantics, origins and
  the security model, printing and pagination, session history and navigation, subframes, plugins, view
  transitions, async scrolling, rubber-banding, reflections, text autosizing.
- [x] **1. Session history and navigation** (with the window APIs that navigate and the navigation part of
  step 3, as `FrameLoader` tied them together). Removed `History/`, `HistoryController`, `NavigationScheduler`,
  `Refresh`, `Page/History`, `Page/Navigation/`, `Page/Location`, `PopStateEvent`, `HashChangeEvent`,
  `BeforeUnloadEvent`; `FrameLoader` loads only what the host asks and hands links and form submissions to the
  host; `View`'s back/forward and before-unload handler; the Editor's router and history buttons.
- [x] **1b. Form state.** `Forms/FormController`'s saving and restoring of form control state (nothing restores
  it now), the controls' `SaveFormControlState`/`RestoreFormControlState`, and form-associated custom elements'
  `Restore` callback.
- [x] **2. Window APIs** (with step 5's gamepads, as `Navigator` held the Gamepad API). Removed `Navigator`,
  `Plugins/`, `BarProp`, `External`, `Screen`, `UserActivation`, `WindowFeatures`, `ConsoleTypes`,
  `MessageEvent`, `ErrorEvent`, `SerializedScriptValue`, `Modules/Gamepad/`, `Platform/Gamepad/`, Krystal.UI's
  `GamepadProvider` and the Editor's `PageGamepads` (its nav-grid half is now `PageSpatialNavigation`); the
  window's prompts, print, `open()`/`close()`, name, geometry, frame-tree getters, `postMessage()`,
  `reportError()`, `atob()`/`btoa()`, origin getters and `window.event`; the online/offline, language change
  and gamepad events; the opener policy; `ChromeClient`'s and `View`'s matching hooks and the user agent; and
  the no-op legacy members (`captureEvents()`/`releaseEvents()`, `document.clear()`, `Range`/`NodeIterator`
  `detach()`, `window.blur()`). History-action activation stays: close watchers use it.
- [x] **3. Navigation leftovers.** (Navigation itself went to the host in step 1.) Form submissions hand the
  entry list to the host unencoded (`Platform/Network/FormData`, `FormDataBuilder` and the mailto handling are
  gone); dropping a URL loads nothing; `CachePolicy` is `Use` or `Reload`; `FetchOptions` lost mode and
  credentials, and `LoadedFromOpaqueSource` (read by nothing) is gone.
- [x] **4. Subframes.** (Nothing loaded nested documents already: the FIXMEs went, with the compositor's
  enclosing-frame attachment and the `IncludingSubframes` names.) The frame tree is the main frame only. `<iframe>`, `<frame>`, `<object>` and `<embed>`
  stay as elements with reflected attributes but load no nested document (check whether `<object>`/`<embed>`
  image or SVG rendering needs a frame). Delete the "frames other than the main frame"/subframe FIXMEs.
- [x] **5. Timers, blobs.** (The gamepads went in step 2.)
  - Replaced `DOMTimer` (`setTimeout`/`setInterval`, `Document::_timeouts`, the nesting clamp) with
    `dom::DocumentTimer` (`Document::StartTimer`/`StartRepeatingTimer`, returning a `DocumentTimerHandle`),
    which fires as a document task, so it pauses while the document is suspended.
  - Removed `DOM/Scheduling/IdleCallbackController` (and `Page::OpportunisticallyRunIdleCallbacks`).
  - Removed `HTMLCanvasElement::ToBlob` (and its tests). `BlobPropertyBag` stays: it's the kept `Blob` and
    `File` constructors' options.
- [x] **6. DOM and Editing leftovers.** (`::target-text` stays, styling text the host marks in the fragment
  highlight registry; `:link` matches every link and the cascade has one value per property.)
  - Remove `DOM/FragmentDirective/` and `TextIterator`'s `FoldQuoteMarks`.
  - Remove `Internals/VisitedLinkState`, `Style/ForVisitedLink`, `StyleVisitedLinkColorData` and the visited
    colour plumbing (`:visited` stays and never matches).
  - Remove mail blockquotes: `BreakBlockquoteCommand`, `InsertNewlineInQuotedContent`, the mail-blockquote checks,
    `Apple-paste-as-quotation` and `Apple-style-span` (keep `Apple-interchange-newline` and
    `Apple-converted-space`).
- [x] **7. Rendering, CSS and SVG leftovers.**
  - `-apple-color-filter`: `AppleColorFilter`/`AppleColorFilterValue`, `AppleInvertLightnessFunction`,
    `StyleAppleColorFilterData`, `EditingStyle::InverseTransformColorIfNeeded`, the `WithColorFilter` colour
    lookups.
  - `WebKitFontFamilyNames`, the `-webkit-user-modify` parser special case, `IsWebkitBoxShadow`,
    `SearchFieldResultsPart`.
  - `display: -webkit-box`/`-webkit-inline-box`, `RenderDeprecatedFlexibleBox` and the `-webkit-line-clamp`
    compat path.
  - `Rendering/Pagination`; `@page` (`CSSPageRule`, `CSSPageDescriptors`, `PageRuleCollector`,
    `Values/Page/PageSize`, the resolver's page style).
  - View transitions: `@view-transition`, `view-transition-name`/`-class`, `::view-transition*`,
    `viewTransitions.css`.
  - `SVGZoomAndPan`/`SVGZoomAndPanType`.
- [x] **8. FIXME sweep.** Delete the FIXMEs for the "not planned" features. Keep the FIXMEs for features wanted
  later: multi-column (and the fragmentation it needs), accessibility, video and audio, fieldsets, scroll
  snapping, reference targets, spellcheck/grammar/autocorrect, marquee; and continuations (implementation notes).
- [x] **9. Non-standard CSS properties** (`codegen/css-properties.json`).
  - Drop: `-apple-pay-button-style`/`-type`, `-apple-visual-effect`, `-webkit-tap-highlight-color`,
    `-webkit-touch-callout`, `-webkit-overflow-scrolling`, `-webkit-font-smoothing`, `-webkit-cursor-visibility`,
    `-webkit-line-align`, `-webkit-column-progression`, `-webkit-user-drag`, `-webkit-user-modify`,
    `-webkit-text-fill-color`, the `-webkit-text-combine`/`-webkit-ruby-position`/`-webkit-background-size`
    aliases, `-internal-text-autosizing-status` and text autosizing.
  - Make internal-only or C++ state: `-webkit-border-horizontal-spacing`/`-vertical-spacing`,
    `-webkit-text-decorations-in-effect`, `-webkit-locale`, `-webkit-text-security`, `-webkit-rtl-ordering`,
    `-webkit-line-box-contain`, `-webkit-mask-position(-x/-y)`.
  - Rename to the standard property: `-webkit-initial-letter` → `initial-letter`, `-webkit-user-select` →
    `user-select`, `-webkit-hyphenate-limit-*` → `hyphenate-limit-chars`/`-lines`.
  - Keep for now: the `::-webkit-*` form control pseudo-elements and `-webkit-link`/`-webkit-focus-ring-color`
    keywords (the only way to style built-in controls until CSS Forms' pseudo-elements).
- [x] **10. Promises.** Convert `AnimationPromise`, the last promise type, to callbacks (`WhenReady`/
  `WhenFinished` with an error path), keeping its replaced-promise and reject-on-cancel behaviour.
- [ ] **11. Finish.** Delete this file and its line in `Krystal.Booey/CLAUDE.md`.

## Kept on purpose

- Animation (CSS animations and transitions, Web Animations, scroll and view timelines, SMIL),
  `requestAnimationFrame`, `AbortController`, `TextEvent`/`BeforeTextInsertedEvent`, `NodeIterator`/`TreeWalker`, the Typed OM.
- `PointerLockController`, `SpatialNavigation`, drag and drop, Resize/IntersectionObserver, `matchMedia`.
- `Blob`/`File`/`FileList`, `<input type=file>`, URL, XML, the Sanitizer, `SmartReplace`, `EditingBehavior`.
- `Forms/Autofill` (computes the `autocomplete` IDL value; a FIXME on `AutofillData` notes a possible trim).
- Every HTML element (obsolete ones included), `<menclose>`, `<marquee>`, SVG `<view>` fragments.
- The image decoders (including ICO; they may move to a shared engine module) and the Adwaita theme.

## Later

- Show/hide for kept-alive screens (the future MVC layer): `Page::SetIsVisible(false)` (`visibilitychange`), then
  `Document::Suspend`; the reverse to show. Richer notifications go on MVC controllers as C++ callbacks.
- SVG 2 aliases: `SVGMatrix`/`SVGPoint`/`SVGRect` as `DOMMatrix`/`DOMPoint`/`DOMRect`.
- A pass over inherited WebKit FIXMEs (bug links, upstream notes): delete links, turn real defects into
  `KNOWN-ISSUES.md` entries.
