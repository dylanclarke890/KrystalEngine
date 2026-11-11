# Known issues

Known bugs and missing features in Krystal.Booey, by area. A bug with a `[!shouldfail]` test names it; most entries also have a `// FIXME:` where the fix goes. Remove an entry when it's fixed, and add one for a bug that's found but not fixed, with a `[!shouldfail]` test when one can be written.

What's out of scope by choice (scripting, the network, frames, workers and the rest), and what's planned for later (video, accessibility and the rest), is in `OVERVIEW.md` and isn't repeated here. Where Booey deliberately differs from a specification or Chrome, it's in `DIFFERENCES.md`. Issues in Render2D and Krystal.UI are in `Krystal.Gfx/docs/OVERVIEW.md` and `Krystal.UI/docs/OVERVIEW.md`.

## Threads

### Missing

- Every page has to run on the same thread: there are no agents, so what the specifications keep per agent is process-wide, as is much else. That includes the event loop (`html::EventLoop::Current()`) and its timers (`ThreadTimers`), the mutation observer flag, pending observers and signal slots (`MutationObserverAlgorithms`), and the backup element queue of custom element reactions (`CustomElementReactionQueue.cpp`). It also includes the CSS value pool (`ValuePool::MainThreadPool`), Krystal.Core's string atom pool, and the singleton caches and services: `MemoryCache`, `FontCache`, `FontDatabase`, `RenderTheme`, `PlatformStrategies`, the style sheet, selector query and text breaking caches, and the other `Singleton()`s. Each page (or each agent, as the specifications group them) should be able to live on its own thread, with this state per thread or agent, or made thread-safe where it's shared (the memory and font caches).
- Only image decoding leaves the main thread, on the host's work queue. Glyph rasterisation, canvas encoding (`toDataURL`, `toBlob`) and SVG image rasterisation still run on it.
- Image objects can't be shared between threads. `ImageBuffer`, `ImageSource` (and `BitmapImageSource`), `ImageDecoder` and `ImageFrameWorkQueue` are single-threaded `RefCounted` with `CanMakeWeakPtr`, and their state (frame caches, decoding state, observers) is unlocked. Decoding works by handing a decoder to a work queue job and back (`ImageFrameWorkQueue`), so only one thread holds it at a time. Sharing them (a compositor or decoding thread that keeps references, as WebKit does) needs Krystal.Core's `RefCountedThreadSafeAndCanMakeWeakPtrThreadSafe` and `ThreadSafeWeakPtr` for their references, and locks or thread checks for their state: changing the base class alone only makes the counts atomic.

## DOM

### Missing

