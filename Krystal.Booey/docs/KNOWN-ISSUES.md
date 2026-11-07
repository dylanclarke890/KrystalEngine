# Known issues

Known bugs and missing features in Krystal.Booey, by area. A bug with a `[!shouldfail]` test names it; most entries also have a `// FIXME:` where the fix goes. Remove an entry when it's fixed, and add one for a bug that's found but not fixed, with a `[!shouldfail]` test when one can be written.

What's out of scope by choice (scripting, the network, frames, workers, accessibility and the rest) is in `OVERVIEW.md` and isn't repeated here. Where Booey deliberately differs from a specification or Chrome, it's in `DIFFERENCES.md`. Issues in Render2D and Krystal.UI are in `Krystal.Gfx/docs/OVERVIEW.md` and `Krystal.UI/docs/OVERVIEW.md`.

## DOM

### Missing

- Responses (`ResourceResponse`) have no HTTP headers, so `document.lastModified` is always the current time, the specification's fallback when the date is unknown (it needs `Last-Modified`, or a file's modification date for file loads; FIXME in `Document.cpp`), and a language from a `Content-Language` header is not read (only the `<meta>` pragma is; FIXME in `SelectorCheckerTestFunctions.cpp`).
- `title` is never shown: there's no advisory information, no `HitTestResult::Title`, no tooltip hook on the chrome client, and Krystal.UI draws no tooltips.
- No Fullscreen API (`:fullscreen`).
- No Event Timing or Long Animation Frames timing: inner invoke doesn't record timing info for event listeners, and firing an event doesn't record task time (FIXMEs in `EventDispatcher.cpp` and `EventDispatcher.hpp`).
- No customized built-in elements: definitions have no `extends`, and an element's `is` value is kept (cloned, serialised) but never looks up a definition. Elements created with an `is` value are uncustomized.
- `[CEReactions]` scopes are missing on members that aren't implemented yet (`execCommand`, `nonce`).
- `ElementInternals` has no ARIA reflection members (`detail::GetContentAttribute` in `ReflectAttribute.hpp` returns null for them) and no `CustomElementDefaultARIA`.
- `DocumentTouch` (`createTouch`, `createTouchList`) is not implemented; `Touch` has no constructor and there is no `TouchInit`.
- `DataTransferItem` has no `getAsEntry` (no file system API).
- No `FileReader`, streams, blob URLs or blob registry. A `Blob` holds its data itself (`BlobData`), and `Blob::Bytes()`/`Text()` read synchronously, returning the exception a failed read rejects with (`NotFoundError` for a missing file, `NotReadableError` otherwise).
- A task's document is not checked for being fully active (documents never in a frame, such as `createHTMLDocument()`, run their tasks). Subresource loads, `close()`, the scroll to the anchor and `AbortSignal.timeout()` are not tagged with a document, so suspension does not hold them.
- Suspension covers only the document's timers and animation frame callbacks: no media, font loading, service workers or IndexedDB, and animations aren't suspended (FIXME in `WebAnimation.hpp`).
- `FirstChild()`, `ParentNode()` and friends return raw pointers rather than `RefPtr`; `NodeFilter` is clunky from C++.

## HTML elements and forms

### Missing

