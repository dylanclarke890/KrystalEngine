# Known issues

Known bugs and missing features in Krystal.Booey, by area. A bug with a `[!shouldfail]` test names it; most entries also have a `// FIXME:` where the fix goes. Remove an entry when it's fixed, and add one for a bug that's found but not fixed, with a `[!shouldfail]` test when one can be written.

What's out of scope by choice (scripting, the network, frames, workers, accessibility and the rest) is in `OVERVIEW.md` and isn't repeated here. Where Booey deliberately follows a specification, Chrome or upstream rather than a missing feature, it's in `DIFFERENCES.md`. Issues in Render2D and Krystal.UI are in `docs/gfx/OVERVIEW.md`.

## DOM

### Bugs

- A hidden page doesn't hide its main frame's view, and a shown one doesn't resume its animated images, as upstream's do (FIXMEs in `Page.cpp`): views have no shown state and animated images pause only out of view.
- `AbortSignal` keeps its source and dependent signals as weak pointers rather than following the specification's garbage collection rule.
- Event listener callbacks that capture their own target keep it alive (there is no garbage collector); noted on `AddEventListener`.

### Missing

- No markup-as-string APIs: `innerHTML`, `outerHTML`, `getHTML()`, `setHTMLUnsafe()`, `insertAdjacentHTML()`. Only `Document::ParseHTMLUnsafe` exists.
- No `dataset`, no `document.lastModified`, and `title` is never shown (the chrome client has no tooltips). A language from a `Content-Language` response header is not read (only the `<meta>` pragma is; FIXME in `SelectorCheckerTestFunctions.cpp`).
- The table APIs (`caption`, `createCaption()`, `rows`, `insertRow()`, ...) are FIXMEs in `HTMLTableElement.hpp`.
- No Fullscreen API (`:fullscreen`).
- `Element` doesn't run the rest of `didMoveToNewDocument` (FIXME in `Element.cpp`): nodes aren't told they have moved. Observer data does move (`Element::OnAdopt`), but a canvas moved to another document doesn't move its context's preparation (FIXME in `HTMLCanvasElement.hpp`), and lazy-image moves are untested.
- No customized built-in elements: definitions have no `extends`, and an element's `is` value is kept (cloned, serialised) but never looks up a definition. Elements created with an `is` value are uncustomized.
- Failed custom element constructions and upgrades, exceptions thrown by listeners and parse errors are not reported anywhere (no console; FIXMEs in `CustomElementInterface.cpp`).
- `[CEReactions]` scopes are missing on members that aren't implemented yet (`innerHTML`, `outerHTML`, `insertAdjacentHTML`, `setHTMLUnsafe`, `designMode`, `execCommand`, `document.open()`/`write()`, `<table>` members, the `ElementContentEditable` members `contentEditable`, `enterKeyHint` and `inputMode`, `nonce`, `DOMStringMap`).
- `ElementInternals` has no ARIA reflection members (`Reflection::GetContentAttribute` returns null for them) and no `CustomElementDefaultARIA`. Form-associated custom elements' state is restored only in the restore mode, never `FormStateRestoreMode::Autocomplete` (no autofill).
- `RadioNodeList`/`HTMLFormControlsCollection` and select collections are not cached: `options` and `selectedOptions` make a new collection each call and `InvalidateSelectedItems` does nothing.
- The backup custom element queue is global rather than one per agent.
- No `onpointer*`, `ontouch*`, `onpointerlockchange` or `onpointerlockerror` handlers, no `AbortSignal.onabort`, `document.onreadystatechange`, and no outermost-`<svg>` window event handlers (`onunload`, `onresize`, `onscroll`, ...).
- `DocumentTouch` (`createTouch`, `createTouchList`) is not ported; `Touch` has no constructor and `TouchInit` is not ported.
- `DataTransferItem` has no `getAsEntry` (no file system API).
- No `FileReader`, streams, blob URLs or blob registry. A `Blob` holds its data itself (`BlobData`), and `Blob::Bytes()`/`Text()` read synchronously, returning the exception a failed read rejects with (`NotFoundError` for a missing file, `NotReadableError` otherwise).
- A task's document is not checked for being fully active (documents never in a frame, such as `createHTMLDocument()`, run their tasks). Subresource loads, `close()`, the scroll to the anchor and `AbortSignal.timeout()` are not tagged with a document, so suspension does not hold them.
- Suspension covers only the document's timers, animation frame callbacks and gamepad haptics: no media, view transitions, font loading, service workers or IndexedDB, and animations aren't suspended (FIXME in `WebAnimation.hpp`).
- Security origins do not exist: `CSSStyleSheet::CanAccessRules`, `ParserContext::HasDocumentSecurityOrigin`, `canReceiveDragData`, `canDisplay` and cross-origin image reuse are FIXMEs that allow everything, and `EventHandler::CanDropCurrentlyDraggedImageAsFile` is always true.
- There is no ID map per tree scope: `getElementById` and selector fast paths walk the tree, live collections re-walk on every access, and `HTMLImageElement::AssociatedMapElement` walks the tree too. A node in a shadow tree finds its tree scope by walking to its root on every call.
- `FirstChild()`, `ParentNode()` and friends return raw pointers rather than `RefPtr`; `NodeFilter` is clunky from C++.

## HTML parser

### Missing

- Parse errors aren't reported (no console, FIXME in `HTMLTreeBuilder.cpp`).
- Unimplemented tree builder steps, all FIXMEs in `HTMLTreeBuilder.cpp`: the rest of `<template>` start-tag handling in "in head", script end tags in "text", the stop-parsing steps, the `xmlns` attribute parse error, the throw-on-dynamic-markup-insertion counter, and resetting resettable elements on creation.
- The tree builder does not give form-associated elements their parser form owner on creation: `FormAssociatedElement::SetFormSetByParser` exists but nothing calls it (FIXME in `HTMLTreeBuilder.cpp`).

## HTML elements and forms

### Bugs

- `pattern` matches with `std::wregex` (ECMAScript) over UTF-16 instead of a JavaScript engine's regular expression with the `v` flag, and e-mail addresses with a `std::regex`; behaviour on exotic patterns can differ from browsers.
- `LocalDOMWindow::Stop` doesn't stop the navigable's loading (FIXME).
- `close()` closes any page: it does not check that the page is script-closable (FIXME).
- `HTMLTableColElement::OnAttributeChanged` does not lay a column out again when its `width` attribute changes (FIXME); `Numbers::ParseInteger` can parse it.

### Missing

