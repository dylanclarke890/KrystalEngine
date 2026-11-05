# Krystal.UI

Krystal.UI is the only module that knows both Booey and Krystal.Gfx: it hosts Booey pages (`ui::View`), replays Booey's display lists through Render2D, and supplies the strategies and providers Booey asks its host for. Krystal.PAL supplies windows, input, the clipboard, the cursor, drag and drop and gamepads. Krystal.Booey never depends on Krystal.Gfx. This covers the contract a host follows, testing, and known issues; how Booey itself works is in `Krystal.Booey/docs/OVERVIEW.md`, and Render2D's design in `Krystal.Gfx/docs/OVERVIEW.md`.

`ui::View` (`Krystal.UI/View.hpp`) is the host object for one page and the unit a host (the editor, a game UI) embeds. It creates the `boo::Page` and main `LocalFrame` with their clients (`ChromeClient`, `FrameLoaderClient`, `DragClient`, `EditorClient`), loads HTML from a string, file or URL, and owns the viewport, the font face cache, the `TextureMapper`, the image-buffer textures and the overlays. `Krystal.Editor` shows the wiring: it drives a `View` below its own header, with a custom element (`BeanRating.hpp`), observers, animations and pointers set up once a page loads, a navigation handler that loads links to its own pages and shows its guide's topics in place, and F5 reload.

## A frame

- **Time is passed in, never read.** Call `View::Update(now, budget)` once a frame, before painting, with a non-decreasing monotonic time. It runs Booey's event loop (due timers, then tasks with a microtask checkpoint after each), then the rendering update if one is scheduled, and returns whether the rendering update ran. The engine can pause or scale UI time, and tests drive time without hooks.
- **Focus and visibility.** The host passes the window's focus to `View::SetFocused` (a view starts focused); losing it hides an open overlay, confirms the composition and cancels a drag. `View::SetVisible` shows and hides the page (a view starts shown): a hidden page fires `visibilitychange`, and suspends animation frame callbacks, SMIL and CSS animations. `View::SetFramesPerSecond` gives the rate the host draws at (its display's refresh rate with vsync, or its frame cap; 60 until told), which animations align their updates to.
- **Paint.** If `NeedsPaint()`, call `View::Paint(Renderer2D &)` between the renderer's `Begin` and `End`. It brings layout and compositing up to date, paints dirty tiles, composites the layer tree under the renderer's current transform, then paints the overlay and any drag image. A host must paint after an update that asks for it. `SetDevice` gives the view its device before the first paint; otherwise it takes the first device it paints with.
- **Record and replay.** `View::Record()` flattens the page into a `DisplayList` as a snapshot (canvases showing textures paint their pixels into it); `View::Replay` replays any display list with the view's fonts.
- **Loading.** `LoadHTML`, `LoadFile`, `Load` and `Reload` commit on a later task; the view starts with the frame's initial empty document. The view never navigates by itself: links and form submissions go to the navigation handler, which loads what it wants, and there's no session history (a host that wants back and forward keeps its own list of what it loaded).
- **Custom elements.** `View::DefineCustomElement(name, factory, options)` applies to each document the view loads, before parsing, and to the current document.

## Rendering