- `<input type=file>` has no file chooser, file icon (`icon()`, `requestIcon`, `iconLoaded`) or tooltip naming the chosen files; the host's file picker is a FIXME in `FileInputType.hpp`. The name uses the room the icon would take. Files are set from C++ (`FileInputType::SetFiles`, which fires `input` and `change` unless set by script) or dropped; there are no directory uploads.
- `<datalist>`: `HTMLInputElement` has no `list` (FIXME), so there are no list suggestions, tick marks on sliders, list button, or colour and date suggestions.
- `<input type=color>` has no `alpha` or `colorspace` support (FIXME in `ColorInputType.hpp`) and no `EventQueueScope`, so its events aren't queued until the value is set (FIXME in `ColorInputType.cpp`).
- Opening a date or colour picker by clicking the control isn't checked against a user gesture (`showPicker()` is); FIXMEs in `BaseDateAndTimeInputType.cpp` and `ColorInputType.cpp`.
- No autofill (`:autofill`), caps lock indicator or strong password fields on password fields, validation message bubble, `RenderSearchField` (search fields have no recent searches), or input purpose hints (`inputmode`, `autocapitalize`, spell checking). Numbers aren't localised (FIXMEs in `NumberInputType.cpp`); dates are, through ICU.
- Form submission has no `rel=noreferrer` handling (FIXME in `HTMLFormElement.cpp`).
- Customizable `<select>`: `appearance: base-select` on the select alone gives a picker with base appearance, where without `::picker(select)` opting in too it should be the native one, as `RenderMenuList` owns the popup menu. List boxes ignore `appearance: base-select` and stay native. Type-ahead with the picker open changes the selection, where it should move the focus. Page Up and Page Down move ten options, not a viewport's worth. A `legend` in an `optgroup` isn't its label in a native menu.
- `<input type=image>` doesn't load or show its image (FIXMEs in `ImageInputType.cpp`), so its dimensions come only from its box or its `width` and `height` attributes.
- `<img>` lacks `decode()` and the `Image()` constructor (FIXMEs in `HTMLImageElement.hpp`), and `sizes=auto` (auto sizes for lazy images: `auto` is an invalid size), and doesn't select a source again when media queries change (FIXME in `HTMLImageElement.cpp`). `HTMLImageLoader` fires no events for video posters and has no `<object>` fallback (FIXMEs); `ImageLoader`'s reuse of available images has no preloads (FIXME).
- `HTMLLinkElement` recognises only `rel=stylesheet` and `alternate`: no `expect` (internal resource links that block rendering until an element is parsed; FIXME in `HTMLLinkElement.cpp`), preloads, icons, manifests, integrity, referrer policies or priorities (no `LinkRelAttribute`/`LinkLoader`).
- `<a ping>` and referrers are not implemented; the `download` attribute is only passed to the host. `<object usemap>` is not supported, and `<object>` and `<embed>` don't display their resources (no images, documents or plugins), so an object always shows its fallback content.
- `<marquee>`'s `hspace`, `vspace`, `loop`, `scrollAmount`, `scrollDelay`, `trueSpeed`, `start()` and `stop()` are not implemented, and marquees don't scroll (FIXME in `HTMLMarqueeElement.hpp`).
- Popovers: no `interestfor` and no `:open`.
- `MIMETypeRegistry::MIMETypeForExtension` has only a built-in table; most other extensions have no type (FIXME in `MIMETypeRegistry.cpp`).
- Slider thumbs can't be dragged by touch: a touch pan scrolls.

## CSS parsing and CSSOM

### Missing

- Typed OM has only numeric values and `CSSKeywordValue` (`CSS/TypedOM/`). There is no `CSSStyleValue::Parse()`/`ParseAll()`, `CSSStyleValueFactory`, `StylePropertyMap` (`styleMap`, `attributeStyleMap`, `computedStyleMap()`), `CSSUnparsedValue`, image, transform or colour values. `CSSNumericFactory` is a struct of static functions (`CSSNumericFactory::Px(10)`) and there is no `CSS` namespace object.
- No `CSS.registerProperty` or `CSS.supports`. `CSSStyleSheet` has `ReplaceSync` only; the async `replace()` needs promises.
- `SingleAnimationRangeStart/End::ToTimelineRangeValue()` returns a null `CSSNumericValue` for a calculated offset (FIXME).
- The `FontFace` string parsers (`ParseFontFaceSrc`, `ParseFontFaceFontWeight` and the rest, FIXME in `Consumers/Font.hpp`) aren't written.
- `@font-face`'s features aren't applied to loaded fonts (FIXME in `FontCustomPlatformData.cpp`). `@font-palette-values` and `@font-feature-values` are parsed but have no effect on fonts (FIXME in `CSSFontSelector.cpp`).
- `::part()` doesn't follow `exportparts`: part names match only in the host's own scope (FIXMEs in `SelectorChecker.cpp` and `ElementRuleCollector.cpp`).
- State pseudo-classes that match nothing, with FIXMEs in `SelectorChecker.cpp`: `:autofill`, `:fullscreen`, `:picture-in-picture`, the media playback pseudo-classes, `:future`/`:past`. `:visited` matches only links with an empty `href` (see *Style*).
- `:lang()` has no document default language or Content-Language header (FIXME in `SelectorCheckerTestFunctions.cpp`); WebVTT elements don't use their cue's language.
- `SelectorQuery` has no ID fast paths (rightmost-ID lookup, filtering the root by an ancestor ID, `[id=...]`; FIXMEs until tree scopes map IDs).
- No user or extension style sheets: `ScopeRuleSets::InitializeUserStyle` adds nothing and `Style::Scope` has no injected author sheets (FIXMEs), and user style sheets' `@import`s and load options aren't handled.
- `RuleSet` doesn't clone `::cue` rules for the cue background box (no WebVTT cues).
- `SizesAttributeParser` doesn't clear the font selector for font-relative units (FIXME).
- `css::calc::Value::DoubleValueDeprecated` doesn't log when the tree needs conversion data (FIXME in `ValueEvaluation.cpp`).
- There is no inspector: `:hover` can't be forced and there are no observer hooks for source ranges.