- `<input type=file>` has no file chooser, file icon (`icon()`, `requestIcon`, `iconLoaded`) or tooltip naming the chosen files; the host's file picker is a FIXME in `FileInputType.hpp`. The name uses the room the icon would take. Files are set from C++ (`FileInputType::SetFiles`, which fires `input` and `change` unless set by script), dropped, or restored with form state; there are no directory uploads.
- `<datalist>`: `HTMLDataListElement` has no `options` and `HTMLInputElement` has no `list` (FIXME), so there are no list suggestions, tick marks on sliders, list button, or colour and date suggestions.
- `<input type=color>` has no `alpha` or `colorspace` support (FIXME in `ColorInputType.hpp`) and no `EventQueueScope`, so its events aren't queued until the value is set (FIXME in `ColorInputType.cpp`).
- Opening a date or colour picker by clicking the control isn't checked against a user gesture (`showPicker()` is); FIXMEs in `BaseDateAndTimeInputType.cpp` and `ColorInputType.cpp`.
- No autofill (`:autofill`, `FormStateRestoreMode::Autocomplete`), caps lock indicator or strong password fields on password fields, validation message bubble, `RenderSearchField` (search fields have no recent searches), or input purpose hints (`inputmode`, `autocapitalize`, spell checking). Numbers aren't localised (FIXMEs in `NumberInputType.cpp`); dates are, through ICU.
- Form submission has no `rel` handling (`noopener`, `noreferrer`, `opener`; FIXME in `HTMLFormElement.cpp`), no image elements in `elements` (FIXME), no request body in the navigation (FIXME in `FormSubmission.cpp`), and encodes UTF-8 only.
- `<select>`: list boxes have no pan scrolling, there is no customizable `<select>` (`appearance: base-select`), and option changes are not reported to the chrome client (FIXME in `HTMLSelectElement.cpp`).
- Fieldset legends are not placed in the border (see *Layout*).
- `<img>` lacks form association, `decode()` and the `Image()` constructor (FIXMEs in `HTMLImageElement.hpp`), and doesn't select a source again when media queries change (FIXME in `HTMLImageElement.cpp`). `HTMLImageLoader` fires no events for video posters and has no `<object>` fallback (FIXMEs); `ImageLoader`'s reuse of available images has no preloads or origins (FIXMEs).
- `HTMLLinkElement` recognises only `rel=stylesheet` and `alternate`: no preloads, icons, manifests, integrity, CORS, referrer policies or priorities (no `LinkRelAttribute`/`LinkLoader`).
- `<a ping>`, `opener` and referrers are not implemented; the `download` attribute is only passed to the host. `<object usemap>` is not supported.
- `<marquee>`'s `hspace`, `vspace`, `loop`, `scrollAmount`, `scrollDelay`, `trueSpeed`, `start()` and `stop()` are not implemented, and marquees don't scroll (FIXME in `HTMLMarqueeElement.hpp`).
- `autofocus` insertion steps run only for HTML elements, not SVG or MathML elements.
- Popovers: no `interestfor` and no `:open`.
- `<details>` auto-expansion for find in page waits for a find feature; text directly inside a `<details>` isn't revealed by a text fragment (reveal starts from the element around the text).
- `MIMETypeRegistry::MIMETypeForExtension` has only a built-in table; most other extensions have no type (FIXME in `MIMETypeRegistry.cpp`).
- Slider thumbs can't be dragged by touch: a touch pan scrolls (upstream handles touches on sliders only on iOS).

## CSS parsing and CSSOM

### Bugs

