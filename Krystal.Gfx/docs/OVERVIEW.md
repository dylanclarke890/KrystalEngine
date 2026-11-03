# Krystal.Gfx

Krystal.Gfx holds the render hardware interface (the RHI, `krys::gfx`) and Render2D, an immediate-mode 2D renderer built on it. Neither knows anything about Booey: Krystal.UI replays Booey's display lists through Render2D (see `Krystal.UI/docs/OVERVIEW.md`). This covers what the headers don't say: the design decisions, conventions and known issues.

## The RHI

- **Ownership.** Whoever creates a resource destroys it; creation returns `Expected` (shader and pipeline errors carry the compile or link log, prefixed with the debug name), and nothing throws. Handles are generational (`Handle<TTag>`: 24-bit index, 8-bit generation, 0 invalid); a stale handle resolves to null (an assert in debug builds). The device logs every resource still alive when it's destroyed. Destroying a resource that's recorded but not yet submitted is an error.
- **Device.** OpenGL 4.5 (`OpenGL/`, WGL) is the only backend. A device needs no window: its context lives on a hidden window it owns and each swapchain attaches a real window, so several windows share one device and tests render offscreen. Equal sampler descriptions return the same, reference-counted handle.
- **Shaders** are GLSL 4.50 strings passed to the driver unchanged (starting with `#version`), with numbered `layout(binding = N, std140)` slots and no uniform-name lookups; their C++ mirrors are `static_assert`-checked. GLSL puts every uniform block's members in one namespace, so blocks can't share member names. Per-draw data goes in an ordinary uniform block from `AllocateTransient`; there are no push constants. Shaders live in `Renderer2D.cpp` and `FilterRenderer2D.cpp`; `OpenGL/ShaderReflector.hpp` is unused.
- **Frames.** `BeginFrame`, record, `Submit`, `Present`, `EndFrame`. Transient memory is one persistently mapped buffer split into `FramesInFlight` (3) regions; `BeginFrame` waits on the fence of the region it reuses, and an exhausted frame returns an empty allocation (an assert in debug builds). Caches (atlases, pooled textures, the layer pool) use `FrameNumber()` to know when a frame's commands have been submitted.
- **Conventions.** Pixel coordinates (viewports, scissors, texture regions) have a top-left origin with y down; texture memory starts with the top row and UV (0, 0) is the top-left texel; clip space has y up and depth 0 to 1. Rendering into a texture and sampling it doesn't flip the image. The RHI has no colour type: clear values are four numbers in the target's encoding. `BlendMode` and `GradientSpreadMethod` live in Krystal.Core (`Colour/`), shared with Booey.

## Render2D

- **Colour.** Colours are sRGB straight alpha in the API and premultiplied in the shader. Blending is on encoded (non-linear) values everywhere, as browsers blend; a target in an sRGB format asserts. Textures in sRGB formats are decoded when sampled and re-encoded by the shader, so premultiplied images belong in unorm formats. A 3D renderer sharing a window must write encoded values itself.
- **Recording.** Draws are appended as instance records and turned into passes at `End`. A batch breaks on a change of clip chain, texture or sampler, gradient or pattern, blend mode, atlas or layer; each instance carries its own transform, so transforms don't break batches. `GetCommandBuffer()` lets other renderers record into the same command buffer first, so a frame can draw textures rendered earlier in it. `GetStats()` reports the last frame's instances, batches, cache hits and backdrop copies.
- **Two paths to the screen.** Rects, circular rounded rects, rects with a rounded hole, straight strokes and their blurred shadows are analytic: the shader computes coverage from the signed distance, with an antialiasing width that follows the transform. Everything else (paths, general strokes via the CPU `PathStroker`, elliptical corners, big glyphs) is rasterised on the CPU (`PathRasteriser`, 16 sub-scanlines per row) into 8-bit masks in the `MaskAtlas`, keyed by path, fill rule, the transform's linear part and the fractional translation, so whole-pixel moves reuse a mask.
- **Atlases.** Glyphs (R8 coverage and RGBA colour pages) and masks have separate atlases sharing `AtlasPages`' shelf packing. A full atlas clears the least recently used page the current frame hasn't drawn from; if the frame has drawn from all of them, the item gets its own texture for the frame.
- **Clips** are antialiased as browsers do: each draw's coverage is multiplied by its clips'. A `ClipRect` on whole pixels under an axis-preserving transform is just the scissor; other shape clips are evaluated exactly in the shader (a uniform block per batch, at most `MaxClips` = 16); path clips sample a coverage mask, and nested path clips are multiplied on the CPU into a per-frame mask.
- **Layers** (`BeginLayer`/`EndLayer`) draw into a pooled offscreen texture sized to what was drawn within the clip (rounded up to 64 px), composited as one premultiplied image with its opacity, blend mode, composite operator and `LayerFilter2D` (colour matrix, Gaussian blur, drop shadow, luminance to alpha). Pooled textures are reused across frames, not within one, and freed after 120 unused frames. A blur grows the layer by three standard deviations within its clip and reads only what's inside it.
- **Blend modes.** A draw with a mode that reads the backdrop gets its own batch: the pass splits, the draw's reach is copied into a pooled backdrop texture, and the shader blends. A swapchain can't be copied from, so a frame that blends onto one draws into a pooled texture first. `PlusLighter` is additive blending with no copy. Composite operators are fixed-function blend state, bounded on draws (only what a draw covers changes).
- **Shadows** follow Skia's model: `DropShadow2D` is drawing state applied beneath each fill and stroke, Gaussian with a standard deviation of half the radius. Analytic shapes blur in the shader; masks blur on the CPU (`CoverageBlur`: an exact kernel below 8 px deviation, three box blurs above) and are cached with the mask.
- **Filter graphs.** `FilterGraph2D` and `FilterRenderer2D` evaluate SVG filter graphs on the GPU, one fragment shader per node kind, over pooled float textures.

