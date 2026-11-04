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
- [ ] **1. History.** Remove `History/` (`BackForwardCache`, `CachedPage`, `CachedFrame`,
  `BackForwardList`/`Controller`/`Client`, `HistoryItem`) and `Loader/HistoryController`. Krystal.UI `View`:
  remove `GoBack`/`GoForward`/`CanGoBack`/`CanGoForward`/`GoToItem`/`GetBackForwardList`, `FromBackForwardCache`,
  `DidRestoreFromBackForwardCache`, `DefaultBackForwardCacheCapacity`. `Forms/FormController`: remove saving and
  restoring form state to history items. `pageshow`/`pagehide` stay (`persisted` always false).
- [ ] **2. Window APIs.**
  - Remove `Page/Navigation/` (with `NavigationPromise`), `Page/History`, `PopStateEvent`, `HashChangeEvent`,
    `Page/Location`, `Page/Navigator` (with `Plugins/` and `View`'s user-agent override; check `Languages`
    readers first), `BarProp`, `External`, `WindowFeatures`, `Screen` (and `MediaEnvironment::ScreenAvailableRect`;
    keep `ScreenSize` and the depth fields for media queries), the `UserActivation` object (the window's
    activation state stays).
  - `LocalDOMWindow`: remove `alert`/`confirm`/`prompt`, `print`, `open`/`close`/`opener`/`closed`/`stop`/`name`,
    `moveTo`/`moveBy`/`resizeTo`/`resizeBy`, `screenX`/`screenY`, `status`, `postMessage`, `atob`/`btoa`,
    `window.event` (and the `_currentEvent` save/restore in `EventDispatcher.cpp`), `captureEvents`/
    `releaseEvents`, `clientInformation`, `reportError`, the origin and secure-context getters, the frame-tree
    getters. Trim `ChromeClient` to match.
  - Remove `MessageEvent`, `ErrorEvent` (and the special `onerror` handling in `EventTarget.cpp`),
    `BeforeUnloadEvent`, `Types/SerializedScriptValue`.
  - Console: remove `ConsoleTypes`, `ChromeClient::AddMessageToConsole`, `View::SetConsoleMessageHandler`/
    `ConsoleMessage`. A future inspector gets its own diagnostics channel.
- [ ] **3. Navigation goes to the host.** Booey doesn't navigate itself: link activation and form submission go
  to `View`'s navigation handler, and documents load only when the host asks. Fragment links still scroll within
  the document (no history entry, no `hashchange`).
  - Shrink `FrameLoader` to loading a request into the main frame: no history items, cached pages, scheduled
    navigation, `navigate`/`beforeunload` plumbing or target lookup.
  - Remove `NavigationScheduler` and `MicroParsers/Refresh`.
  - `FormSubmission`: keep the entry list and the `submit` event, hand the data to the host; remove the URL and
    body encoding with `Platform/Network/FormData` and `FormDataBuilder`.
  - Drag and drop: remove the "load the dropped URL" action.
  - Trim HTTP-only semantics: `CachePolicy` revalidation/expiry, `MemoryCache` expiry, `FetchOptions`/
    `ResourceLoaderOptions` mode and credentials, `LoadedFromOpaqueSource` (check readers first).
- [ ] **4. Subframes.** The frame tree is the main frame only. `<iframe>`, `<frame>`, `<object>` and `<embed>`
  stay as elements with reflected attributes but load no nested document (check whether `<object>`/`<embed>`
  image or SVG rendering needs a frame). Delete the "frames other than the main frame"/subframe FIXMEs.
- [ ] **5. Gamepad, timers, blobs.**
  - Remove `Modules/Gamepad/`, `Platform/Gamepad/`, Krystal.UI `GamepadProvider`, the window's gamepad listener
    counting, and the Editor's `PageGamepads` sample (and its markup in `pages/index.html`). `View`'s gamepad
    focus navigation stays.
  - Replace `DOMTimer` (`setTimeout`/`setInterval`, `Document::_timeouts`, the nesting clamp) with a C++
    document timer that fires as a document task, so it pauses while the document is suspended.
  - Remove `DOM/Scheduling/IdleCallbackController`.
  - Remove `HTMLCanvasElement::ToBlob` (and its tests) and `BlobPropertyBag`.
- [ ] **6. DOM and Editing leftovers.**
  - Remove `DOM/FragmentDirective/` and `TextIterator`'s `FoldQuoteMarks`.
  - Remove `Internals/VisitedLinkState`, `Style/ForVisitedLink`, `StyleVisitedLinkColorData` and the visited
    colour plumbing (`:visited` stays and never matches).
  - Remove mail blockquotes: `BreakBlockquoteCommand`, `InsertNewlineInQuotedContent`, the mail-blockquote checks,
    `Apple-paste-as-quotation` and `Apple-style-span` (keep `Apple-interchange-newline` and
    `Apple-converted-space`).
- [ ] **7. Rendering, CSS and SVG leftovers.**
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
- [ ] **8. FIXME sweep.** Delete the FIXMEs for the "not planned" features. Keep the FIXMEs for features wanted
  later: multi-column (and the fragmentation it needs), accessibility, video and audio, fieldsets, scroll
  snapping, reference targets, spellcheck/grammar/autocorrect, marquee; and continuations (implementation notes).
- [ ] **9. Non-standard CSS properties** (`codegen/css-properties.json`).
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
- [ ] **10. Promises.** Convert `AnimationPromise`, the last promise type, to callbacks (`WhenReady`/
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