- `GraphicsContextRender2D` is the `boo::GraphicsContext` that replays display lists through Render2D, mostly one Render2D call per item. It saves the renderer's state on construction and restores it on destruction. Corners within a sixteenth of a pixel of circular use Render2D's analytic shapes (snapping otherwise made 16 px corners 15.99 by 16 ellipses and sent every card through path masks); elliptical corners are paths.
- Booey's graphics layers are `GraphicsLayerTextureMapper`s composited by `TextureMapperLayer` (`Compositing/`); the view keeps the painted page in tiles and repaints only damage.
- Image buffers: `ImageBufferRender2DBackend` is the accelerated buffer (canvas bitmaps). It records into a display list and renders into the texture it keeps when next drawn; readbacks render at once and use `Device::ReadTexture`. `ImageBufferTextures` renders and uploads images through renderers that record into the replay's command buffer ahead of its passes; textures are pooled by size across replays, which is safe because OpenGL runs commands in order. `AcceleratedImageBuffers` renders outside a paint, waiting for the GPU before and after because transient memory belongs to the host's frames.
- External images (engine textures through a `<canvas>`'s `"krystal-texture"` context, canvas snapshots) are `TextureImage`s drawn without a copy.
- `FilterGraph2DBuilder` is Booey's accelerated filter backend, turning each effect into `FilterGraph2D` nodes.
- `FontFaceCache` maps a Booey `Font` to a `FontFace2D`, one per FreeType face and synthetic bold or oblique combination, shared by every size.
- Controls use the Adwaita theme behind `RenderTheme`'s interface, so another theme can replace it.

## Input

`View` takes Krystal.PAL's types and `Input.hpp` maps them to DOM events (`code`, `key`, legacy key identifier, Windows key code).
- **Mouse:** `MouseMoved`, `MousePressed`, `MouseReleased`, `MouseLeft`, `MouseWheel`, with click counting and the cursor. A wheel notch is the system's line count times 100/3 px; Shift turns the vertical wheel horizontal.
- **Pen and touch** come from Windows pointer messages through `PenInput` and `TouchInput`. Gestures honour `touch-action`: tap, double tap, pan with fling, long press as `contextmenu`, and pinch-scale between 1 and 5. `SetPointingDevices` drives the pointer and hover media features and `maxTouchPoints` (until set, a mouse alone).
- **Keyboard:** `KeyPressed`, `KeyReleased`, `InsertText`. `EditorClient` runs the editor commands Windows binds keys to and keeps the undo and redo stacks.
- **Input methods** follow GTK's model: `SetComposition`, `ConfirmComposition`, `InsertText`, `AcceptsTextInput`, `TextInputRect` (for the host's candidate window) and `SetCompositionDiscardedHandler`. The page confirms its composition on a press and on window focus loss.
- **Gamepads:** `View::GamepadInput` maps Krystal.PAL's pads to page navigation (see *Input and events* in `Krystal.Booey/docs/ARCHITECTURE.md`); `SetGamepadNavigationEnabled(false)` turns it off for pages that read pads themselves.
- **Pointer lock** is offered to `SetPointerLockHandler`; without one pages can't lock the pointer.
- **Drag and drop:** the host runs the drag. Mouse moves become `DragEnteredOrUpdated`/`DragExited`, release becomes `PerformDragOperation`, then `DragEnded`/`DragSourceEndedAt`; Escape and focus loss cancel and Ctrl copies. Krystal.PAL captures the mouse while a button is held, and the window's OLE drop target is registered only while the host has `OnDrag`. The drag image is drawn over the page at three-quarters opacity.

## Host seams