## Style (cascade and computed values)

### Missing

- `env()` has no values: without the document's `ConstantPropertyMap` an `env()` reference falls back or is invalid at computed-value time (FIXME in `VariableReferenceValue.cpp`).
- `::cue` rules for WebVTT elements aren't collected (FIXME in `ElementRuleCollector.cpp`).
- Visually ordered documents aren't applied in `ResolveForDocument` (FIXME).
- No visibility adjustment (`VisibilityAdjustment`): `adjustVisibilityForPseudoElement` and the force-hidden subtree adjustment are FIXMEs.
- `Adjuster` leaves SVG transforms not stored on the style, and the layer-based SVG engine's `z-index`/stacking branches, as FIXMEs.
- `Style::Extractor` has no branch for properties with running keyframe effects (FIXME).
- `Style::PendingResources` has no user agent shadow tree support and no anonymous mode for masks (FIXME).
- Anchor positioning has no fragmented flows: an anchor's rect isn't the bounding box of its fragments (FIXMEs in `AnchorPositionEvaluator.cpp`). Anchor-positioned viewport-constrained objects need a scrolling coordinator Booey doesn't have.
- The switch control sheet (`htmlSwitchControl.css`) and the fullscreen sheet are never loaded (FIXMEs in `UserAgentStyle.cpp`).
- `DidChangeViewportSize` doesn't visit pseudo-elements, as they aren't nodes (FIXME in `Scope.cpp`).
- `RenderStyle::HyphenString()` gives the hyphen-minus for `hyphenate-character: auto` (FIXME: U+2010 when the primary font has the glyph, locale dependent).

## Animations

### Missing