- `title` is never shown (nor an SVG `<a>`'s `xlink:title`, which Chrome shows): there's no advisory information, no `HitTestResult::Title`, no tooltip hook on the chrome client, and Krystal.UI draws no tooltips.
- No customized built-in elements: definitions have no `extends`, and an element's `is` value is kept (cloned, serialised) but never looks up a definition. Elements created with an `is` value are uncustomized.
- `ElementInternals` has no ARIA reflection members (`detail::GetContentAttribute` in `ReflectAttribute.hpp` returns null for them) and no `CustomElementDefaultARIA`.
- `DocumentTouch` (`createTouch`, `createTouchList`) is not implemented; `Touch` has no constructor and there is no `TouchInit`.
- `DataTransferItem` has no `getAsEntry` (no file system API).
- No `FileReader`, streams, blob URLs or blob registry. A `Blob` holds its data itself (`BlobData`), and `Blob::Bytes()`/`Text()` read synchronously, returning the exception a failed read rejects with (`NotFoundError` for a missing file, `NotReadableError` otherwise).

## HTML elements and forms

### Missing

- A number input in a locale with other digits (Arabic-Indic, say) shows its value in them, but digits typed are inserted as ASCII ones; the decimal separator typed is the locale's.
- Customizable `<select>`: `appearance: base-select` on the select alone gives a picker with base appearance, where without `::picker(select)` opting in too it should be the native one, as `RenderMenuList` owns the popup menu. List boxes ignore `appearance: base-select` and stay native. Type-ahead with the picker open changes the selection, where it should move the focus. Page Up and Page Down move ten options, not a viewport's worth. A `legend` in an `optgroup` isn't its label in a native menu.
- `<img>` lacks the `Image()` constructor (FIXME in `HTMLImageElement.hpp`), and `sizes=auto` (auto sizes for lazy images: `auto` is an invalid size), and doesn't select a source again when media queries change (FIXME in `HTMLImageElement.cpp`). `HTMLImageLoader` has no `<object>` fallback (FIXME); `ImageLoader`'s reuse of available images has no preloads (FIXME).
- `HTMLLinkElement` recognises only `rel=stylesheet`, `alternate` and `icon`: no `expect` (internal resource links that block rendering until an element is parsed; FIXME in `HTMLLinkElement.cpp`), preloads, manifests, integrity, referrer policies or priorities (no `LinkRelAttribute`/`LinkLoader`).
- `<a ping>` and referrers are not implemented; the `download` attribute is only passed to the host. `<object usemap>` is not supported, and `<object>` and `<embed>` don't display images, so an object always shows its fallback content.

## CSS parsing and CSSOM

### Missing

- No user or extension style sheets: `ScopeRuleSets::InitializeUserStyle` adds nothing and `Style::Scope` has no injected author sheets (FIXMEs), and user style sheets' `@import`s and load options aren't handled.

## Style (cascade and computed values)

### Missing

- `Style::PendingResources` has no user agent shadow tree support and no anonymous mode for masks (FIXME).
- Anchor-positioned viewport-constrained objects need a scrolling coordinator Booey doesn't have.

## Animations

### Missing

- Scroll and view timelines have no accelerated representation (no threaded animations), and a change in view timeline metrics doesn't update the timeline (FIXME in `KeyframeEffect.cpp`).
- Image animations pause out of view and resume (`RenderView::AddRendererWithPausedImageAnimations`), and there is no per-element animation control (`RenderElement::AllowsAnimation` follows only the page's `SetImageAnimationEnabled`; FIXME).

## Layout

### Bugs

- A transformed box doesn't add its transformed rect to its container's scrollable overflow (FIXME in `RenderBox::LayoutOverflowRectForPropagation`). Porting upstream's transform makes the overflow stretch balanced columns, which Chrome doesn't: the last column set grows to the flow's layout overflow, so `RenderMultiColumnSet::CalculateBalancedHeight` sees more columns than specified and stretches them (the `multicol` golden's rotated box). Fixing it needs balancing to ignore overflow from transforms.

### Missing

- An unstyled XML document (one with no style sheets, hints or HTML, SVG or MathML elements) is rendered as its text, not as a view of its source or tree.
- `layout_integration::GridLayout::CanUseFor` accepts only single-track spans at explicit unnamed lines and the self-alignments `start`, `flex-start`, `self-start`, `normal` and `stretch`; other grids fall back to `RenderGrid`. Multi-column spans in `ImplicitGrid` are a FIXME.

## Painting

### Bugs

- A dashed rounded border with different widths on each side has its dashes drift from Chrome's along the sides, by a few pixels (FIXME in `BorderPainter::DrawBoxSideFromPath`).
- Dashed collapsed table borders share out the junctions between cells slightly differently from Chrome: an edge after a junction starts a few pixels off, so its gaps differ a little (the `tables` and `dashed-borders` goldens).
- A mask's `color-interpolation: linearRGB` is drawn in sRGB (`LegacyRenderSVGResourceMasker`, FIXME), because image buffers have no colour space conversion.
- A composited mask's or clip path's edge pixels (and those of a turned masked layer) can differ slightly from the whole-page repaint.

### Missing

- There's no software filter path: filters are applied by the host's accelerated backend, or by the graphics context for the ones it can draw. `filter: url()` to an external document is not supported (see *Images and resources*).
- Backdrop filters are drawn only by the compositor: the flattened whole repaint (`View::Record`) doesn't draw them.
- `ImageQualityController::ChooseInterpolationQuality` returns the style's quality or the default: there is no size- or scroll-dependent choice.
- Text painting has no document markers (spelling, grammar), no IME composition painting, no dragged or transparent content and no glyph display list cache. There is no print-backgrounds setting, and font subpixel quantization is a FIXME.
- Render theme parts left out or FIXMEs: `<input type=checkbox switch>`, devolvable widgets, the text area resize corner image, and the system's reduced motion and contrast settings.
- Images changed mid-drag are not updated, and there are no root view rects (`caretRectInRootViewCoordinates`, `editableElementRectInRootViewCoordinates`) for hosts' drop previews.

## Compositing and scrolling

### Missing

- There's no scroll anchoring, autoscroll timers beyond the scrollbar repeat, kinetic scrolling or rubber-banding (FIXMEs in `Element`, `LocalFrameView`, `EventHandler`, `ScrollableArea`, `Scrollbar`). Scroll positions are whole pixels, so wheel deltas are rounded, and a smooth scroll moves in whole pixels.
- Scroll snapping for touchpads without phases: a touchpad that isn't a precision one (or a host that doesn't take `OnTouchpadPan`) reports its pans as the wheel's notches, each going to the next snap position, so its small deltas page through a mandatory container.
- There are no native scrollbars.
- `VisualViewport` is the layout viewport at the page scale and never scrolls within it, so its `scroll` event never fires (its `resize` does). There are no obscured insets, auto-sizing, base viewport layout size scale factor (FIXME in `VisualViewport.cpp`) or layout viewport separate from the scroll position. Pinch zoom is only the page scale (1 to 5) from the view's gesture recogniser.
- There is no async scrolling or threaded compositing: no scrolling coordinator or tree, no reconciliation of the composited scroll position after user scrolls, and animations and scrolling run on the main thread. A sticky box stuck to the view therefore repaints its old and new rects on scroll instead of compositing.
- Reference (SVG `url()`) filters are identities on composited layers (`TextureMapper.cpp` converts the other filter functions to `LayerFilter2D`).
- Clip paths never composite as shape layers; a composited clip path is painted into the mask layer. Replicas (reflections), repaint counters, repeating contents tiles (`WrapMode::Repeat`) and creating tiles over several frames are FIXMEs.
- The compositor has no scrolling nodes, event or interaction regions, layer flush throttling, model or separated layers, or HDR and wide gamut backings (`ContentsFormat` is RGBA8 only). There is no `TiledBacking` (`SetIsInWindow` and `SetTileCoverage` do nothing), the fixed root background layer is never composited, and the host's compositing policy under memory pressure is a FIXME.