## Known issues

### Bugs

- `BlendMode::PlusDarker` isn't supported by `Renderer2D`: it asserts in debug builds and draws as `Normal`. `CompositeOperator2D` has no `PlusDarker` or `Difference` (FIXME).
- `MaskAtlas` and `GlyphAtlas` only clear whole pages, so a page with one mask drawn every frame keeps its stale masks until the atlas is destroyed. Single masks are not freed and pages are not compacted.
- A mask shadow is cached under its fractional position like any mask, so a shadow moving by fractions of a pixel is rasterised and blurred on the CPU every frame.
- Gradients are flattened on every draw, with no cache (513 interpolations per span per draw for a gradient in a non-sRGB space). `StrokePath` also works out the stroke outline on every call, before the mask cache is looked up.
- Each batch that copies the backdrop costs a pass split and a copy, and overlapping blended draws copy once each. A draw's reach is a bounding box, so nearby draws may not share a copy.
- A clip chain holds at most 16 shape clips; more assert and the extra clip is dropped. If a frame's transient memory runs out, its draws are dropped (an assert in debug builds).
- Antialiasing is approximate in places. The analytic rounded-box shadow is within about 2 colour steps of numerical integration. Under a transform that stretches unevenly, mask shadows blur by the average scale rather than in the shape's own coordinates. Analytic coverage on curves and diagonals can differ from true area by up to a tenth of a pixel's coverage, and rects thinner than a pixel come out heavier than their true coverage. Flattened gradient colours are within a quarter of a step of the interpolation at the samples and up to half a step at clamping kinks.
- Images cast no drop shadows, and a gradient's shadow uses its shape's coverage as if opaque. `LayerFilter2D::DropShadow` covers CSS `drop-shadow()` filters.

### Missing

- Elliptical corners: `RoundedRect2D`/`CornerRadii2D` and the analytic rounded hole are circular per corner, so elliptical corners are drawn by the caller as paths.
- Blurs read only what is inside the clip and reach at most 256 px; blur edge modes are `None` and `Duplicate` only.
- Coverage-correct antialiased edges for unbounded operators on draws.
- Text contrast and gamma adjustment (Skia's) and DirectWrite-like rendering.
- No partial clear, no threaded painting, and no hardware blending through `KHR_blend_equation_advanced`.
- Colour management: sRGB only, 8 bits a channel.
- 2D only: no perspective or 3D rendering.
- RHI: no compute, texture views, explicit barriers (OpenGL doesn't need them; another backend would), multiple queues, multithreaded command recording, bindless resources or push constants. There's no second backend (Vulkan or D3D) and no shader toolchain (reflection, offline compile, SPIR-V, cross-compilation); `gl.cpp` has a Windows-only loader (FIXME). There's no 3D scene renderer and no shared asset layer.

## Differences

### From the specs

- Render2D blends sRGB-encoded values everywhere, as browsers do, rather than in linear light. Layer textures use the target's format (8-bit unorm, as browsers do) rather than 16-bit float.
- Coverage is sampled in 16 sub-scanlines per pixel row (exact horizontally, sixteenths vertically), like Chrome's CPU scan converter, and curve flattening stays under a fiftieth of a pixel of error. Exact-area accumulation was rejected because it can't do even-odd.

### From Chrome

- Render2D's rendering model is its own: a GPU instance batcher with analytic shapes and CPU coverage masks, not Skia. Differences in antialiasing and shadow blurs follow from that.
- Backdrop blur edges are duplicated rather than mirrored, so goldens differ along the glass's edges.