- Scroll and view timelines have no accelerated representation (no threaded animations), and a change in view timeline metrics doesn't update the timeline (FIXME in `KeyframeEffect.cpp`).
- Keyframe offsets in named timeline ranges compute to NaN without a view timeline.
- `TreeResolver` skips the back/forward cache and printing checks on the animated-update path, and has a FIXME to push after-change style into the parent stack.
- Image animations pause out of view and resume (`RenderView::AddRendererWithPausedImageAnimations`), and there is no per-element animation control (`RenderElement::AllowsAnimation` follows only the page's `SetImageAnimationEnabled`; FIXME).
- Extent computation for animated motion path (`offset-*`) properties is missing (FIXME in `KeyframeEffect.cpp`).
- `onbegin`, `onend` and `onrepeat` attributes on SMIL elements don't exist.
- Quirks that prevent acceleration (`shouldPreventKeyframeEffectAcceleration`) aren't implemented (FIXME).

## Layout

### Missing

- An unstyled XML document (one with no style sheets, hints or HTML, SVG or MathML elements) is rendered as its text, not as a view of its source or tree.
- There is no multi-column layout or fragmentation: no column renderers, column rules, fragmented flows or pagination. `collectFragments` makes one fragment, paginated layers paint unfragmented, and out-of-flow boxes, `TextBoxTrimmer`, `RenderView::NodeForHitTest`, the highlight range iterator and repaint containers have no column handling (FIXMEs). Multi-column flows don't lay out `content-visibility` skipped content (FIXME in `RenderBlockFlow.cpp`).
- Image-based `shape-outside` is not supported: `ShapeOutsideInfo` builds only box, rectangle and polygon shapes (no raster shape, no `shape-image-threshold`), and shapes have no display paths for highlighting (FIXMEs in `LayoutShape.hpp` and `ShapeOutsideInfo.cpp`).
- `text-combine-upright` is not implemented: there is no combined-text renderer (`RenderCombineText`), so `BoxTreeUpdater`, the inline iterators, `RenderBlockFlow` and `RenderText` treat combined text as ordinary text (FIXMEs).
- `layout_integration::GridLayout::CanUseFor` accepts only single-track spans at explicit unnamed lines and the self-alignments `start`, `flex-start`, `self-start`, `normal` and `stretch`; other grids fall back to `RenderGrid`. Multi-column spans in `ImplicitGrid` are a FIXME.
- `RenderTreeAsText` leaves out SVG paint features and visual overflow, and file upload control names (FIXMEs).

## Painting

### Bugs

- A dashed rounded border with different widths on each side has its dashes drift from Chrome's along the sides, by a few pixels (FIXME in `BorderPainter::DrawBoxSideFromPath`).
- Dashed collapsed table borders share out the junctions between cells slightly differently from Chrome: an edge after a junction starts a few pixels off, so its gaps differ a little (the `tables` and `dashed-borders` goldens).
- A mask's `color-interpolation: linearRGB` is drawn in sRGB (`LegacyRenderSVGResourceMasker`, FIXME), because image buffers have no colour space conversion.
- `Path::ApplyElements` applies nothing for continuous rounded rects, and `Path::Transform` drops them, as there's no platform path (FIXMEs in `Path.cpp`); nothing in Booey makes one yet.
- A composited mask's or clip path's edge pixels (and those of a turned masked layer) can differ slightly from the whole-page repaint.

### Missing

- The compositing operators `darker` and `difference` are not mapped: `CompositeOperator` falls back to source-over (FIXME in Krystal.UI's `GraphicsContextRender2D.cpp`).
- There's no software filter path: filters are applied by the host's accelerated backend, or by the graphics context for the ones it can draw. Reference filter clients and SVG shapes' filter regions are FIXMEs (see *SVG*), and `filter: url()` to an external document is not supported (see *Images and resources*).
- Backdrop filters are drawn only by the compositor: the flattened whole repaint (`View::Record`) doesn't draw them.
- `::backdrop` is missing from repaint and `Styleable::FromRenderer`.
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

- `TextureMapperAnimation.cpp` carries two inherited FIXMEs about suspicious handling: `_totalRunningTime` is cleared where it seems it should not be, and `Pause` takes a time offset where it is passed a time.

### Missing

- `GraphicsLayerTextureMapper` has no contents scaling filters, so composited images and texture canvases with `image-rendering: pixelated` or `crisp-edges` scale smoothly.
- There's no scroll anchoring, autoscroll timers beyond the scrollbar repeat, wheel latching, kinetic scrolling, touchpad gesture states or rubber-banding (FIXMEs in `Element`, `LocalFrameView`, `EventHandler`, `ScrollableArea`, `Scrollbar`). Scroll positions are whole pixels, so wheel deltas are rounded, and a smooth scroll moves in whole pixels.
- Scroll snapping without gesture phases: each wheel event goes to the next snap position, so a touchpad's small deltas page through a mandatory container. An autoscroll and a pan scroll don't snap, even when they end, until the next layout. Smooth scrolls aren't run for list boxes, whose keyboard and wheel scrolls stay immediate.
- There are no overlay scrollbars, native scrollbars, scroll corners or resizers. `scrollbar-color` has no effect, and `ScrollableArea` has no scrollbars controller.
- `VisualViewport` is the layout viewport at the page scale and never scrolls within it, so its `scroll` event never fires (its `resize` does). There are no obscured insets, auto-sizing, base viewport layout size scale factor (FIXME in `VisualViewport.cpp`) or layout viewport separate from the scroll position. Pinch zoom is only the page scale (1 to 5) from the view's gesture recogniser.
- There is no async scrolling or threaded compositing: no scrolling coordinator or tree, no reconciliation of the composited scroll position after user scrolls, and animations and scrolling run on the main thread. A sticky box stuck to the view therefore repaints its old and new rects on scroll instead of compositing.
- Only `transform` and `opacity` animations run on layers: `GraphicsLayerTextureMapper::AddAnimation` refuses the rest, so animations of filters, `backdrop-filter` and the individual transform properties run on the page.
- 3D rendering contexts do not exist: `preserve-3d` sorting and perspective are not done, and Render2D draws 3D transforms flattened (FIXMEs in `TextureMapper.cpp`, `TextureMapperLayer.cpp`).
- Reference (SVG `url()`) filters are identities on composited layers (`TextureMapper.cpp` converts the other filter functions to `LayerFilter2D`).
- Clip paths never composite as shape layers; a composited clip path is painted into the mask layer. Replicas (reflections), repaint counters, repeating contents tiles (`WrapMode::Repeat`) and creating tiles over several frames are FIXMEs.
- `TextureMapperLayer` composites the whole tree each frame, with no damage tracking.
- The compositor has no scrolling nodes, widget layers, video or plug-in layers, event or interaction regions, layer flush throttling, model or separated layers, or HDR and wide gamut backings (`ContentsFormat` is RGBA8 only). There is no `TiledBacking` (`SetIsInWindow` and `SetTileCoverage` do nothing), the fixed root background layer is never composited, and the host's compositing policy under memory pressure is a FIXME.

## Text and fonts

### Missing

- Fonts come only from the host and the bundled faces; there is no system font lookup or system fallback. `SystemFontDatabase::PlatformSystemFontShorthandInfo` returns the standard family at 16px, normal weight, for every system font keyword (`caption`, `icon`, `menu`, `message-box`, `small-caption`, `status-bar`).
- There are no hyphenation dictionaries, so `hyphens: auto` never hyphenates (FIXME in `Core/Hyphenation.hpp`).
- Colour glyphs and emoji presentation are not detected (FIXME in `FontHarfBuzz.cpp`), so `font-variant-emoji` only adds a variation selector to fallback clusters. `ResolveEmojiPolicy` cannot return `RequireText` or `RequireEmoji` (FIXME in `FontCascade.cpp`).
- Colour glyphs from bitmap strikes (`CBDT`) are drawn at their strike's size, unscaled, while COLR glyphs scale. Large colour glyphs stay bitmaps.
- `text-transform: full-width` does nothing (FIXME in `RenderText.cpp`).
- A backslash is never shown as a yen sign, because that depends on the font and the document encoding (FIXME in `RenderText.cpp`).
- `FontDescription::SetSpecifiedLocale` resolves every Han locale to `zh-hans`, as there are no user language preferences.
- `FontCache` has no memory-pressure limits; the host cannot report pressure (FIXME in `FontCache.cpp`). The purge of inactive fonts runs on a timer only.
- Right-to-left scripts other than Arabic and Hebrew (Syriac, Thaana, N'Ko and so on) have no bundled font and work only if the host registers one. Kashida justification is not done.
- Web fonts: SVG fonts, font collections (the item in a collection is not selectable), a font parsing policy (safe font parser, trusted types) and `FontFace`/`FontFaceSet` as an API from C++ are missing. Font loading has no settings: installed fonts are always allowed and load completions are never ignored.
- Text search (`findPlainText`, `SearchBuffer`) is absent. The text iterator does not traverse the flat tree, enter image overlays or emit text placeholders (FIXMEs in `TextIterator.cpp`).

## Images and resources

### Missing

- External reference filters (`filter: url(doc.svg#f)`) are not loaded or watched (FIXME in `RenderLayerFilters.cpp`). `cross-fade()` and filter images load their inputs but do not follow them as clients (FIXMEs in `StyleCrossfadeImage.cpp`, `StyleFilterImage.cpp`), so they do not update when an input loads, and `StyleCrossfadeImage::Blend` always returns null, so `cross-fade()` percentages don't interpolate.
- Style sheets have no `nosniff` or HTTP status checks (FIXMEs in `CachedCSSStyleSheet.cpp`).
- `CachedResource` has no preloads, priorities or streaming and partial data; a resource decodes only once all its data has arrived. `CachedResourceLoader` loads images, fonts and style sheets only; other types, request counting and loads that do not delay the load event are FIXMEs. `MemoryCache` has one LRU list. `SharedBuffer` is one contiguous immutable buffer.
- Images decode synchronously: there is no decoding work queue, so `DecodingMode::Asynchronous` decodes at once and its callbacks run before the call returns (FIXMEs in `BitmapImageSource.cpp`). `HTMLImageElement` and `SVGImageElement` have no `decode()`.
- Animated images (GIF, APNG, animated WebP) do not decode the next frame ahead, have no `allowsAnimation` override or system animation controls, and no controls for suspended documents or off-screen views.
- Images are sRGB only: ICC profiles are ignored (FIXME in `PNGImageDecoder.cpp`), and there is no HDR, headroom or gain map support. `NativeImage` has no colour spaces.
- PDF images are not supported (FIXME in `Image.cpp`).
- Orientation is applied to the pixels, not by drawing; density-corrected sizes are not applied (FIXME in `BitmapImageSource.cpp`). `GraphicsContext`'s default `DrawImageBuffer` and `DrawPattern` for image buffers draw nothing (FIXMEs in `GraphicsContext.cpp`: `nativeImageForDrawing`).
- `shape-outside: url()` does not use the image's alpha (see *Layout*).
- `<video>`: `HTMLVideoElement::VideoWidth()` and `VideoHeight()` are unimplemented, a poster has no events or renderer, and there are no video decoders.
- SVG images keep the light appearance and do not copy the embedding page's settings (generic font families, dark appearance), because image observers cannot reach them (FIXME in `SVGImage.cpp`). The page of an SVG image has no loader strategy, so it loads only `data:` URLs.
- Drags: no drag icon for a dragged image's file (`CreateDragImageIconForCachedImageFilename` returns none), the response's suggested filename is not parsed, colours cannot be dragged or dropped, and there are no text indicators or visible paths. `PasteboardImage` and `SelectionData` carry no image, so nothing prefers images' types when pasting.
- Krystal.UI's `FileLoaderStrategy` reads files synchronously on the main thread (FIXME).

## SVG

### Bugs

- Positioned boxes inside an SVG `<foreignObject>` are painted into its layer rather than composited, whatever their styles ask for, as compositing goes by the layer lists that leave the foreign object out.
- `RenderSVGText` and `LegacyRenderSVGRoot` compute their decorated bounding box by delegating to the accurate repaint rect (FIXMEs).

### Missing

- Filters on SVG shapes use the border box for the filter region and source image rect rather than the object bounding box (FIXMEs in `RenderLayerFilters.cpp`). Attribute parse errors (`ReportAttributeParsingError`) and filter warnings are not reported, because there is no console.
- `buffered-rendering: static` is not supported (FIXME in `LegacyRenderSVGImage.cpp`). `setMayHaveRenderedSVGForeignObjects` is not called (FIXME in `SVGForeignObjectElement.cpp`), as its only reader, composited scrolling, is absent.
- The inline formatting context's SVG text path (`Settings::UseIFCForSVGText`, off) compiles but is untested.
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
- An image drawn without a renderer uses its own orientation, not its computed style's (FIXME).
- A canvas copy and every readback wait for the GPU.
- Image buffers of a resolution scale other than 1 replay their transforms unscaled (FIXME on Krystal.UI's `ImageBufferRender2DBackend`); canvases' buffers are always scale 1.
- `letterSpacing` and `wordSpacing` reject `calc()` lengths.

## Editing and selection

### Bugs

- `Pasteboard::CanSmartReplaceContents()` is false for the system clipboard, because the smart-paste type is neither written nor read; smart replace only works within a `SelectionData` carried by a drag.

### Missing

- `execCommand()` and `queryCommandState()`/`queryCommandValue()` are not implemented; `Document` only declares the IDL in a comment. `EditorCommand` has 86 selection, navigation, delete, insert, clipboard and undo/redo commands but no state or value for them.
- Formatting commands are absent: bold, italic and the rest of the `styleWithCSS`/`-webkit-font-size-delta` family, lists, indent and outdent, format block, links, remove format. Tab spans are absent with them. The `MakeTextWritingDirection*` commands apply styles but report no state.
- Paste does not render the fragment in a staging document to take its text as rendered and drop invisible content (`ReplacementFragment`); it does not remove event handler attributes or `javascript:` URLs from pasted markup (FIXMEs in `ReplaceSelectionCommand.cpp`). `MarkupAccumulator` does not resolve URL attributes against the base URL, and the markup sanitiser's staging page has no dark appearance.
- Bidi-aware pasting (matching each pasted paragraph's direction) and the bidi contiguity helpers (`AdjustToVisuallyContiguousRange` and relatives) are absent.
- `EditorClient` is only the selection, clipboard permission, undo/redo, composition and text-field hooks. It has no `shouldInsertText`/`shouldDeleteRange`/`shouldBeginEditing`/`shouldApplyStyle` delegates, so the host cannot veto or observe edits, and no spelling, grammar or text-checking, autocorrection or autofill hooks.
- The drag caret, the overtype block cursor, caret browsing's focusing of the anchor the caret is in, stopping the blink while typing, beeping on empty cuts and deletes, and spatial navigation's use of the caret are absent.
- Image documents and writing images to the pasteboard are absent. Link decoration filtering and the `application/x-moz-file` quirk are not implemented.
- Script access to the clipboard (`javaScriptCanAccessClipboard`, `domPasteAllowed`) is off (FIXMEs in `EditorCommand.cpp`). Password fields are kept off the clipboard.
- Input methods are IMM32 only. TSF features (reconversion, inline handwriting, speech), custom highlights and annotations (inline predictions, writing suggestions), `inputmode`, `ime-mode` and an on-screen keyboard are not supported. The page draws no candidate list and the host is not told the candidates; the IME draws its own windows.
- Undo does not cancel a composition.
- Context menus are not built: only the `contextmenu` event is dispatched, and there is no keyboard (menu key) route to it.
- Dragging a selection out to other applications is not supported.

## Input and events

### Missing

- While the page doesn't have system focus, `focus()` still fires `focus` and `focusin` at the element, and they fire again when the page gets system focus. Chrome only moves the focused element then, and fires them once, when the page gets system focus.
- There is no keyboard route to `contextmenu` (`sendContextMenuEventForKey`), and no menu controller beyond the `contextmenu` event.
- Custom cursors (`cursor: url()`) are absent (`Cursor` has no images; FIXME in `EventHandler::SelectCursor`), as are auto-hiding cursors.
- Wheel: `PlatformWheelEvent` has no phases, velocities, precise deltas or inverted directions, so there is no scroll latching, wheel delta filter or precision touchpad gesture support (pinching, swiping, DirectManipulation). Pan gestures chain on to the next scroll box mid-gesture, as the wheel does.
- Drag and drop: colour drags and drops are absent. Touch and pen drags are not done (pointer events cancel when a drag starts).
- Sequential focus navigation does not handle moving the focus out to the host, and wraps at the end of the document. Focusable areas do not include scrollable regions or draggable elements (FIXMEs in `FocusAlgorithms.cpp`).
- Keyboard: caps lock handling in text fields, Escape hiding validation messages, the backward-compatibility mode of `charCode` (`needsKeyboardEventDisambiguationQuirks`) and `getCurrentModifierState` are FIXMEs.
- Touch: touch event emulation from the mouse is absent. Touch `pointermove` events do not carry the delta since the last event.
- `RenderLayer` hit testing lacks SVG resources, fixed container sampling and transformed layers in paginated layers, and the visual viewport is not applied to the hit test area (FIXMEs in `RenderLayer.cpp`).
- `IsDraggableLink` is a local check (an HTML link with `href`, not editable); the editable-link-behaviour setting is absent.
- Page overlays and synthetic touch events are FIXMEs in `EventHandler`.

## Loading, navigation and history

### Missing

- Responses can't be streamed: the main resource arrives whole on a networking task, and the loader strategy has no cancellation or synchronous loads (FIXME in `LoaderStrategy.hpp`).
- Icons (`<link rel=icon>`) are not loaded or handed to the host.
- Animation frames are not throttled below the page's rate (FIXME in `ScriptedAnimationController.cpp`), and scroll events are not throttled (FIXME in `LocalFrameView.cpp`).