## Text and fonts

### Missing

- Colour glyphs from bitmap strikes (`CBDT`) are drawn at their strike's size, unscaled, while COLR glyphs scale. Large colour glyphs stay bitmaps.
- A colour glyph's layers in the foreground colour (`CPAL` entry `0xFFFF`) are drawn black, FreeType's default, rather than in the text's colour (`FontFace2D::Rasterise` sets no foreground colour). The atlas would need to key colour glyphs by that colour.
- `FontDescription::SetSpecifiedLocale` resolves every Han locale to `zh-hans`, as there are no user language preferences.
- Right-to-left scripts other than Arabic and Hebrew (Syriac, Thaana, N'Ko and so on) have no bundled font and work only if the host registers one. Kashida justification is not done.
- Web fonts: SVG fonts, font collections (the item in a collection is not selectable), and a font parsing policy (safe font parser, trusted types) are missing. Font loading has no settings: installed fonts are always allowed and load completions are never ignored.
- Text search (`findPlainText`, `SearchBuffer`) is absent. The text iterator does not traverse the flat tree or emit text placeholders (FIXMEs in `TextIterator.cpp`).

## Images and resources

### Missing

- External reference filters (`filter: url(doc.svg#f)`) are not loaded or watched (FIXME in `RenderLayerFilters.cpp`). `cross-fade()` and filter images load their inputs but do not follow them as clients (FIXMEs in `StyleCrossfadeImage.cpp`, `StyleFilterImage.cpp`), so they do not update when an input loads, and `StyleCrossfadeImage::Blend` always returns null, so `cross-fade()` percentages don't interpolate.
- `CachedResource` has no preloads, priorities or streaming and partial data; a resource decodes only once all its data has arrived. `CachedResourceLoader` loads images, fonts and style sheets only; other types, request counting and loads that do not delay the load event are FIXMEs. `MemoryCache` has one LRU list. `SharedBuffer` is one contiguous immutable buffer.
- Without a work queue from the host, every frame decodes on the main thread as it's drawn. Images still decoding aren't redrawn synchronously for a host's snapshot (`invalidateImagesWithAsyncDecodes`; FIXMEs in `Page.cpp`, `RenderImage.cpp` and `BackgroundPainter.cpp`).
- Animated images (GIF, APNG, animated WebP) have no `allowsAnimation` override or system animation controls, and no controls for suspended documents or off-screen views.
- Images are sRGB only: ICC profiles are ignored (FIXME in `PNGImageDecoder.cpp`), and there is no HDR, headroom or gain map support. `NativeImage` has no colour spaces.
- Orientation is applied to the pixels, not by drawing. `GraphicsContext`'s default `DrawImageBuffer` and `DrawPattern` for image buffers draw nothing (FIXMEs in `GraphicsContext.cpp`: `nativeImageForDrawing`).
- SVG images keep the light appearance and do not copy the embedding page's settings (generic font families, dark appearance), because image observers cannot reach them (FIXME in `SVGImage.cpp`). The page of an SVG image has no loader strategy, so it loads only `data:` URLs.
- Drags: no drag icon for a dragged image's file (`CreateDragImageIconForCachedImageFilename` returns none), the response's suggested filename is not parsed, and there are no text indicators or visible paths. `PasteboardImage` and `SelectionData` carry no image, so nothing prefers images' types when pasting.

## MathML

### Missing

- Negative leading and trailing operator spaces, negative `lspace` on `<mpadded>` and negative `<mspace>` widths are not implemented (FIXMEs). Operator spacing is not applied to the whole embellished operator, and the dictionary does not accept multi-character operators.
- Percentages in some MathML lengths are not resolved (FIXMEs in `RenderMathMLBlock.cpp`). Floats are neither positioned nor cleared because `display: math` is unimplemented.
- An operator whose assembly font is missing returns without painting.

## Canvas

### Missing

- Without a host device (Booey's tests, or a view before its first paint), `getImageData` and other readbacks return transparent black, and `createPattern` of a canvas throws `InvalidStateError`.
- Drawing is sRGB with 8 bits a channel. `display-p3` is accepted and drawn as sRGB; `rgba-float16` `ImageData` reads and writes 8 bits a channel (no `Float16Array`); `PixelFormat` has RGBA8, BGRX8 and BGRA8 only. The `alpha` context setting is accepted but ignored (the context is always transparent).
- `ImageBitmap` and `OffscreenCanvas` are not `CanvasImageSource`s. `createImageBitmap`, `OffscreenCanvas`, `ImageBitmapRenderingContext`, `PaintRenderingContext2D`, WebGL and WebGPU contexts do not exist, and there are no context loss events.
- A canvas copy and every readback wait for the GPU.

## Editing and selection

### Bugs

- `Pasteboard::CanSmartReplaceContents()` is false for the system clipboard, because the smart-paste type is neither written nor read; smart replace only works within a `SelectionData` carried by a drag.

### Missing

- Formatting commands are absent: bold, italic and the rest of the `styleWithCSS`/`-webkit-font-size-delta` family, lists, indent and outdent, format block, links, remove format. Tab spans are absent with them. The `MakeTextWritingDirection*` commands apply styles but report no state.
- Paste does not render the fragment in a staging document to take its text as rendered and drop invisible content (`ReplacementFragment`). `MarkupAccumulator` does not resolve URL attributes against the base URL, and the markup sanitiser's staging page has no dark appearance.
- Bidi-aware pasting (matching each pasted paragraph's direction) and the bidi contiguity helpers (`AdjustToVisuallyContiguousRange` and relatives) are absent.
- `EditorClient` is only the selection, clipboard permission, undo/redo, composition and text-field hooks. It has no `shouldInsertText`/`shouldDeleteRange`/`shouldBeginEditing`/`shouldApplyStyle` delegates, so the host cannot veto or observe edits, and no spelling, grammar or text-checking, autocorrection or autofill hooks.
- The drag caret, the overtype block cursor, caret browsing's focusing of the anchor the caret is in, stopping the blink while typing, beeping on empty cuts and deletes, and spatial navigation's use of the caret are absent.
- Image documents and writing images to the pasteboard are absent.
- Input methods are IMM32 only. TSF features (reconversion, inline handwriting, speech), custom highlights and annotations (inline predictions, writing suggestions), `inputmode`, `ime-mode` and an on-screen keyboard are not supported. The page draws no candidate list and the host is not told the candidates; the IME draws its own windows.
- Dragging a selection out to other applications is not supported.

## Input and events

### Missing

- Context menus are not built: only the `contextmenu` event is dispatched, and there is no keyboard (menu key) route to it (`sendContextMenuEventForKey`).
- Wheel: `PlatformWheelEvent` has no velocities or inverted directions, and there is no scroll latching or wheel delta filter. A precision touchpad's pans come with their phases and momentum (DirectManipulation in Krystal.PAL), but not its pinches or swipes. Pan gestures chain on to the next scroll box mid-gesture, as the wheel does.
- Drag and drop: colour drags and drops are absent. Touch and pen drags are not done (pointer events cancel when a drag starts).
- Sequential focus navigation does not handle moving the focus out to the host, and wraps at the end of the document. Focusable areas do not include scrollable regions or draggable elements (FIXMEs in `FocusAlgorithms.cpp`).
- Touch: touch event emulation from the mouse is absent. Touch `pointermove` events do not carry the delta since the last event.
- `IsDraggableLink` is a local check (an HTML link with `href`, not editable); the editable-link-behaviour setting is absent.

## Loading, navigation and history

### Missing

- Responses can't be streamed: the main resource arrives whole on a networking task, and the loader strategy has no cancellation or synchronous loads (FIXME in `LoaderStrategy.hpp`).