- The first `View` installs, unless the host already set one: `FileLoaderStrategy` (the loader), `SystemPasteboardStrategy` (over Krystal.PAL's clipboard; tests and the sample use an in-memory clipboard) and `PathStrokerStrategy` (stroke hit testing and bounds from Render2D's `PathStroker`, so hit tests match what's drawn).
- Callbacks on `View`, all optional with conservative defaults (an action is ignored): `SetNavigationHandler` (without one, links and submissions do nothing), `SetLoadHandler`, `SetFocusRequestHandler`, `SetPointerLockHandler`, `SetPointingDevices`.
- Dark mode is the main frame's `MediaEnvironment::UseDarkAppearance`; `View` has no setter, so a host sets it through the frame's `SetMediaEnvironment`.
- Overlays: a `<select>`'s popup, the date and time pickers and the colour picker are `Overlay`s, internal `View`s placed in the owner's viewport whose generated HTML lists the items (`PopupMenuOverlay`, `DateTimeChooserOverlay`, `ColorChooserOverlay`). While one is open it's painted over the page and takes the keys and mouse; it hides before telling its client the choice. Popups are mirrored for right-to-left controls.

## Testing

- **Goldens** are the quality bar for Render2D and Krystal.UI. Scenes are rendered offscreen, read back with `ReadTexture`, and compared with Render2D's own reviewed PNGs (tight tolerance, catching regressions) and with Chrome's rendering of the same HTML (looser tolerance, measuring fidelity). Render2D scenes are in Krystal.Gfx.Tests (`Scene::ChromeReportOnly` marks report-only comparisons); page goldens are in `Krystal.UI.Tests/tests/Golden.cpp`. The harness runs the event loop before painting so resources and fonts are loaded. `KRYS_UPDATE_GOLDENS=1` rewrites goldens; fresh output goes to `build/Krystal.UI.Tests/golden-output`.
- **Chrome comparisons** use a per-channel tolerance and a small share of pixels allowed past it; text is only reported. References are captured by `tools/golden/capture_chrome.py` (headless Chrome at device pixel ratio 1; `--golden DIR`). With `tools/golden/cdp.py` it also records `?rects` client rects (`<name>.chrome-rects.txt`), `?record` selection steps (`chrome-editing.txt`) and scripted input (`chrome-input.txt`). Headless Chrome lays out at least 500 px wide, so scenes fix their width and put `direction: rtl` on a wrapper, not `<body>`. Chrome draws popups in separate windows, so popup goldens are Booey's alone.
- Goldens differ slightly on battery power (the `filters` golden by about 26 pixels); small GPU differences aren't regressions. A failing golden tagged `[!shouldfail]` marks a known bug.

## Known issues

### Bugs

- `GraphicsContextRender2D` falls back to source-over for `PlusDarker` and `Difference`, which Render2D lacks (FIXME).
- Buffers of a resolution scale other than 1 replay their transforms unscaled (FIXME in `ImageBufferRender2DBackend.hpp`).
- `EditorClient`'s text-field notifications (`TextFieldDidBeginEditing`, `TextFieldDidEndEditing`, `TextDidChangeInTextField`, `TextWillBeDeletedInTextField`, `TextDidChangeInTextArea`) and `DoTextFieldCommandFromEvent` do nothing.
- Pressing the mouse in the view while an overlay opens doesn't end the event handler's press (FIXME in `View.cpp`).
- The resizer grip is drawn in pixels rather than the platform's image, and search fields have no recent searches.
- `TextureMapperAnimation.cpp` has two FIXMEs about suspect behaviour: `_totalRunningTime` is cleared where it seems it shouldn't be, and `Pause` is passed a time where it takes a time offset.

### Missing

- `TextureMapperLayer` flattens 3D transforms (Render2D is 2D only). Replicas, reference filters in `TextureMapper`, masks and backdrops on `TextureMapperLayer`, contents scaling filters (`image-rendering` on composited images and canvases) and repeating textures are FIXMEs.
- Layers are composited on the main thread, with no compositor thread or async scrolling. That's worth doing only once something can keep the main thread busy.
- The colour space passed to a host image buffer is ignored, and the view's target and pixel readback are sRGB, 8 bits a channel (FIXME in `View.cpp`). `Path` has no platform implementation: arcs are flattened into bezier curves (FIXME in `Graphics/Paths/Path.hpp`).
- Host-reported system values are constants: the double-click time (500 ms) and distance (2 px) and the touch thresholds on `View` (FIXMEs); the caret blink, reduced-motion and contrast settings; the page's screen position (`screenX`/`screenY`, unless the host calls `SetWindowRect`; FIXMEs in `Input.cpp` and `View.cpp`); dead keys (`"Dead"`), which Krystal.PAL doesn't report.
- Input methods: a host `CompleteComposition()` on the window is unused, and there is no input purpose or hints.
- The host answers a drag a frame late: the source hears the host's answer at its next move and at the drop. Entering answers no operation. A release where the last answer was no operation comes as a leave, not a drop. Over the editor's header, or with another view shown, a drag is off the page. Drag positions aren't offset by the drag offset, and `setDragImage()` after the drag started doesn't change the image.
- Drags from other applications: the host reads a drag's data once as it enters. The legacy `FileName`/`FileNameW` formats, virtual file descriptors (browser-dragged images and links), pages' custom data and smart pasting aren't read. Drags out of a page to other applications aren't supported. Krystal.PAL's OLE side isn't driven by tests.
- No host-supplied file picker for `<input type=file>`, and no autofill (FIXME in `EditorClient.hpp`).
- `FileLoaderStrategy` reads files on the main thread (FIXME).
- The date and time chooser uses fixed English names and first day of the week (FIXME); colour inputs' `alpha` and `colorspace` and user activation for pickers aren't handled. Typing to choose in an open popup isn't supported, and the popup's scrollbar look differs.
- Controls: devolvable widgets (FIXMEs in `RenderTheme.cpp`), checkmarks for `appearance: base`, customizable `<select>` (`appearance: base-select` parses but draws as `auto`), file inputs, datalists and tick marks, the validation message bubble, media controls, attachment parts, and search-field icons.

## Differences

### From the specs

- Date and time fields: partly filled dates report bad input.
- The date inputs' locale comes from `lang` when `Settings::LangAttributeAwareFormControlUIEnabled` is on, else the user interface's.

### From Chrome

- Text-control and number-field layouts differ slightly from Chrome's (report-only goldens).
- The control theme is Adwaita rather than Chrome's. Popups are drawn by the page's own overlays; Chrome's are separate windows. Matching Chrome is the long-term aim, with the theme staying behind `RenderTheme`'s interface so a Chrome-like theme can replace it.