- `Percentage` consumers don't divide percentages produced by `calc()` by 100 (upstream does the same). Test: `PropertyParserHelpers(ConsumePercentageDividedBy100OrNumber calc percentage)` (`[!shouldfail]`, `tests/CSS/Properties/Consumers/Percentage.cpp`).
- A root `calc()` `Negate`/`Invert` serialises only its child (`calc(1em)`), although "serialize a math function" step 4 serialises the root's children. Test: `TreeSerialisation(root negate and invert)` (`[!shouldfail]`).
- Trigonometric functions in `calc()` evaluate their argument as radians: simplification converts degrees first but evaluation doesn't, so an unsimplified angle that reaches evaluation (for example `sin(sibling-index() * 10deg)` through `DoubleValue`) is wrong. Test: `TreeEvaluation(trigonometric functions of angles)` (`[!shouldfail]`).
- `:heading` / `:heading()` (Selectors 5) aren't in the pseudo-class table, and `:host(:not(.a .b))` parses although WPT says it is invalid. Tests: `CSSSelectorParser(heading-01.dat)` and `CSSSelectorParser(not-03.dat)` in `tests/CSS/Selectors/SelectorParser.cpp` (`[!shouldfail]`).
- Range syntax in `style()` container queries and lists of container queries (css-conditional-5) aren't supported. Two cases in `tests/CSS/Queries/ContainerQueryParser.cpp` (`[!shouldfail]`).
- `css::calc::Value::CreateCalculationValue(NoConversionDataRequiredTag)` passes no conversion data, so a tree with a length (which doesn't require conversion data) asserts in `style::calc::ToStyle`, as upstream. `ComputeCanonicalNonCalcLengthDouble` with conversion data that has no style dereferences a null style unless computing font-size.
- `PaintImageValue` doesn't serialise its arguments.
- `SingleAnimationRangeType` is defined twice (`css`, in `Consumers/Timeline.hpp`, and `style`, in `SingleAnimationRange.hpp`); upstream only declares it in the parser header, so the `css` one should go.
- Upstream bugs kept: `CSSPageRule::SetSelectorText` parses a selector list rather than a page selector; `CSSKeyframeRule::SetKeyText` changes the keyframe outside a mutation scope; `CSSNamespaceRule::Reattach` keeps the old rule; removing a shorthand returns an empty string; the `negative` descriptor always writes its suffix; serialising a name-only container query gives `--foo ` with a trailing space; the specificity of a bare `&` hits a debug assertion (nesting must be resolved first).
- `@import ... supports()` is parsed in `CSS.supports()` mode, which falls back to general-enclosed and is never invalid, so the "discard the import rule" branch is unreachable (upstream's).
- `BorderImage.cpp` and `MaskBorder.cpp` in `Style/Values` use literal initial values instead of `ComputedStyle`'s (FIXME).
- Copying a `StyleSheetContents` leaves out its `@import` and `@namespace` rules (upstream's FIXMEs).

### Missing

- Typed OM has only numeric values and `CSSKeywordValue` (`CSS/TypedOM/`). There is no `CSSStyleValue::Parse()`/`ParseAll()`, `CSSStyleValueFactory`, `StylePropertyMap` (`styleMap`, `attributeStyleMap`, `computedStyleMap()`), `CSSUnparsedValue`, image, transform or colour values. `CSSNumericFactory` is a struct of static functions (`CSSNumericFactory::Px(10)`) and there is no `CSS` namespace object.
- No `CSS.registerProperty` or `CSS.escape`. `CSSStyleSheet` has `ReplaceSync` only; the async `replace()` needs promises.
- `SingleAnimationRangeStart/End::ToTimelineRangeValue()` returns a null `CSSNumericValue` for a calculated offset (FIXME); upstream dereferences a null percentage.
- The `color-layers()` function doesn't parse a blend mode (FIXME in `Consumers/Color.cpp`), so only normal blending is reachable from CSS.
- The `FontFace` string parsers (`ParseFontFaceSrc`, `ParseFontFaceFontWeight` and the rest, FIXME in `Consumers/Font.hpp`) aren't written.
- `@font-face`'s `font-size-adjust`, features and variations aren't applied to loaded fonts (FIXMEs in `FontCustomPlatformData.cpp`). `@font-palette-values` and `@font-feature-values` are parsed but have no effect on fonts (FIXME in `CSSFontSelector.cpp`).
- The `s` attribute-selector flag (Selectors 4) is rejected (FIXME in `SelectorParser.cpp`); supporting it needs a selector flag and serialisation.
- `::part()` doesn't follow `exportparts`: part names match only in the host's own scope (FIXMEs in `SelectorChecker.cpp` and `ElementRuleCollector.cpp`).
- State pseudo-classes that match nothing, with FIXMEs in `SelectorChecker.cpp`: `:autofill`, `:fullscreen`, `:picture-in-picture`, the media playback pseudo-classes, `:future`/`:past`, `:active-view-transition(-type)` and the view-transition pseudo-elements. `:visited` matches only links with an empty `href` (see *Style*).
- `:lang()` has no document default language or Content-Language header (FIXME in `SelectorCheckerTestFunctions.cpp`); WebVTT elements don't use their cue's language.
- `SelectorQuery` has no ID fast paths (rightmost-ID lookup, filtering the root by an ancestor ID, `[id=...]`; FIXMEs until tree scopes map IDs).
- No user or extension style sheets: `ScopeRuleSets::InitializeUserStyle` adds nothing and `Style::Scope` has no injected author sheets (FIXMEs), user style sheets' `@import`s and load options aren't handled, and `ImportRule` has FIXMEs for opaque CORS sources.
- `RuleSet` doesn't clone `::cue` rules for the cue background box (no WebVTT cues).
- `SizesAttributeParser` doesn't clear the font selector for font-relative units (FIXME).
- The standard `user-select` isn't implemented; only `-webkit-user-select` exists and the user agent sheets use it.
- `css::calc::Value::DoubleValueDeprecated` doesn't log when the tree needs conversion data (FIXME in `ValueEvaluation.cpp`).
- There is no inspector: `:visited` and `:hover` can't be forced and there are no observer hooks for source ranges.
- Imported sheets are always strict about the sheet type; quirks-mode lax loading for same-origin requests needs security origins (FIXME in `StyleSheetContents.cpp`).

## Style (cascade and computed values)

### Bugs

- `Scope` iterates every element when viewport units change instead of a saved element list (FIXME in `Style/Scope.cpp`).
- `scrollContainerSizeForPositionOptions` in `TreeResolver.cpp` tests `IsScrollContainerY` twice (FIXME: the second is likely the x axis); upstream's bug.
- `AnchorScrollAdjuster`'s constructor (`AnchorPositionEvaluator.cpp`) reads `align-self` in both writing modes for the vertical axis' `anchor-center` (FIXME: the vertical writing mode likely means `justify-self`); upstream's bug.
- `Style::Scope::UpdateActiveStyleSheets` doesn't wait for a living render tree or defer the update while style is being resolved (FIXME in `Scope.cpp`).
- `TreeResolver` has a FIXME that its animation handling is wrong when transitions and animations both run on one element.

### Missing

- `env()` has no values: without the document's `ConstantPropertyMap` an `env()` reference falls back or is invalid at computed-value time (FIXME in `VariableReferenceValue.cpp`).
- `::cue` rules for WebVTT elements aren't collected (FIXME in `ElementRuleCollector.cpp`).
- View transitions are not implemented: named view transition pseudo-elements get no captured class list, the document element's pseudo-elements aren't created in `TreeResolver`, and an element captured in a view transition isn't forced flat in `Adjuster`.
- `VisitedLinkState` has no visited link store: links with an empty `href` (the document itself) are visited, all others unvisited, and links aren't invalidated when their visited state changes.
- Pagination (`-webkit-paged-x/y`, `setColumnStylesFromPaginationMode`), visually ordered documents and design mode aren't applied in `ResolveForDocument` and `Adjuster` (FIXMEs).
- No visibility adjustment (`VisibilityAdjustment`): `adjustVisibilityForPseudoElement` and the force-hidden subtree adjustment are FIXMEs.
- `Adjuster` leaves SVG transforms not stored on the style, and the layer-based SVG engine's `z-index`/stacking branches, as FIXMEs.
- `Style::Extractor` has no branch for properties with running keyframe effects (FIXME).
- `Style::PendingResources` has no user agent shadow tree support and no anonymous mode for masks (FIXME).
- Anchor positioning has no fragmented flows: an anchor's rect isn't the bounding box of its fragments (FIXMEs in `AnchorPositionEvaluator.cpp`). Anchor-positioned viewport-constrained objects need a scrolling coordinator Booey doesn't have.
- The switch control sheet (`htmlSwitchControl.css`) and the fullscreen sheet are never loaded (FIXMEs in `UserAgentStyle.cpp`).
- `scroll-behavior` is read by `UseSmoothScrolling`, but nothing acts on it (no scroll animations).
- `DidChangeViewportSize` doesn't visit pseudo-elements, as they aren't nodes (FIXME in `Scope.cpp`).
- `RenderStyle::HyphenString()` gives the hyphen-minus for `hyphenate-character: auto` (FIXME: U+2010 when the primary font has the glyph, locale dependent).

## Animations

### Bugs

- `DOMHighResTimeStamp` is an integer, so `DocumentTimeline` origin times lose their fractions (see *DOM*).
- SMIL animations don't pause for the image-animation setting (FIXME in `SVGDocumentExtensions.cpp`).
- `SMILTimeContainer` runs on a fixed 1/60 s timer (`SMILAnimationFrameDelay`, no throttled delay) and mutates the DOM while updating animations, as there is no `EventQueueScope` (FIXMEs in `SMILTimeContainer.cpp`).
- `KeyframeEffect::TimeToNextTick` wakes for the next iteration whether or not the document has `animationiteration` listeners (FIXME).
- `DocumentTimeline::AnimationInterval` is always 15 ms (`FullSpeedAnimationInterval`), as there is no frame rate alignment (FIXME: the host's display refresh).

### Missing

- Animations aren't suspended in hidden pages, aren't aligned to a frame rate (FIXMEs in `AnimationTimelinesController.cpp`), and aren't suspended with their document (FIXME in `WebAnimation.hpp`).
- No `Animation.frameRate` or `CustomEffect` (non-standard WebKit proposals).
- Scroll and view timelines have no accelerated representation (no threaded animations), and a change in view timeline metrics doesn't update the timeline (FIXME in `KeyframeEffect.cpp`).
- Keyframe offsets in named timeline ranges compute to NaN without a view timeline, as upstream.
- View transitions' animations are absent, so view transition names take no part in composite order comparisons (FIXME in `WebAnimationUtilities.cpp`) and group pseudo-elements' size and transform animations don't exist.
- `TreeResolver` skips the back/forward cache and printing checks on the animated-update path, and has a FIXME to push after-change style into the parent stack.
- Image animations pause out of view and resume (`RenderView::AddRendererWithPausedImageAnimations`), but the test is only the main frame's visible content rect (FIXME in `LocalFrameView.cpp` for subframes), and there is no per-element or page animation control (`RenderElement::AllowsAnimation` is always true; FIXME).
- The per-timer throttle in `DOMTimer` for timers that make no visible changes is a FIXME.
- Extent computation for animated motion path (`offset-*`) properties is missing (FIXME in `KeyframeEffect.cpp`).
- `onbegin`, `onend` and `onrepeat` attributes on SMIL elements don't exist.
- Quirks that prevent acceleration (`shouldPreventKeyframeEffectAcceleration`) aren't implemented (FIXME).

## Layout

### Missing

- There is no multi-column layout or fragmentation: no column renderers, column rules, fragmented flows or pagination. `collectFragments` makes one fragment, paginated layers paint unfragmented, and out-of-flow boxes, `TextBoxTrimmer`, `RenderView::NodeForHitTest`, the highlight range iterator and repaint containers have no column handling (FIXMEs). Multi-column flows don't lay out `content-visibility` skipped content (FIXME in `RenderBlockFlow.cpp`).
- Continuations do not exist, so `RenderInline` keeps both its start and end borders, margins and padding and always paints its own outline when a block sits inside it. FIXMEs remain in `RenderInline`, `RenderBlock`, `RenderElement`, `OutlinePainter`, `BoxGeometryUpdater` and the render tree builders.
- Fieldset legends are not placed in the fieldset's border: layout, preferred widths (`ComputePreferredWidthsForExcludedChildren` returns false), painting and hit testing of the legend and the fieldset's intrinsic border are FIXMEs in `RenderBlock.cpp` and `LayoutIntegrationBoxGeometryUpdater.cpp`.
- Image-based `shape-outside` is not supported: `ShapeOutsideInfo` builds only box, rectangle and polygon shapes (no raster shape, no `shape-image-threshold`), and shapes have no display paths for highlighting (FIXMEs in `LayoutShape.hpp` and `ShapeOutsideInfo.cpp`).
- `text-combine-upright` is not implemented: there is no combined-text renderer (`RenderCombineText`), so `BoxTreeUpdater`, the inline iterators, `RenderBlockFlow` and `RenderText` treat combined text as ordinary text (FIXMEs).
- `layout_integration::GridLayout::CanUseFor` accepts only single-track spans at explicit unnamed lines and the self-alignments `start`, `flex-start`, `self-start`, `normal` and `stretch`; other grids fall back to `RenderGrid`. Multi-column spans in `ImplicitGrid` are a FIXME.
- `RenderTreeAsText` leaves out SVG paint features and visual overflow, file upload control names, the fieldset's intrinsic border and the Apple-style-span editing artefact (FIXMEs).

## Painting

### Bugs

- A mask's `color-interpolation: linearRGB` is drawn in sRGB (`LegacyRenderSVGResourceMasker`, FIXME), because image buffers have no colour space conversion.
- `Path::ApplyElements` applies nothing for continuous rounded rects, and `Path::Transform` drops them, as there's no platform path (FIXMEs in `Path.cpp`); nothing in Booey makes one yet.
- A composited mask's or clip path's edge pixels (and those of a turned masked layer) can differ slightly from the whole-page repaint.

### Missing

- The compositing operators `darker` and `difference` are not mapped: `CompositeOperator` falls back to source-over (FIXME in Krystal.UI's `GraphicsContextRender2D.cpp`).
- There's no software filter path: filters are applied by the host's accelerated backend, or by the graphics context for the ones it can draw. Reference filter clients and SVG shapes' filter regions are FIXMEs (see *SVG*), and `filter: url()` to an external document is not supported (see *Images and resources*).
- Backdrop filters are drawn only by the compositor: the flattened whole repaint (`View::Record`) doesn't draw them.
- `::backdrop` is missing from repaint and `Styleable::FromRenderer`, and there are no view transition captures.
- The broken image icon is not drawn (FIXMEs in `CachedImage.cpp` and `RenderImage.cpp`).
- `ImageQualityController::ChooseInterpolationQuality` returns the style's quality or the default: there is no size- or scroll-dependent choice.
- Text painting has no document markers (spelling, grammar), no IME composition painting, no dragged or transparent content and no glyph display list cache. There is no print-backgrounds setting, and font subpixel quantization is a FIXME.
- `RenderReplaced::paint` skips event regions, accessibility bounds and document markers (FIXMEs); SVG painting has no event region context, and the paint milestones are a FIXME.
- Render theme parts left out or FIXMEs: media controls, attachments, Apple Pay, image controls, `<input type=checkbox switch>`, devolvable widgets, the system preview badge, data list parts, the text area resize corner image, and the system's reduced motion and contrast settings.
- Dragged images' files have no icons, there are no text indicators, and images changed mid-drag are not updated. There are no root view rects (`caretRectInRootViewCoordinates`, `editableElementRectInRootViewCoordinates`) for hosts' drop previews.
- Frame scrollbars have no pan scroll icon or overhang areas.
- `image-rendering` is not applied to composited images and canvases (see *Compositing and scrolling*).

## Compositing and scrolling

### Bugs

- Composited scrollers at fractional positions place their scrolled contents without pixel snapping (upstream's "need to do some pixel snapping here" FIXME in `RenderLayerBacking.cpp`), so contents can sit a pixel off the whole repaint; it shows in the editor's second list card.
- `TextureMapperAnimation.cpp` carries two inherited FIXMEs about suspicious handling: `_totalRunningTime` is cleared where it seems it should not be, and `Pause` takes a time offset where it is passed a time.

### Missing

- `GraphicsLayerTextureMapper` has no contents scaling filters, so composited images and texture canvases with `image-rendering: pixelated` or `crisp-edges` scale smoothly.
- There are no scroll animations: no smooth scrolling (`scroll-behavior`, `scrollTo()` with `behavior`, animated keyboard scrolling), scroll snapping (including snapping after a thumb move and for wheel scrolling), scroll anchoring, autoscroll timers beyond the scrollbar repeat, wheel latching, kinetic scrolling, touchpad gesture states or rubber-banding (FIXMEs in `Element`, `LocalFrameView`, `EventHandler`, `ScrollableArea`, `Scrollbar`). Scroll positions are whole pixels, so wheel deltas are rounded.
- There are no overlay scrollbars, native scrollbars, scroll corners or resizers. `scrollbar-color` has no effect, and `ScrollableArea` has no animator or scrollbars controller.
- `VisualViewport` is the layout viewport at the page scale and never scrolls within it, so its `scroll` event never fires (its `resize` does). There are no obscured insets, auto-sizing, base viewport layout size scale factor (FIXME in `VisualViewport.cpp`) or layout viewport separate from the scroll position. Pinch zoom is only the page scale (1 to 5) from the view's gesture recogniser.
- There is no async scrolling or threaded compositing: no scrolling coordinator or tree, no reconciliation of the composited scroll position after user scrolls, and animations and scrolling run on the main thread. A sticky box stuck to the view therefore repaints its old and new rects on scroll instead of compositing.
- Only `transform` and `opacity` animations run on layers: `GraphicsLayerTextureMapper::AddAnimation` refuses the rest, so animations of filters, `backdrop-filter` and the individual transform properties run on the page.
- 3D rendering contexts do not exist: `preserve-3d` sorting and perspective are not done, and Render2D draws 3D transforms flattened (FIXMEs in `TextureMapper.cpp`, `TextureMapperLayer.cpp`).
- Reference (SVG `url()`) filters are identities on composited layers (`TextureMapper.cpp` converts the other filter functions to `LayerFilter2D`).
- Clip paths never composite as shape layers; a composited clip path is painted into the mask layer. Replicas (reflections), repaint counters, repeating contents tiles (`WrapMode::Repeat`) and creating tiles over several frames are FIXMEs.
- `TextureMapperLayer` composites the whole tree each frame, with no damage tracking.
- The compositor has no scrolling nodes, iframe or widget layers, video or plug-in layers, event or interaction regions, layer flush throttling, model or separated layers, or HDR and wide gamut backings (`ContentsFormat` is RGBA8 only). There is no `TiledBacking` (`SetIsInWindow` and `SetTileCoverage` do nothing, as with upstream's TextureMapper), the fixed root background layer is never composited, and the host's compositing policy under memory pressure and subframe `forceCompositingMode` are FIXMEs.

## Text and fonts

### Bugs

- Synthesised small caps write a capital in place of a lowercase letter only when both have the same UTF-8 length; otherwise the letter is shaped as written (FIXME in `ComplexTextController.cpp`).
- `font-size-adjust` does not change the platform font's size: `FontPlatformData::updateSizeWithFontSizeAdjust` is not called from `FontCache` or `FontCustomPlatformData` (FIXMEs).
- A page whose `@font-face` font is already in the memory cache keeps its table cells' preferred widths from before the font applied (a fifth of a pixel per cell in `bidi-layout`), so that golden fails when another page was loaded earlier in the process. Test: "Golden: a page whose web font is in the memory cache lays out as with a fresh one" (`[!shouldfail]`, `Krystal.UI.Tests/tests/Golden.cpp`).

### Missing

- Fonts come only from the host and the bundled faces; there is no system font lookup or system fallback. `SystemFontDatabase::PlatformSystemFontShorthandInfo` returns the standard family at 16px, normal weight, for every system font keyword (`caption`, `icon`, `menu`, `message-box`, `small-caption`, `status-bar`).
- Variable fonts are shaped at their default instance (FIXME in `FontHarfBuzz.cpp`) and optical sizing is not applied (FIXME in `ComplexTextControllerHarfBuzz.cpp`).
- There are no hyphenation dictionaries, so `hyphens: auto` never hyphenates (FIXME in `Core/Hyphenation.hpp`).
- Colour glyphs and emoji presentation are not detected (FIXME in `FontHarfBuzz.cpp`), so `font-variant-emoji` only adds a variation selector to fallback clusters. `ResolveEmojiPolicy` cannot return `RequireText` or `RequireEmoji` (upstream's FIXME in `FontCascade.cpp`).
- Colour glyphs from bitmap strikes (`CBDT`) are drawn at their strike's size, unscaled, while COLR glyphs scale. Large colour glyphs stay bitmaps.
- `text-transform: full-width` does nothing (FIXME in `RenderText.cpp`; upstream implements it only on Cocoa).
- A backslash is never shown as a yen sign, because that depends on the font and the document encoding (FIXME in `RenderText.cpp`).
- `FontDescription::SetSpecifiedLocale` resolves every Han locale to `zh-hans`, as there are no user language preferences.
- `FontCache` has no memory-pressure limits; the host cannot report pressure (FIXME in `FontCache.cpp`). The purge of inactive fonts runs on a timer only.
- Right-to-left scripts other than Arabic and Hebrew (Syriac, Thaana, N'Ko and so on) have no bundled font and work only if the host registers one. Kashida justification is not done.
- Web fonts: SVG fonts, font collections (the item in a collection is not selectable), a font parsing policy (safe font parser, trusted types) and `FontFace`/`FontFaceSet` as an API from C++ are missing. Font loading has no settings: installed fonts are always allowed and load completions are never ignored.
- Text search (`findPlainText`, `SearchBuffer`) is absent, as is the text fragment finder's first pass through it. The text iterator does not traverse the flat tree, enter image overlays or emit text placeholders (FIXMEs in `TextIterator.cpp`).
- Text fragments have no text indicator and no directive generator (making a link to a selection).
- There are no spelling or grammar markers, text checking, dictation or kill ring.

## Images and resources

### Missing

- External reference filters (`filter: url(doc.svg#f)`) are not loaded or watched (FIXME in `RenderLayerFilters.cpp`). `cross-fade()` and filter images load their inputs but do not follow them as clients (FIXMEs in `StyleCrossfadeImage.cpp`, `StyleFilterImage.cpp`), so they do not update when an input loads, and `StyleCrossfadeImage::Blend` always returns null, so `cross-fade()` percentages don't interpolate.
- Style sheets and linked text decode as UTF-8 only: the request's, response's, `@charset` and `<link charset>` encodings are not honoured, and an XML document's encoding declaration is recorded but not followed. There are no `nosniff` or HTTP status checks (FIXMEs in `CachedCSSStyleSheet.cpp`).
- `CachedResource` has no revalidation by validators, preloads, CORS, priorities, digests or streaming and partial data; a resource decodes only once all its data has arrived. `CachedResourceLoader` loads images, fonts and style sheets only; other types, request counting, security checks and loads that do not delay the load event are FIXMEs. `MemoryCache` has one LRU list and no sessions, partitions or origins. `SharedBuffer` is one contiguous immutable buffer.
- Images decode synchronously: there is no decoding work queue, so `DecodingMode::Asynchronous` decodes at once and its callbacks run before the call returns (FIXMEs in `BitmapImageSource.cpp`). `HTMLImageElement` and `SVGImageElement` have no `decode()`.
- Animated images (GIF, APNG, animated WebP) do not decode the next frame ahead, have no `allowsAnimation` override or system animation controls, and no controls for suspended documents or off-screen views.
- Images are sRGB only: ICC profiles are ignored (FIXME in `PNGImageDecoder.cpp`), and there is no HDR, headroom or gain map support. `NativeImage` has no colour spaces.
- PDF images are not supported (FIXME in `Image.cpp`).
- Orientation is applied to the pixels, not by drawing; density-corrected sizes are not applied (FIXME in `BitmapImageSource.cpp`). `GraphicsContext`'s default `DrawImageBuffer` and `DrawPattern` for image buffers draw nothing (FIXMEs in `GraphicsContext.cpp`: `nativeImageForDrawing`).
- `shape-outside: url()` does not use the image's alpha (see *Layout*).
- `<video>`: `HTMLVideoElement::VideoWidth()` and `VideoHeight()` are unimplemented, a poster has no events or renderer, and there are no video decoders.
- SVG images keep the light appearance and do not copy the embedding page's settings (generic font families, dark appearance), because image observers cannot reach them (FIXME in `SVGImage.cpp`). The page of an SVG image has no loader strategy, so it loads only `data:` URLs.
- `SVGImageElement` does not reload when its CORS setting changes. `SVGUseElement` loading another file does not carry the fetch mode, destination or sniffing policy (FIXMEs).
- Drags: no drag icon for a dragged image's file (`CreateDragImageIconForCachedImageFilename` returns none), the response's suggested filename is not parsed, colours cannot be dragged or dropped, and there are no text indicators or visible paths. `PasteboardImage` and `SelectionData` carry no image, so nothing prefers images' types when pasting.
- Krystal.UI's `FileLoaderStrategy` reads files synchronously on the main thread, and `FileStorageProvider` writes local storage on every change on the main thread (FIXMEs).

## SVG

### Bugs

- Positioned boxes inside an SVG `<foreignObject>` lay out in it, but their layers are not painted and are hit tested at their page position instead of the foreign object's. Test: `SVGForeignObjectElement: positioned content paints and is hit` (`[!shouldfail]`).
- `RenderSVGText` and `LegacyRenderSVGRoot` compute their decorated bounding box by delegating to the accurate repaint rect (upstream's FIXMEs).

### Missing

- `zoomAndPan="disabled"` is not respected in standalone SVG documents (FIXME in `LocalFrame.cpp`), and user zooming and panning of a standalone SVG document is not done (FIXMEs in `EventHandler.cpp`).
- Filters on SVG shapes use the border box for the filter region and source image rect rather than the object bounding box (FIXMEs in `RenderLayerFilters.cpp`). Attribute parse errors (`ReportAttributeParsingError`) and filter warnings are not reported, because there is no console.
- SMIL has no throttled frame delay and updates outside an `EventQueueScope` (see *Animations*).
- `buffered-rendering: static` is not supported (FIXME in `LegacyRenderSVGImage.cpp`). `setMayHaveRenderedSVGForeignObjects` is not called (FIXME in `SVGForeignObjectElement.cpp`), as its only reader, composited scrolling, is absent.
- The inline formatting context's SVG text path (`Settings::UseIFCForSVGText`, off as upstream) compiles but is untested.
- SVG `<a>` has no `title` override (`xlink:title`) and no focus overrides (`supportsFocus`, `isMouseFocusable`, `isKeyboardFocusable`, `defaultTabIndex`); `SVGStyleElement` has no load event timer.

## MathML

### Missing

- Negative leading and trailing operator spaces, negative `lspace` on `<mpadded>` and negative `<mspace>` widths are not implemented (FIXMEs). Operator spacing is not applied to the whole embellished operator, and the dictionary does not accept multi-character operators.
- Percentages in some MathML lengths are not resolved (FIXMEs in `RenderMathMLBlock.cpp`). Floats are neither positioned nor cleared because `display: math` is unimplemented.
- `<menclose>`'s over and under bar thicknesses are not read from the MATH table (FIXME).
- An operator whose assembly font is missing returns without painting.

## Canvas

### Missing

- Without a host device (Booey's tests, or a view before its first paint), `getImageData` and other readbacks return transparent black, and `createPattern` of a canvas throws `InvalidStateError`.
- Drawing is sRGB with 8 bits a channel. `display-p3` is accepted and drawn as sRGB; `rgba-float16` `ImageData` reads and writes 8 bits a channel (no `Float16Array`); `PixelFormat` has RGBA8, BGRX8 and BGRA8 only. The `alpha` context setting is accepted but ignored (the context is always transparent).
- `willReadFrequently` is stored but changes nothing: there is no unaccelerated backend to read from (FIXME in `CanvasBase.cpp`).
- `<video>`, `ImageBitmap` and `OffscreenCanvas` are not `CanvasImageSource`s. `createImageBitmap`, `OffscreenCanvas`, `ImageBitmapRenderingContext`, `PaintRenderingContext2D`, WebGL and WebGPU contexts do not exist, and there are no context loss events.
- `getImageData` throws no `SecurityError` and canvases and patterns are always origin-clean (FIXME).
- An image drawn without a renderer uses its own orientation, not its computed style's (FIXME).
- No console messages are produced for failed canvas reads or for saves past the state stack limit.
- A canvas copy and every readback wait for the GPU.
- Image buffers of a resolution scale other than 1 replay their transforms unscaled (FIXME on Krystal.UI's `ImageBufferRender2DBackend`); canvases' buffers are always scale 1.
- `letterSpacing` and `wordSpacing` reject `calc()` lengths, as upstream's do.

## Editing and selection

### Bugs

- Composition is not cancelled when the selection changes in ways that do not go through `RespondToChangedSelection()`, nor when the selection becomes `NoSelection` (FIXMEs in `Krystal.UI/src/EditorClient.cpp`).
- `Pasteboard::CanSmartReplaceContents()` is false for the system clipboard, because the smart-paste type is neither written nor read; smart replace only works within a `SelectionData` carried by a drag.
- `input` is dispatched straight away rather than through a scoped event queue, so typing commands that are applied again fire it from within the command (FIXME in `Editor.cpp`).

### Missing

- `execCommand()` and `queryCommandState()`/`queryCommandValue()` are not implemented; `Document` only declares the IDL in a comment. `EditorCommand` has 86 selection, navigation, delete, insert, clipboard and undo/redo commands but no state or value for them.
- Formatting commands are absent: bold, italic and the rest of the `styleWithCSS`/`-webkit-font-size-delta` family, lists, indent and outdent, format block, links, remove format. Tab spans, mail blockquotes and `quoteFragmentForPasting` are absent with them. The `MakeTextWritingDirection*` commands apply styles but report no state.
- Paste does not render the fragment in a staging document to take its text as rendered and drop invisible content (`ReplacementFragment`); it does not remove event handler attributes or `javascript:` URLs from pasted markup (FIXMEs in `ReplaceSelectionCommand.cpp`). `MarkupAccumulator` does not resolve URL attributes against the base URL, and the markup sanitiser's staging page has no dark appearance.
- Bidi-aware pasting (matching each pasted paragraph's direction) and the bidi contiguity helpers (`AdjustToVisuallyContiguousRange` and relatives) are absent.
- `EditorClient` is only the selection, clipboard permission, undo/redo, composition and text-field hooks. It has no `shouldInsertText`/`shouldDeleteRange`/`shouldBeginEditing`/`shouldApplyStyle` delegates, so the host cannot veto or observe edits, and no spelling, grammar or text-checking, autocorrection or autofill hooks.
- Spelling and grammar, document markers (so `::spelling-error` and `::grammar-error` highlights are never painted), dictation and Writing Tools are absent.
- The kill ring and mark, find in page, the drag caret, the overtype block cursor, caret browsing's focusing of the anchor the caret is in, stopping the blink while typing, beeping on empty cuts and deletes, and spatial navigation's use of the caret are absent.
- Taps do not place the caret at the nearest word boundary (`WordBoundaryForPositionWithoutCrossingLine`; FIXME in `EventHandler.cpp`).
- Image documents, image overlays and writing images to the pasteboard are absent. Link decoration filtering and the `application/x-moz-file` quirk are not implemented.
- Text extraction and app highlights are not implemented (for text fragments, see *Text and fonts*).
- Script access to the clipboard (`javaScriptCanAccessClipboard`, `domPasteAllowed`) is off (FIXMEs in `EditorCommand.cpp`). Password fields are kept off the clipboard.
- Input methods are IMM32 only. TSF features (reconversion, inline handwriting, speech), custom highlights and annotations (inline predictions, writing suggestions), `inputmode`, `ime-mode` and an on-screen keyboard are not supported. The page draws no candidate list and the host is not told the candidates; the IME draws its own windows.
- Undo does not cancel a composition (upstream does this only on iOS).
- Context menus are not built: only the `contextmenu` event is dispatched, and there is no keyboard (menu key) route to it.
- Dragging a selection out to other applications is not supported.

## Input and events

### Bugs

- `mouseenter`/`mouseleave` (and `pointerenter`/`pointerleave` in `PointerCaptureController`) are dispatched to every element entered or left, not only those with a listener or a capturing ancestor listener, which is slow on deep trees (FIXMEs in `EventHandler.cpp` and `PointerCaptureController.cpp`).
- Dead keys are reported with key `Unidentified`: `input::Describe` does not use the platform layer's `KeyText::DeadKey` mark (FIXME in `Krystal.UI/src/Input.cpp`).
- `PointerCaptureController` leaves `targetOverride` set after releasing a capture (an upstream bug, FIXME in `PointerCaptureController.cpp`).
- The hovered element does not follow content under a still mouse until the fake mouse move after a scroll fires, 100 ms later (250 ms when moves are slow), as upstream. Fake moves use the last mouse event's modifiers and do not check page visibility or focus (FIXME in `EventHandler.cpp`).
- The XInput gamepad backend never reports the Guide button (it stays released).
- Drag events' modifier keys are always none: `DragController` makes their mouse event without keys (FIXME in `DragController.cpp`).

### Missing

- The window gets no `focus`/`blur` events: `FocusAlgorithms` has FIXMEs for firing them at a document's `Window`, and `FocusEvent`/`UIEvent` have no `view`. `FocusOptions` has no selection restoration mode or focus direction.
- Keys the platform layer has no code for: Insert, Print Screen, Pause, Scroll Lock, Num Lock and the context menu key (FIXME in `Win32Window.cpp`). There is no keyboard route to `contextmenu` (`sendContextMenuEventForKey`), and no menu controller beyond the `contextmenu` event.
- Access keys: `HTMLElement::AccessKeyAction` exists but nothing calls it (FIXME in `EventHandler.cpp`).
- `screenX`/`screenY` on mouse, pen and touch events are not meaningful: the host does not report where the page is on the screen (FIXMEs in `View.cpp` and `Input.cpp`).
- The window size ignores DPI (`Win32Window.cpp`, "Account for dpi?").
- Click counting (500 ms, 2 px) and the touch gesture thresholds (tap, pan, long press, fling, pinch) are fixed constants in `View` rather than the system's double click time and distance and touch settings (FIXMEs in `View.hpp`).
- Custom cursors (`cursor: url()`) are absent (`Cursor` has no images; FIXME in `EventHandler::SelectCursor`), as are pan-scrolling and auto-hiding cursors.
- Wheel: `PlatformWheelEvent` has no phases, velocities, precise deltas or inverted directions, so there is no scroll latching, wheel delta filter or precision touchpad gesture support (pinching, swiping, DirectManipulation). Pan gestures chain on to the next scroll box mid-gesture, as the wheel does.
- Pan scrolling (middle-button autoscroll) and its icon are absent; selection autoscroll works.
- Drag and drop: colour drags and drops, plugin documents and image overlays are absent. Touch and pen drags are not done (pointer events cancel as upstream when a drag starts), and dropped URLs load as user-input requests without an external URL policy.
- Dragging out of a page to other applications is not supported: `DoDragDrop` is a modal loop that would block the frames, so a drag that starts in a page stays in it.
- Sequential focus navigation does not handle frames, popovers as scope owners or moving the focus out to the host, and wraps at the end of the document. Focusable areas do not include scrollable regions, nested navigables or draggable elements (FIXMEs in `FocusAlgorithms.cpp`).
- Spatial navigation is upstream's only (the focus candidate search, off by default through `Settings::SpatialNavigationEnabled`); the CSS Spatial Navigation draft (`navbeforefocus`, `navnotarget`, `spatialNavigationSearch()`, `focusableAreas()`, `spatial-navigation-*`) is not implemented.
- Keyboard: caps lock handling in text fields, Escape hiding validation messages and leaving fullscreen, the backward-compatibility mode of `charCode` (`needsKeyboardEventDisambiguationQuirks`) and `getCurrentModifierState` are FIXMEs. The default keyboard handler doesn't handle Backspace (history navigation; FIXME).
- Touch: documents without touch event listeners aren't skipped (`hasTouchEventHandlers` is a FIXME), and touch event emulation from the mouse is absent. Touch `pointermove` events do not carry the delta since the last event.
- Pointer lock: the sandbox flag check (`SecurityError`) is a FIXME. The window side of pointer lock and gamepads is untested (it needs a real cursor or device).
- Gamepad: XInput only (Xbox-style pads, four at most, polled), with no GameInput, SDL or Windows.Gaming.Input backend. There is no `trigger-rumble`, no extensions (`GamepadPose`, touches, lights), no `KnownGamepad` readers and no permissions-policy `SecurityError`.
- Hit testing ignores child frames. `HitTestResult` queries about what was hit (selection, images, media, titles, editing, link targets) are FIXMEs. `RenderLayer` hit testing lacks view transitions, SVG resources, fixed container sampling, resizers and transformed layers in paginated layers, and the visual viewport is not applied to the hit test area.
- `IsDraggableLink` is a local check (an HTML link with `href`, not editable); the editable-link-behaviour setting is absent.
- Page overlays, subframes and synthetic touch events are FIXMEs in `EventHandler`.

## Loading, navigation and history

### Bugs

- `FrameLoader` does not fire `visibilitychange`, and a document left or cached keeps its visibility state (FIXMEs in `FrameLoader.cpp` and `CachedPage.cpp`).
- `ScheduledFormSubmission` does not count as a quick redirect, so a pending submission does not stop a page being cached.
- `Navigation` has no `sourceSnapshotParams` (FIXME in `Navigation.cpp`), does not consume history-action user activation on a traversal, and does not set destination state. `NavigationHistoryEntry` does not hide another document's entry URL for its referrer policy.
- The unloaded document keeps its event listeners (upstream removes them).
- `Document::ShouldScheduleLayout` does not wait for the view to be visually non-empty (FIXME in `Document.cpp`), and a deferred `SetNeedsLayout` is never made to happen (upstream's FIXME in `LocalFrameViewLayoutContext.cpp`).

### Missing

- Remote loading: there is no network loader, cookies, origins, CORS, referrers, pings, preloads or integrity checks; the host pushes data in through the loader strategy. `navigator.cookieEnabled` is false.
- `DocumentLoader` has no redirects, content policies (responses shown as downloads), archives or streaming; the main resource arrives whole on a networking task. `POST` bodies do not reach the loader strategy, so a reload or history traversal cannot re-post one. There is no progress tracker, and HTTP status checks, `X-Content-Type-Options: nosniff` and content type parsing (`ParsedContentType`) are absent because responses carry no headers or statuses.
- `DocumentWriter` takes HTML in UTF-8 only (BOMs removed, split sequences kept, invalid bytes replaced), with no other encodings or document types; form `accept-charset` is not honoured.
- `NavigationScheduler` has no page blocks, client redirect notifications or user gesture forwarding, and no sandboxed automatic features flag. The only pragmas are `refresh`, `content-language` and `default-style`; the `Content-Language` header is ignored.
- Prerendering, speculation rules, `javascript:` URLs, `pagereveal`/`pageswap` and view transitions are not implemented. Icons, manifests and alternate style sheet sets chosen by script are not loaded.
- No DOM `URL` interface or `URLSearchParams` (`url::URL` is internal), so WPT `setters_tests.json` is not run.
- There is no global history or visited link store, so `:visited` matches only links with an empty `href` and `VisitedLinkState` has nothing to ask. History items hold no document title (documents do not tell their loader when the title changes), failed-visit flag, form data in requests, page scale factor, platform view state or policy container. There are no deferred loads, multipart loads, unreachable URLs or child frame entries.
- Back/forward cache eligibility does not check `Cache-Control: no-store`, `Clear-Site-Data`, visually empty pages, error pages or memory pressure.
- `History` has no cached script value for the state and no origin checks; the quota counts only the URL (FIXME in `History.cpp`).
- `Window`: `print()` does not wait for the document to finish loading, there are no message ports or transfers, `postMessage()` only reaches the window itself, `screen.orientation` is absent, reported errors carry no script location, and prompts cannot be shown without a page. There is no sandboxed modals flag. Timers are not throttled or aligned, and animation frames are not throttled below the page's rate.
- Fragment navigation does not reset scroll anchoring (there is none) and has no text indicator for a text fragment (FIXME in `LocalFrameView.cpp`).
- `Page` has no rendering update scheduler: the host runs a scheduled update at its next frame (FIXME in `Page.cpp`). Revealing documents, fullscreen and view transitions steps of the update are FIXMEs, and `Document` has no post-layout task timer.
- Frames: there is no frame loader for subframes and no frame tree. Sequential navigation, hit testing and painting skip frames.

## Core and infrastructure

### Bugs

- Krystal.Core's `IntrusivePtr` `Downcast`/`DynamicDowncast` overloads can't take an explicit target type, and `Downcast` asserts on null; `OffsetRotateValue.cpp` works around it (FIXME).
- Some of Krystal.Core's text tests aren't built: `tests/Text/Unicode.cpp`, `EncodingRegistry.cpp` and `Encodings/ASCII.cpp` are missing from `Krystal.Core.Tests/CMakeLists.txt`.

### Missing

- **No console.** Exceptions thrown by listeners, parse errors, and failed custom element constructions and upgrades have nowhere to be reported; only `reportError()` reaches the host, through `ChromeClient::AddMessageToConsole`.
- **No garbage collector.** Listener callbacks that capture their own target keep it alive.
