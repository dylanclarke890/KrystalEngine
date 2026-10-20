# Graphics redesign

Status (2026-10-19): C0 (the colour types) and G1 (the RHI and its OpenGL backend) are done; Render2D (G2) is in progress (G2a, solid rects, G2b, rounded rects and antialiasing, G2c, transforms and rect clips, G2d, images, G2e, paths, masks and rounded holes, G2f, strokes, G2g, clips, G2h, layers and blend modes, G2i, gradients, and G2j, shadows, and G2k, golden image tests, are done); Krystal.Text (T1) and text (G3) are done; the Booey integration (G4) is in progress (G4a, the graphics context and display lists, G4b, Krystal.UI and replay, G4c1, the paint plumbing, layers and backgrounds, G4c2, borders, shadows and outlines, G4c3, image buffers and patterns, G4d1, text and inline boxes, G4d2, decorations, markers and ellipses, G4e, table painting, G4f1, hit testing, hover and active, G4f2, mouse events, clicks and cursors, G4g, focus and blur, and G4h1, keyboard events, are done; see *G4 plan*).

## Why

The current Krystal.Gfx has an interface per resource type (`I*Registry`). Each registry creates GPU objects, loads assets from the VFS, and caches by name with manual reference counts. The interfaces are OpenGL-shaped: the renderer `static_cast`s them back to the OpenGL types, and the Vulkan and D3D11 contexts can't implement them. Ownership is implicit (`Load`/`Unload` pairs, `TextureOwner`), and errors are exceptions thrown through `noexcept` functions. `IContext` also owns the LearnOpenGL demo scenes.

The first real consumer will be Booey: drawing a page's display list into a window or texture. The redesign is sized for that, while keeping the 3D path open.

## Goals and non-goals

Goals:
- One job per layer: the GPU API, 2D drawing, assets, and the Booey integration are separate.
- A rendering hardware interface (RHI) shaped like the modern APIs (pipelines, render passes, binding slots, recorded command buffers). OpenGL 4.5 is its only backend for now, but a Vulkan or D3D12 backend should be possible without changing callers.
- Explicit ownership; no exceptions; every creation can fail with a message.
- A fast 2D renderer: batched, antialiased, premultiplied alpha.
- One set of colour types for the whole engine.

Non-goals for now:
- A second backend, compute, multithreaded command recording, async asset streaming.
- A 3D scene renderer (cameras, lights, materials, meshes). It comes later, on the same RHI.

## Modules

```
Krystal.Core                colour types (moved from Booey), CommandList
  └─ Krystal.Text           font files and faces (FreeType, with their HarfBuzz faces), shared by Gfx and Booey
       ├─ Krystal.Gfx       the RHI: device, handles, descriptions, command buffers. No files, no caches.
       │                    Backends are subdirectories (`OpenGL/`, namespace `krys::gfx::opengl`).
       │                    Render2D/: 2D drawing on the RHI (fills, strokes, paths, images, glyphs, clips, layers)
       └─ Krystal.Booey     paints into a display list; shapes text (its port of upstream's); no dependency on Gfx
Krystal.UI                  the Booey host: pages, display list replay into Render2D, input routing
Krystal.Assets (later)      VFS loading, decoding, path caching, reference-counted asset handles
```

Decision (2026-10-17): backends aren't separate modules; Krystal.Gfx holds the RHI and an `OpenGL/` subdirectory (more later, one directory each).

Booey never depends on Krystal.Gfx. Krystal.UI is the only module that knows both.

## Colour

Decision (2026-10-17): the engine uses Booey's typed colours, moved into Krystal.Core, spelled "Colour", in the `krys` namespace.

- Done in C0 (2026-10-17): Booey's typed colour maths moved from `Krystal.Booey/Graphics/Color/` to `Krystal.Core/Colour/`: the components, models, types (`SRGBA<T>`, `LinearSRGBA<T>` and the other colour spaces), transfer functions, matrices, conversion, colour spaces, `PackedColour`, normalisation and utilities, the typed halves of luminance and interpolation, and the interpolation method types. Blending, which works on the CSS `Color`, stays in Booey. The ledger (`docs/booey/PORTING.md`, *Colour types in Krystal.Core*) has the detail.
- Rename "Color" to "Colour" in the moved files and names (`ColourComponents`, `ColourSpace`, `ColourMatrix`, `PackedColour`, `ColourConversion.hpp`, ...), and move them from `krys::boo` to `krys`.
- Booey keeps the CSS-facing `Color` value type and its serialisation, built on the moved types. It keeps the spelling `Color`, matching the CSS specifications (decision, 2026-10-17).
- Gfx's `Colour.hpp`, `ColourFormatter.hpp` and `ColourHasher.hpp` were deleted in G1, with the registries and renderer that used them.

Where each layer gets its colours:
- **The RHI has no colour type.** A clear value is four numbers in the target's encoding (`Array<float, 4>`, or integers for integer formats). Whether a value is sRGB or linear depends on the texture's `TextureFormat`, not on a colour class.
- **Render2D and Krystal.UI** use the typed colours: vertex colours are `PackedColour::RGBA` (premultiplied `SRGBA<uint8>`). Render2D blends sRGB-encoded values, as browsers do (decided in G2h2).

## Geometry

Decided (2026-10-18): Render2D uses its own small, plain geometry types for now. Booey's geometry types (`FloatRect`, `AffineTransform`, ...) stay in Booey; Krystal.UI converts between them at the boundary. Moving Booey's types to Core was considered and dropped for now: they are tied to layout-only types (`LayoutUnit`, `Region`, `Path`, `TransformationMatrix`, `WritingMode`), so the move needed many API changes in Booey.

## Layer 1: the RHI (Krystal.Gfx)

Namespace `krys::gfx`.

### Concepts

#### Pipelines

In OpenGL, a draw call uses whatever global state is set when it's made: the bound program and VAO, blending, depth testing, culling, the framebuffer. That has three costs:

1. The driver learns the full combination at the last moment. Blending, the vertex layout and the target format are partly compiled into the shader's machine code, so a new combination recompiles the shader during the draw (the warning `131218`, "Vertex shader is being recompiled based on GL state", which `Context.cpp` currently silences).
2. State leaks between unrelated code. The current renderer disables depth testing and enables blending at the start of `Submit` and undoes it at the end, and `CompositeRenderTargetWithOpacity` swaps `glBlendFunc` and restores it by hand.
3. Mismatches (a shader's inputs against the vertex layout) aren't caught; they draw garbage.

A **pipeline** is an immutable object, created once at load time, that holds everything affecting how the GPU runs the shaders:

```
Pipeline
├─ Shaders           vertex + fragment
├─ Vertex input      buffer stride; attribute 0 = float2 at offset 0, attribute 1 = 4 unorm bytes at offset 8
├─ Topology          triangles / lines / strips
├─ Rasteriser        cull mode, front face, fill mode, scissor test on or off
├─ Depth/stencil     test on or off, write on or off, compare function, stencil ops
├─ Blend             on or off, factors and ops, colour write mask
├─ Output formats    "renders into one RGBA8 sRGB colour target, no depth"
└─ Resource layout   "uniform block at slot 0, texture at slot 0"
```

Binding one is a single call at draw time, and nothing is left for the driver to work out.

**Dynamic state** isn't in the pipeline: the viewport and scissor rectangle, which buffers and textures are bound (the pipeline only fixes the slots and their kinds), the blend constant, the stencil reference, and clear colours (which belong to the render pass).

Pipelines are cheap to bind and expensive to create (that's when shaders compile and link), so they're created up front: one per combination of shaders and state actually used. A 2D UI renderer needs about 5 to 8. Pipelines that differ only in state share shaders: today's `CompositeRenderTarget` and `CompositeRenderTargetWithOpacity` become two pipelines with the same shaders and different blend state.

The output formats are part of the pipeline because blending and the conversion to the target's format are compiled in. The same shaders drawing into the window (RGBA8 sRGB) and into an RGBA16F texture are two pipelines; Render2D hides this with a cache keyed by the description and the formats.

#### Binding slots

Shaders declare numbered slots instead of being set by uniform name, and the program fills buffers and binds them to those numbers:

```glsl
#version 450
layout(binding = 0, std140) uniform Frame      // slot 0: per-frame data, one buffer for the whole frame
{
  mat4 u_Projection;
  vec2 u_ViewportSize;
};
layout(binding = 1, std140) uniform Draw       // slot 1: small per-draw data, from transient memory
{
  mat4 u_Transform;
};
layout(binding = 0) uniform sampler2D u_Texture; // texture slot 0

layout(location = 0) in vec2 a_Position;      // must match the pipeline's vertex input
layout(location = 1) in vec4 a_Colour;
```

There are no string lookups, data shared by every draw is uploaded once per frame, and the C++ struct mirroring each block is checked with `static_assert`s on its size and offsets. OpenGL has supported `layout(binding = N)` since 4.2.

#### Render passes

A render pass brackets drawing into a set of targets, and says what to do with their contents:
- At the start: `Load` keeps them, `Clear` clears them, `DontCare` means everything will be overwritten.
- At the end: `Store` keeps the result, `DontCare` lets the GPU discard it (a depth buffer only needed during the pass).

On tiled GPUs this saves memory bandwidth. In OpenGL it's a framebuffer bind, clears, the draws, and `glInvalidateNamedFramebufferData` for discarded targets. Its main value here is that the target is always explicit, never "whatever was bound last".

#### Command buffers

Draws aren't executed when they're recorded. A command buffer records them as plain structs into Krystal.Core's `CommandList`, and the backend replays them on `Submit`. Recording costs a copy per command, there's no virtual call per draw, and the same recording model works for Vulkan, which needs it.

### As built (G1, 2026-10-17)

The headers are the reference; this lists what they hold and the behaviour that isn't obvious from them.

```
Krystal.Gfx/include/Krystal.Gfx/
  Handles.hpp          Handle<TTag> (24-bit index, 8-bit generation; 0 is invalid) and the six handle types
  Pool.hpp             generational storage; stale handles resolve to null, nothing throws
  Formats.hpp          TextureFormat, IndexType, VertexFormat and their sizes and kinds
  CompareOp.hpp        depth, stencil and sampler comparisons
  Buffer.hpp           BufferDesc, BufferUsage, MemoryUsage
  Texture.hpp          TextureDesc, TextureDimension, TextureUsage, TextureRegion
  Sampler.hpp          SamplerDesc (SamplerFilter, SamplerMipmapMode, SamplerAddressMode, SamplerBorderColour)
  Shader.hpp           ShaderDesc, ShaderStage
  Pipeline.hpp         PipelineDesc: vertex layouts, topology, BlendState, DepthStencilState, RasterState
  RenderPass.hpp       RenderPassDesc, ColourAttachment, DepthStencilAttachment, AttachmentLoadOp/StoreOp
  Swapchain.hpp        SwapchainDesc
  Commands.hpp         the recorded commands (krys::gfx::commands)
  CommandBuffer.hpp    the recorder, with usage checks
  Device.hpp           the device interface; Device::Create
  OpenGL/              GLDevice, GLContext (Win32), GLMappings, the loader hooks, the Khronos headers
```

**Conventions.** Pixel coordinates (viewports, scissors, texture regions) have a top-left origin, y pointing down. Texture memory starts with the top row, and UV (0, 0) is the top-left texel. In clip space, y points up and depth runs from 0 to 1. Rendering into a texture and sampling it doesn't flip the image.

**Shaders** are GLSL 4.50, passed to the driver unchanged, and must start with a `#version` line. GLSL puts every uniform block member in one namespace, so blocks can't share member names.

**No push constants** (decision, 2026-10-18). G1 had them, emulated in OpenGL with a `KRYS_PUSH_CONSTANTS` macro injected into the source that expanded to a uniform block at a reserved slot 15. They were dropped: the macro was a hidden dialect, the reserved slot was a surprise, and the emulation used `std140` where Vulkan push constants default to `std430`, so it wouldn't have been portable anyway. Per-draw data goes in an ordinary uniform block filled from `AllocateTransient`. Revisit with the shader toolchain decision (open decision 2).

**Device.**
- `Device::Create(DeviceDesc)` needs no window: the OpenGL context lives on a hidden window the device owns, and each swapchain attaches a real window to it (sharing the pixel format). Several windows can share one device, and tests render offscreen without showing anything.
- Creation returns `Expected`; shader and pipeline errors carry the compile or link log, prefixed with the debug name.
- `Destroy` is immediate in OpenGL: the driver keeps objects alive while queued commands use them. Destroying something recorded but not yet submitted is an error, as in Vulkan.
- Samplers are shared: equal descriptions return the same handle, reference counted by `Destroy`.
- At destruction, the device logs every resource still alive, by debug name.
- Beyond the draft: `ReadTexture` (synchronous readback, for tests and screenshots), `GetTextureDesc`, `SetVSync`, `WaitIdle`, `Limits`, and `FrameNumber` (added in G2e2: frames begun so far, so caches know when a frame's commands have been submitted).
- `AllocateTransient` hands out per-frame memory from one persistently mapped buffer split into `FramesInFlight` (3) regions; `BeginFrame` waits on the fence of the region it reuses. It returns an empty allocation (and asserts in debug builds) when the frame's memory runs out.

**Command buffers** record plain structs into `CommandList`; `Submit` replays them and resets the buffer. Debug builds assert on misuse where it happens: drawing outside a render pass or without a pipeline, nested passes, copies inside a pass, unbalanced debug groups. Buffer bindings carry over between render passes; each pass binds its own pipeline.

**The OpenGL backend.**

| RHI | OpenGL 4.5 |
|---|---|
| Buffer | `glNamedBufferStorage` with `GL_DYNAMIC_STORAGE_BIT`, so `WriteBuffer` works on any buffer. |
| Texture | `glTextureStorage*`; BGRA formats are stored as RGBA and uploaded as `GL_BGRA`. |
| Pipeline | A linked program, a vertex array built from the layouts (`glVertexArrayAttribFormat`/`Binding`), and the state as data. Bound buffers are applied to the pipeline's vertex array at draw time, only when they change. |
| State | A cache of the last applied blend, depth-stencil and raster state; binding a pipeline changes only what differs. |
| Render pass | A cached framebuffer object per attachment set (framebuffer 0 for a swapchain). `glClipControl` is `GL_UPPER_LEFT` for textures and `GL_LOWER_LEFT` for windows, which gives the conventions above; the front face is flipped to match. Clears open the write masks and scissor first. `DontCare` stores call `glInvalidateNamedFramebufferData`. |
| sRGB | `GL_FRAMEBUFFER_SRGB` is on for textures (it only affects sRGB formats) and set per swapchain format. |
| Context | WGL: a throwaway legacy context loads the entry points, then a 4.5 core context (debug when validating) on the hidden window. Debug output goes to the global logger. |

Tests: `Krystal.Gfx.Tests` (21 cases): handles and the pool, formats, command recording, and a GPU suite on a real device that reads results back (clears, sRGB encoding, per-draw uniform data, orientation, sampling, transient uniform buffers, indexed draws, scissor, blending, depth, copies and blits, mipmaps, errors, sampler sharing, stale handles). The editor draws a rotating triangle through the RHI.

### Left out for now

Compute, texture views, explicit barriers (OpenGL doesn't need them; a Vulkan backend would track them internally), multiple queues, multithreaded recording, bindless. Each can be added without changing the shape above.

## Layer 2: the 2D renderer (Render2D, in Krystal.Gfx)

A generic, immediate-mode 2D API on the RHI. It knows nothing about Booey; game HUD code and debug overlays can use it directly.

Revised (2026-10-18): the API mirrors upstream's `GraphicsContext` primitives, not CSS concepts. Booey ports upstream's painters (`BorderPainter`, `BorderShape`, backgrounds, shadows) as they are, and they express everything as fills, strokes, clips and layers; Render2D only has to draw those. The draft's CSS-shaped calls (`DrawBorder`, `DrawBoxShadow`) are dropped. The goal is upstream's geometry and paint order exactly, with antialiasing that looks the same; browsers don't agree with each other pixel for pixel, so images are compared with a tolerance.

```cpp
namespace krys::gfx
{
  class Renderer2D
  {
  public:
    static Expected<UniquePtr<Renderer2D>> Create(Device &device) noexcept;

    void Begin(CommandBuffer &commands, const Render2DTarget &target) noexcept;
    void End() noexcept;

    // State, as stacks.
    void Save() noexcept;
    void Restore() noexcept;
    void ConcatTransform(const Transform2D &transform) noexcept; // also Translate, Scale, Rotate
    void SetTransform(const Transform2D &transform) noexcept;
    const Transform2D &GetTransform() const noexcept;
    void ClipRect(const Rect2D &rect) noexcept;
    void ClipRoundedRect(const RoundedRect2D &rect) noexcept;
    void ClipOutRect(const Rect2D &rect) noexcept;
    void ClipOutRoundedRect(const RoundedRect2D &rect) noexcept;
    void ClipPath(const Path2D &path, FillRule rule) noexcept;
    void BeginLayer(float opacity, BlendMode blend = BlendMode::Normal) noexcept;
    void EndLayer() noexcept;

    // Fills and strokes.
    void FillRect(const Rect2D &rect, const Paint2D &paint) noexcept;
    void FillRoundedRect(const RoundedRect2D &rect, const Paint2D &paint) noexcept;
    void FillRectWithRoundedHole(const Rect2D &rect, const RoundedRect2D &hole, const Paint2D &paint) noexcept;
    void FillPath(const Path2D &path, FillRule rule, const Paint2D &paint) noexcept;
    void StrokePath(const Path2D &path, const StrokeStyle &stroke, const Paint2D &paint) noexcept; // width, caps, joins, dashes

    // Content.
    void DrawImage(TextureHandle texture, const Rect2D &source, const Rect2D &destination, const ImageOptions &options) noexcept;
    void DrawGlyphs(Span<const GlyphQuad> glyphs, const Paint2D &paint) noexcept;
  };
}
```

`Paint2D` is a solid colour or a linear, radial or conic gradient (stops flattened on the CPU, see *G2i plan*); image patterns can join it later. Shadows follow upstream's model (a shadow is graphics state applied to the next fill: a blurred copy of its coverage, offset and coloured), and are designed with that part.

Two ways a shape reaches the screen; everything is drawn on the GPU either way:

- **The fast path: analytic shapes.** Rects and rounded rects (most of a UI: backgrounds, simple borders as a rect with a rounded hole) are instances drawn by the shader from G2b, which computes each pixel's coverage from the signed distance to the shape. No CPU work beyond the instance.
- **The general path: coverage masks.** Any other shape (paths, strokes, dashed and dotted borders, `clip-path`, multi-coloured borders built from clipped side polygons) is rasterised on the CPU into a greyscale coverage mask, like a font rasteriser does for a glyph, and kept in a mask atlas. The GPU draws a quad that samples the mask and multiplies it by the paint. Masks are cached by shape, so the CPU only works when a shape is new or changes. Glyphs (G3) are drawn the same way, from their own atlas.

Considered for the general path (2026-10-18): CPU tessellation into triangles with MSAA (4 to 8 coverage levels, stepping on near-horizontal edges; concave shapes need a robust triangulator or stencil-then-cover), and tessellation with an antialiased fringe (good for convex shapes only). Coverage masks were chosen for their antialiasing (exact per-pixel coverage, closest to browser output) and because they handle any path and both fill rules.

Implementation notes:
- Draws are batched in painter's order; a batch breaks on a change of pipeline, texture or clip.
- Pipelines (analytic shapes, masks, images, layer composite) are created on first use and cached per target format.
- Clips: axis-aligned rects with no transform use the scissor. Other clips (rounded rects, paths, transformed rects) are antialiased like the browser's, as a clip mask the draws inside them multiply their coverage by.
- Layers render to pooled offscreen textures and composite with their opacity and blend mode.
- Coordinates are pixels with a top-left origin, y pointing down. Booey's painting already snaps border widths and box edges to device pixels, so untransformed edges land on pixel boundaries; thinner or fractional geometry only comes from transforms, and is antialiased like any other edge.
- Atlases (decided 2026-10-18): glyphs and shape masks have separate atlases, each with its own packing and eviction, drawn through the same textured-quad pipeline. Glyphs are small, reused every frame and long-lived; masks can be large and short-lived (animated or resized shapes), and sharing would let mask churn fragment the space and evict glyphs. Both use shelf packing into R8 pages; the glyph atlas also has RGBA pages for colour glyphs. Masks too large for an atlas page get their own texture. Switching between the atlases breaks a batch; if that shows up in profiling, both can become layers of one array texture, read from a single binding, while keeping their own allocators.

## Layer 3: the Booey integration (Krystal.UI)

- Booey ports upstream's `GraphicsContext` and `DisplayList::Recorder` (`platform/graphics/GraphicsContext.h`, `displaylists/`: 63 item types). Painting records into a display list, so Booey stays testable without a GPU, and tests can compare display lists.
- The first items to port are the ones boxes and text need:
  - state: `Save`, `Restore`, the transforms;
  - clips: `Clip`, `ClipRoundedRect`, `ClipOut`, `ClipOutRoundedRect`;
  - fills: `FillRect` (colour, gradient), `FillRoundedRect`, `FillRectWithRoundedHole` (borders, inset shadows), `FillPath`;
  - strokes: `StrokeRect`, `StrokePath`, `DrawLine`, `DrawLinesForText` (underlines);
  - path clips: `ClipPath` (side polygons for multi-coloured borders, `clip-path`);
  - content: `DrawGlyphs`, `DrawNativeImage`;
  - layers: `BeginTransparencyLayer`, `EndTransparencyLayer`.
- Booey ports upstream's painters (`rendering/BorderPainter`, `BorderShape`, the background and shadow painting) as they are; they decide how borders and shadows are built from the primitives above.
- Krystal.UI replays a display list into Render2D, mostly one call per item.
- Glyphs are rasterised by Render2D with FreeType, from the same `krys::text::FreeTypeFace` Booey shapes with, and cached in Render2D's glyph atlas (see *G3 plan*).
- Images: Booey asks the host to load and decode them; Krystal.UI uploads them to textures and maps Booey's image objects to texture handles.
- Krystal.UI also hosts pages and routes input.

## Layer 4: assets (later)

`Asset<T>` is a reference-counted handle (`RefPtr` to an asset record) that releases its RHI resources when the last reference goes away, cached by VFS path. It replaces the `Load`/`Unload` pairs. The Booey slice doesn't need it: Krystal.UI loads fonts and images directly.

## What happens to the current code

| Current | Fate |
|---|---|
All done in G1 (2026-10-17) unless noted.

| Old | Fate |
|---|---|
| `ResourceManager` | Replaced by `Pool`. |
| `ResourceHandleCache` | Deleted; caching moves to the asset layer. |
| `I*Registry`, the OpenGL registries and resource wrappers | Replaced by `Device` and `GLDevice`'s plain records. |
| `IContext`, `IRenderer`, `Commands.hpp` | Replaced by `Device` and `CommandBuffer`; `Renderer2D` comes in G2. |
| `Colour.hpp`, `ColourFormatter.hpp`, `ColourHasher.hpp` | Deleted; the colour types moved to `Krystal.Core/Colour/` in C0. |
| `Context.cpp`'s demo scenes, every editor shader and asset (`Krystal.Editor/data/`) | Deleted (decision, 2026-10-17); they're in git history. |
| `Mesh`, `Material`, `FlatColourMaterials`, `Light`, `ICamera`, `FirstPersonCamera`, `MeshDataUtils`, `InstanceData`, `Vertex.hpp` | Deleted with the scenes they served; the 3D renderer will define its own. |
| The old 2D renderer's text drawing | Deleted; the editor shows no text until G3 (decision, 2026-10-17). |
| `FontAtlasLoader`, msdf-atlas-gen | Kept, moved to `Text/`, unused until text in 3D. Deleted in G3a (2026-10-18), with `msdfgen`: text is greyscale bitmaps (open decision 3). |
| `ShaderReflector` | Kept in `OpenGL/`, not yet wired up (planned: checking pipeline binding layouts in debug builds). |
| `Krystal.Gfx.OpenGL` | Merged into Krystal.Gfx as `OpenGL/`; the loader hooks and WGL code moved with it. |
| `Krystal.Gfx.Vulkan`, `Krystal.Gfx.D3D11` | Deleted (decision, 2026-10-17), with the `vulkan-headers` vcpkg dependency. |
| `Handle.hpp` in Krystal.Core (namespace `krys::Gfx`) | Deleted; `Krystal.Gfx/Handles.hpp` replaces it. |
| `Texture.hpp` in Krystal.Gfx (namespace `Gfx::OpenGL`) | Deleted with the registries. |

## Slices

| Slice | Content | Done when |
|---|---|---|
| C0 | Done (2026-10-17). Move the colour types from Booey to `Krystal.Core/Colour/`, renamed to "Colour" in `krys`. | Booey and its tests build and pass against the moved types. |
| G1 | Done (2026-10-17), as one part. The RHI: `Device`, `Pool` and handles, buffers, textures, samplers, shaders, pipelines, command buffers, swapchains, transient memory; the OpenGL backend. The editor moves onto it; Gfx's old colour headers go with the registries. | The editor clears its window and draws a triangle through the RHI. The old registries are gone. |
| G2 | Render2D: the analytic fast path, transforms, images, paths and coverage masks, strokes, clips, layers, gradients, shadows. Reviewed one part at a time (see *G2 parts*). | A test app draws a UI-like scene; golden image tests pass. |
| T1 | Done (2026-10-18). `Krystal.Text`: font files and faces move down from Booey, shared by Booey and Gfx (see *T1 plan*). | Booey and its tests build and pass on the moved faces; Krystal.Text has its own tests. |
| G3 | Done (2026-10-18). Text: the glyph atlas (R8 pages, RGBA pages for colour glyphs), `DrawGlyphs`, FreeType rasterisation. It doesn't depend on G2e's mask atlas. In two parts (see *G3 plan*): done (2026-10-18). | Text renders crisply at several sizes and scales. |
| G4 | Booey: `GraphicsContext`, the display list recorder, painting (paint order, backgrounds, borders, text), hit testing; Krystal.UI replays it. Done (2026-10-19), in parts G4a to G4g (see *G4 plan*); G4h, keyboard events and focus navigation, extends it (in progress). | The vertical slice: a page loads, renders, resizes and responds to a click. |

### G2 parts

Decided (2026-10-18): one part at a time, each reviewed before the next. Revised the same day after G2b: the API follows `GraphicsContext` and general shapes go through coverage masks (see *Layer 2*), so borders and shadows are no longer Render2D parts of their own.

| Part | Content |
|---|---|
| G2a | Done (2026-10-18). The `Render2D/` skeleton, `Begin`/`End`, solid `FillRect`, batching; an offscreen readback test; the editor draws rects. |
| G2b | Done (2026-10-18). Rounded rects and antialiasing: the analytic fast path (the signed distance shader). |
| G2c | Done (2026-10-18). Transforms (`Transform2D`), the `Save`/`Restore` stack, scissor clips; the analytic antialiasing width follows the transform. |
| G2d | Done (2026-10-18). Images: `DrawImage`, sampling, the textured-quad pipeline the masks reuse. |
| G2e1 | Done (2026-10-18). Paths and the rasteriser, CPU only: `Path2D`, `FillRule`, curve flattening, the coverage rasteriser (see *G2e plan*). |
| G2e2 | Done (2026-10-18). The mask atlas and its cache; `FillPath`. |
| G2e3 | Done (2026-10-18). `FillRectWithRoundedHole`, analytic for circular corners; elliptical corners through masks. |
| G2f1 | Done (2026-10-18). Strokes, CPU only: `StrokeStyle`, the stroker and dashing (see *G2f plan*). |
| G2f2 | Done (2026-10-18). `StrokePath` through masks; `StrokeLine` for straight lines, analytic. |
| G2g1 | Done (2026-10-18). Clips in the shader: `ClipRect` under any transform, `ClipRoundedRect`, `ClipOutRect`, `ClipOutRoundedRect` (see *G2g plan*). |
| G2g2 | Done (2026-10-18). Path clips through masks: `ClipPath`, `ClipOutPath`. |
| G2h1 | Done (2026-10-18). Layers with opacity: `BeginLayer`/`EndLayer`, pass ordering, the texture pool (see *G2h plan*). |
| G2h2 | Done (2026-10-18). Blend modes: backdrop copies and a blend shader; blending on encoded values everywhere (see *G2h2 plan*). |
| G2i | Done (2026-10-18). Gradients (linear, radial, conic) as `Paint2D`, as one part (see *G2i plan*). |
| G2j | Done (2026-10-18). Shadows: upstream's shadow state; analytic blurs for rects, rounded rects and rounded holes, blurred masks for paths (see *G2j plan*). |
| G2k | Done (2026-10-18). Golden image tests: UI-like scenes against Render2D's own reviewed images and against Chrome's (see *G2k plan*). |

### G2e plan (decided 2026-10-18)

- **`Path2D`** holds primitive segments only: `MoveTo`, `LineTo`, `QuadTo`, `CubicTo`, `Close`, with helpers that build shapes from them (`AddRect`, `AddRoundedRect` with elliptical radii, `AddEllipse`, `AddArc`, arcs as cubics). Booey's path records richer segments; the Booey host (planned, G4) converts them with these helpers.
- **Flattening:** the path is transformed into target pixels first, then curves are split into lines whose error stays under a fiftieth of a pixel, so the subdivision is right at any scale. The lines cut inside convex curves; at a tenth of a pixel (the first attempt) curved edges sat about 0.05 px inward, enough to thin a 1 px curved stroke by a tenth.
- **The rasteriser** (option B of three considered): each pixel row is sliced into 16 sub-scanlines; along each, the edge crossings and their winding give the inside spans under either fill rule, and each pixel accumulates the exact length of the spans over it. Coverage is exact horizontally and in sixteenths vertically, like Chrome's CPU scan converter. Rejected: exact area accumulation (font-rs style; can't do even-odd, which SVG, `clip-path` and canvas need) and 16-by-16 supersampling (about 16 times slower).
- **The mask atlas:** R8 pages packed into shelves, separate from the glyph atlas. A mask is reused only when the path, the transform's linear part and the fractional part of the translation all match exactly (exact reuse, chosen over rounding to quarter pixels, since Booey snaps most painting to whole pixels anyway); whole-pixel moves reuse it. Masks unused for a few frames are evicted; masks too large for a page get their own texture. Page size, eviction age and the size limit are tuning values.
- **Drawing masks:** a mask is rasterised in target pixels, so it's drawn as an untransformed, pixel-aligned instance whose colour is multiplied by the mask's coverage (a `Mask` instance flag); the rect shader covers such a quad exactly.
- **Borders' fast path:** `FillRectWithRoundedHole` is analytic when the hole's corners are circular (outer coverage times one minus the hole's). Rounded rects with elliptical corners (from `10px / 20px`, from percentage radii on non-square boxes, and from the inner edges of borders whose sides differ in width) go through masks for now; an analytic approximation can be added later if profiling asks for it.

### G2f plan (decided 2026-10-18)

- **`StrokeStyle`:** width, caps (butt, round, square), joins (miter, round, bevel), miter limit, dash pattern and dash offset, with the meanings canvas and SVG give them. The miter limit defaults to 10 (canvas's default; SVG's is 4). A width of 0 strokes a hairline one target pixel wide, as Skia does; canvas callers skip such strokes instead.
- **Dashes** follow SVG and canvas: an odd-length pattern is repeated to make it even (on and off alternate, so the pattern only repeats after two passes); each contour restarts the pattern; a zero-length dash with round or square caps draws a dot or a square. Patterns that are empty, have a negative length or sum to zero stroke solid.
- **General strokes go through masks:** a CPU stroker turns the stroke into an outline in the path's coordinates (dashing first, by length along the flattened path), which is filled under non-zero with the same transform, so overlapping pieces merge and the mask cache applies. Curves are flattened finely enough for both the curve and the offset edge: the tolerance is in target pixels, and the turn per line is limited by the width, so wide strokes stay accurate on their outer edge.
- **Straight lines are analytic:** a straight stroke is a rotated rect, a round-capped one a rotated rounded rect with radius half the width, and a round dot a circle, which the rounded-rect shader already draws exactly under any transform. Border sides, underlines and straight dotted or dashed lines don't touch the CPU.

### G2g plan (decided 2026-10-18)

- **Clip coverage in the shader** (option B of three considered): every draw multiplies its coverage by how much of the pixel its clips let through, so clip edges are antialiased as in the browser and no extra passes are needed. Rejected: the stencil buffer (pixels are in or out, so clip edges would be jagged) and a layer per clip (an offscreen texture and pass each; layers are G2h's job).
- **Shape clips** (rects under any transform, rounded rects, and their clip-outs) are computed exactly in the shader, each under its own transform; a clip-out is one minus the shape's coverage, and nested clips multiply. **Path clips** (G2g2) sample the path's coverage mask, from the mask atlas.
- **Per batch:** a clip change already starts a batch. Each batch binds a small uniform buffer with its clip chain (the shapes in effect), and the scissor is the chain's bounds, so pixels certainly outside are skipped; instances don't change. Clips whose edges are all on whole pixels under a transform that keeps rects axis-aligned are the scissor alone, which costs nothing.
- **At most 16 shape clips** in a chain; more asserts (a tuning value; real pages rarely nest more than a few).
- **Known seam:** where a shape's edge lies on a clip's edge, both partly cover the edge pixels and the product is fainter (a half times a half is a quarter, not a half), as in browsers built on Skia.

### G2h plan (decided 2026-10-18)

- **What for:** a layer draws a group into an offscreen texture and composites it as one image, as CSS `opacity` on an element with children, `mix-blend-mode` and `isolation` need. `BeginLayer(opacity, blend)` saves the drawing state and `EndLayer()` restores it, as upstream's `beginTransparencyLayer`/`endTransparencyLayer` do.
- **Passes:** draws are already only recorded at `End`, so a layer's batches are kept apart and its pass is recorded before the pass that composites it: children before parents, the target last. Compositing is an image draw of the layer's texture (premultiplied, at the layer's opacity) in the parent; clips and transforms carry on inside layers unchanged, with each pass offsetting its viewport and scissor by the layer's position in target pixels.
- **Textures:** layer textures have the target's format (8-bit, like browsers, rather than 16-bit float) and cover the bounds of what was drawn in the layer, within the clip, worked out at `End`. They come from a pool reused across frames but not within one, since every layer's pass runs before the parent pass that reads it.
- **Blend modes (G2h2)** read the backdrop: the parent's pass is split at the layer, the backdrop copied, and a shader blends the two. Copying from a swapchain needs something the RHI doesn't have yet (likely drawing the frame offscreen when a blend mode is used); that's decided in G2h2.

### G2h2 plan (decided 2026-10-18)

- **Colour space (open decision 4, decided):** blending works on sRGB-encoded values everywhere, as browsers do. Render2D's targets and layers are unorm (`RGBA8Unorm`), and colours, images, coverage, layer opacity and blend functions all work on premultiplied encoded values; a target in an sRGB format asserts. The GPU still decodes images in sRGB formats when sampling them, and the shader encodes each channel back, which is exact for premultiplied and straight images alike. The editor's window becomes `RGBA8Unorm`; a 3D renderer sharing a window writes encoded values itself, as its final pass usually does.
- **`BlendMode` moves to Core** (`Krystal.Core/Colour/BlendMode.hpp`, `krys::BlendMode` and `BlendModeName`), shared by Booey and Render2D. `PlusDarker` (Apple only) stays in the enum; Render2D asserts it isn't used and draws normally. `CompositeOperator` stays in Booey until canvas needs it.
- **API:** `SetBlendMode`/`GetBlendMode` are drawing state, saved and restored with the rest. `BeginLayer(opacity)` composites the layer with the current blend mode, as upstream's `beginTransparencyLayer` does, and `BeginLayer(opacity, blend)` names it; inside a layer the blend mode starts as `Normal`.
- **Backdrop copies:** a draw with a blend mode other than `Normal` and `PlusLighter` gets a batch of its own. At `End`, the pass it's in is split before it: the pass ends, the pixels the batch reaches (within its scissor) are copied with `BlitTexture` into a backdrop texture, and the pass resumes keeping its contents. A later draw with the same blend mode joins the batch when nothing else came between and its pixels don't overlap the batch's, so a dashed line or tiled background shares one copy. The backdrop texture has the pass's pixel layout, so the shader reads the fragment's own texel; one texture from the layer pool serves the whole frame.
- **Shader:** the backdrop is on texture unit 2 (the white texture when unused). The shader un-premultiplies the backdrop and the source, applies the blend function `B(Cb, Cs)` (the separable and non-separable functions of Compositing and Blending Level 1) and outputs `as·((1 − ab)·Cs + ab·B)` with alpha `as`; normal blending then adds `(1 − as)·cb`, so the pipeline's blend state doesn't change and pixels the draw doesn't cover stay exactly as they were. `PlusLighter` is additive blending (a second pipeline, `ONE, ONE`) and needs no copy.
- **Swapchains (option A of three):** a swapchain can't be copied from, so in a frame where a draw blends straight onto a swapchain target, the target's draws go into a pooled texture the window's size, cleared to the clear colour (or transparent), which is then composited into the window. When the target keeps its contents (`Load`), what was already in the window isn't part of the backdrop: top-level blend modes blend with what Render2D drew this frame, as a page's root is an isolated group. Texture targets are copied from directly. Rejected for now: letting the RHI copy from swapchains (B), which every backend would need.
- **Later optimisation (C):** `KHR_blend_equation_advanced` (and `VK_EXT_blend_operation_advanced`) blend in hardware without copies, on drivers that have them; an opt-in for apps that know their hardware, with the copies as the fallback.
- **Tests:** every mode against the specification's formulas computed on the CPU; antialiased edges; draws sharing a copy and draws that can't; blend draws inside a layer and a layer composited with a blend mode; `Save`/`Restore`; `PlusLighter`; sRGB images into unorm targets. The swapchain path only runs with a window, so the editor covers it: shapes over the image in several modes and a layer blended with `multiply`.

### G2i plan (decided 2026-10-18)

- **Types:** `Gradient2D` in `Render2D/Gradient2D.hpp`, Render2D's own plain mirror of upstream's `Gradient`: linear (two points), radial (two circles and an aspect ratio, the two-point conical model of canvas and SVG, of which CSS's `radial-gradient` is the case with one centre and a start radius of 0) and conic (a centre and a start angle) data, colour stops (`ExtendedSRGBA<float>`, so wide-gamut colours last until interpolation), a spread method and a `ColourInterpolationMethod`. `GradientSpreadMethod` moves from Booey to Core (`Krystal.Core/Colour/GradientSpreadMethod.hpp`), as `BlendMode` did.
- **API:** a `Paint2D`, a colour or a gradient with its gradient-space transform (gradient space to the current coordinates, as upstream's `setFillGradient` takes it), taken by every fill and stroke. A colour converts to it implicitly, so existing calls don't change; image patterns can join it later. Rejected: a gradient overload of every fill and stroke.
- **Evaluation (option C of three):** the CPU flattens the stops, with Core's interpolation in the gradient's colour space, into stops that are linear in premultiplied encoded sRGB (what the GPU blends), splitting each span until the flattened colours are within a quarter of a colour step of the true ones, as curves are flattened into lines. Interpolation in legacy sRGB with premultiplied alpha (the default) is exactly linear there, so it isn't split. The shader finds the pixel's span by binary search and mixes linearly, so hard stops stay exact. Rejected: a ramp texture (hard stops blur over a texel) and every colour space in the shader (exact, but a large shader).
- **Per batch:** a gradient is a uniform block on slot 2 (shape, spread, the map from target pixels into gradient space, the radial geometry, up to 256 flattened stops); instances with it carry a flag. A draw with a different gradient starts a batch, as a clip chain does; solid draws join any batch. Consecutive draws with the same gradient share it.
- **Geometry follows upstream's Skia backend:** the gradient space, the transform and each shape's own normalisation (the linear gradient's line onto the x axis; the radial gradient's aspect ratio as a vertical scale about its first centre; the conic gradient rotated by its angle less 90 degrees) fold into one map from target pixels, so the shader is short and clips, masks and transforms work unchanged. Radial gradients solve the two-point conical equation for the largest `t` whose radius isn't negative; pixels without one are transparent. Conic gradients go clockwise from straight up. Spread methods pad, reflect or repeat `t`. Degenerate gradients (a line or circles of zero size) are solid fills of the last colour (pad) or the average colour (reflect, repeat); a radial gradient whose circles coincide is the first colour inside and the last outside. No stops draws nothing; one stop is a solid fill.
- **Dithering:** gradients are dithered on 8-bit targets, as Chrome does: an 8-by-8 ordered (Bayer) pattern in target pixels, under half a colour step, added before blending. The frame block carries its strength (a step for 8-bit formats, nothing otherwise). Solid colours aren't dithered.
- **Tests:** the flattened stops against Core's interpolation in several spaces (sRGB premultiplied and not, OKLab, OKLCH and HSL with each hue method), hard stops, out-of-range offsets and the stop limit; pixels of each shape and spread method against CPU-computed colours within a colour step; two-point conical cases; degenerate gradients; a transformed gradient space; gradients through masks, strokes, clips and layers; batching. The editor draws linear, radial and conic gradients.

### G2j plan (decided 2026-10-18)

- **Upstream's model, as its Skia backend draws it:** a shadow is drawing state (`setDropShadow` with an offset, a blur radius and a colour), and each fill after it draws a blurred, offset, coloured copy of its shape beneath the shape. The blur is a Gaussian with a standard deviation of half the radius, as CSS defines it; the legacy radius mode (canvas, `-webkit-box-shadow`) is ignored, as Skia ignores it. Offset and blur are in the current coordinates, or in target pixels when shadows ignore transforms (canvas). `box-shadow` fills the box's shape beside the clip with the shadow offset back into it (outer shadows), or fills a rect with the box's inner shape as a hole inside a clip to it (inset shadows), so blurred rects, rounded rects and rounded holes cover every CSS shadow.
- **API:** `SetDropShadow(DropShadow2D {Offset, BlurRadius, Colour})`, `ClearDropShadow()` and `SetShadowsIgnoreTransforms(bool)`, drawing state that `Save` and `Restore` keep. A shadow with no offset and no blur, or a transparent colour, draws nothing.
- **Analytic blurs:** rects, rounded rects (circular corners) and rects with rounded holes blur in the shader: a rect exactly (a product of `erf`s), a rounded rect with the usual approximation (exact across, Gaussian-weighted samples down), and a rect with a hole as the rect's blur less the hole's (blurring is linear). Every `box-shadow` and the shadows of straight strokes are one instance each, at any blur, with no passes.
- **Blurred masks:** paths, path strokes and rounded rects with elliptical corners blur their coverage mask on the CPU with three box blurs (close to a Gaussian, as Skia's CPU blur does), cached in the mask atlas under the mask's key and the blur. Glyphs (G3) will too.
- **Rejected for now:** blurring on the GPU (coverage drawn offscreen, then two blur passes). It's the only way to blur what only the GPU holds, images and gradients, and CSS `filter` (`blur()`, `drop-shadow()`) needs it anyway, so it comes with filters. Until then images cast no shadows, and gradients' shadows use their shapes' coverage as if they were opaque.
- **Drawing:** a shadow is its own instance, drawn just before its shape with the same clip, layer and blend mode, usually in the same batch. Shadow instances have a `Shadow` flag and their standard deviation in fields they don't otherwise use; their quads grow by three standard deviations. When shadows ignore transforms, the offset and the blur are mapped back through the inverse transform, as Skia maps them.
- **Tests:** a blurred rect against a convolution worked out on the CPU, within a step; rounded rects against numerical integration, within a few steps; rounded holes; blurred path masks against a CPU Gaussian; offsets, ignoring transforms, clips, layers, `Save`/`Restore` and batching. The editor draws cards with outer and inset shadows and a star with a shadow.

### G2k plan (decided 2026-10-18)

- **Infrastructure:** `stb_image` (in `third-party/stb`) reads PNGs and `stb_image_write` (added beside it) writes them. A test helper renders a scene offscreen, reads it back and compares it with a PNG in `Krystal.Gfx.Tests/golden/`, within a per-channel tolerance and a small share of pixels allowed past it; on a mismatch, it writes the actual image and a difference image into the build directory. `KRYS_UPDATE_GOLDENS=1` writes the goldens instead (a switch of the test harness, not of the renderer).
- **Scenes:** a few UI-like scenes that together use everything G2 has: cards with rounded corners, borders, gradients and shadows; clips, scrolling and layers; blend modes; strokes and paths. They're G2's "UI-like scene".
- **References (option C of three):** each scene has a golden of Render2D's own output, reviewed by eye before it's committed, checked with tight tolerances (regressions); and a golden of Chrome's rendering of the same scene written in HTML and CSS, captured by a `tools/` script with headless Chrome at a device pixel ratio of 1, checked with looser tolerances and a report of the largest differences (fidelity). Rejected: either alone.

### T1 plan (decided 2026-10-18)

- **Why:** Render2D draws glyphs from the faces Booey shapes with. Rather than open each font twice, the faces move down into a module both depend on. Shaping stays in Booey: `FontCascade` and `ComplexTextController` are a port of upstream's text system, tied to CSS (fallback through `FontCache`, features, spacing), and the editor's UI will be built with Booey, so nothing needs a shaper outside it yet. A plain run shaper can join Krystal.Text when something outside Booey needs one. Core stays free of FreeType and HarfBuzz.
- **`Krystal.Text`** (namespace `krys::text`, which Core's text helpers already use): `FontFileData` and `FreeTypeFace` (`FreeTypeFace.hpp`), and `HbUniquePtr` (`HarfBuzz.hpp`, with its deleters), moved from Booey's `Graphics/`. It links FreeType and HarfBuzz; the face keeps its HarfBuzz face.
- **Font selection stays in Booey:** `FontSelectionCapabilities` is CSS's (from upstream's `FontSelectionAlgorithm`), so the face reports the font's own style instead (`FaceStyle`: its OS/2 weight, its width as a percentage and whether it's italic or oblique), and Booey builds the capabilities from it.
- **Sizes:** faces stay unsized, as Booey measures in font units. A user that needs a size (Render2D, G3) makes its own FreeType size objects, so sizes never change under another user.
- **Booey** uses the moved classes as `text::FreeTypeFace`, `text::FontFileData` and `text::HbUniquePtr`; the ledger records the move. Booey's tests of the face move to `Krystal.Text.Tests`, which reads Liberation Sans from `third-party/liberation-fonts`.

### G3 plan (decided 2026-10-18)

- **Out:** `Text/FontAtlas.hpp`, `Text/FontAtlasLoader.hpp` and `.cpp` (the MSDF atlas, unused), the vendored `Krystal.Gfx/third-party/msdf-atlas-gen`, and `msdfgen` from `vcpkg.json` and Gfx's CMake (and `vcpkg.json`'s second `freetype`).
- **Faces:** Render2D draws from Krystal.Text's faces (T1). `FontFace2D` wraps one with how it's drawn: hinting (slight by default, snapping vertically only, as fontconfig's default has Linux browsers draw; none and full as options), and synthetic bold (`FT_Outline_Embolden`) and oblique (a shear), as Booey's `FontPlatformData` asks for them. It makes its own FreeType sizes.
- **Rasterising:** FreeType's own rasteriser (exact-area antialiasing, hinting), 8-bit greyscale; colour glyphs (`FT_LOAD_COLOR`) are RGBA. Rejected: Render2D's `PathRasteriser` from the outlines (no hinting; sixteenths down each pixel).
- **Positions:** horizontally to a quarter of a pixel (four variants of a glyph, as Skia has), vertically to whole pixels. Small text under a scale, rotation or skew is rasterised under it (`FT_Set_Transform`), cached under its linear part as masks are; text over about 256 pixels is drawn as paths (`FontFace2D::GlyphPath`, which stroked text uses too) through the mask atlas, as Skia does.
- **The glyph atlas:** separate from the mask atlas, as decided, with R8 pages and RGBA pages for colour glyphs; the shelf packing and page eviction move out of `MaskAtlas` into a helper both share. Glyphs are keyed by face, glyph, size, subpixel position and the transform's linear part.
- **API:** `DrawGlyphs(const FontFace2D &, float size, Span<const uint32> glyphs, Span<const Vec2f> positions, Vec2f origin, const Paint2D &)`, positions rather than advances (upstream's Skia backend turns advances into positions first). Glyphs are mask instances, so paints, clips, layers and blend modes apply; colour glyphs are drawn as images. Shadows (`text-shadow`) blur glyph masks as path masks are blurred, cached per glyph. For tests and the editor until G4: `GlyphIndex(codepoint)` and `Advance(glyph, size)`, enough for left-to-right labels without shaping.
- **Not yet:** the gamma and contrast adjustments Skia makes to text coverage (raw coverage first, a TODO, compared against Chrome later), LCD antialiasing, variable font axes.
- **Parts:** G3a: the removals, `FontFace2D`, rasterising, outlines, the shared atlas pages and the glyph atlas, CPU only. G3b: `DrawGlyphs`, colour glyphs, transformed and large text, shadows, a golden text scene (against Chrome, loosely and reported: Chrome on Windows draws text through DirectWrite) and the editor's labels.
- **Tests:** coverage against FreeType's own output, the subpixel variants, outlines, atlas packing and eviction; drawn glyphs at whole and fractional positions, gradient, clipped and transformed text, large text, text shadows; the golden scene.

### G4 plan (decided 2026-10-18)

Booey has layout (block, inline, flex, grid, table), the layer tree, fonts holding `text::FreeTypeFace`s, `Path` with `PathStream`, `Gradient`, `FloatRoundedRect` and the colour types; it has no painting, no hit testing and no host. Upstream's sizes: `GraphicsContext` and its state about 1.4k lines, `displaylists/` about 1.8k (67 item types), `BackgroundPainter` 1.1k, `BorderPainter` and `BorderShape` 1.7k, `OutlinePainter` 0.7k, the text painters (`TextBoxPainter`, `TextPainter`, `TextDecorationPainter`, `InlineBoxPainter`) 2.5k, and the painting and hit testing in `RenderLayer.cpp` (7k in all).

- **Decisions:**
  - Booey ports upstream's display list (the item types, `DisplayList`, the recorder), trimmed to the items in *Layer 3*; the rest are FIXMEs.
  - No repaint rects and no compositing at first: any change (style, layout, hover) marks the page dirty; a dirty page is painted into a new display list, and every frame replays the last one. The repaint FIXMEs stay until a damage-tracking part.
  - Hit testing and clicks are part of G4 (G4f).
  - A device scale factor of 1; Booey keeps its scale plumbing, HiDPI comes later.
  - The host module is Krystal.UI (open decision 1).
  - Out of G4 (FIXMEs in Booey): images, `border-image` and `mask-image` (resource loading), `filter` and `backdrop-filter` (Render2D has no GPU filters), scrolling input and scrollbars, selection and carets, SVG, form controls.
- **Parts:**
  - **G4a, the graphics context and display lists** (Booey): `GraphicsContext` with its state (fill and stroke, shadows, composite and blend, alpha, clips), `NullGraphicsContext`, `DisplayList`, the recorder and the items boxes and text need; `Path` backed by `PathStream`.
  - **G4b, Krystal.UI and replay**: a new module on Booey and Gfx. Replays a display list into Render2D, mostly one call per item (`Path` to `Path2D` through its segments, `Gradient` to `Gradient2D`, colours as they are, fonts cached as `FontFace2D`s keyed by face and synthetic bold and oblique, shadows to `SetDropShadow`, transparency layers to `BeginLayer`). A `Page` host loads HTML from a string or a file, sets the viewport, lays out, paints and replays. The editor shows a page.
  - **G4c1, paint plumbing, layers and backgrounds** (Booey): `PaintPhase`, `PaintInfo`, the renderers' `paint`, `paintObject` and `paintChildren`, `RenderLayer::paintLayer` (stacking order, opacity layers, overflow clips, transforms, `z-index`, positioned content), `LocalFrameView::paintContents` and `RenderView`'s painting, and `BackgroundPainter` for colours and gradients.
  - **G4c2, borders, shadows and outlines** (Booey): `BorderPainter`, `BorderShape` and `BorderEdge` (every style, rounded corners, multi-coloured sides through side clips), outer and inset box shadows, `OutlinePainter` and focus rings.
  - **G4c3, image buffers and patterns** (Booey, Krystal.UI, Render2D): `ImageBuffer` with a display list backend, `GraphicsContext::drawImageBuffer` and `drawPattern`, gradient tiles drawn as upstream (rendered once into an image buffer, then filled as a pattern); replay renders image buffers into textures, and Render2D fills with a repeating image.
  - **G4d1, text and inline boxes** (Booey): `InlineContentPainter`, `TextBoxPainter`, `TextPainter`, `FontCascade::drawText` and `drawGlyphBuffer`, `text-shadow`, `InlineBoxPainter`, atomic inlines.
  - **G4d2, decorations, markers and ellipses** (Booey): `TextDecorationPainter` with skip-ink, emphasis marks, list markers, ellipses, inline outlines.
  - **G4e, the other boxes** (Booey): flex, grid and table painting, collapsed table borders, placeholders for replaced content.
  - **G4f1, hit testing, hover and active** (Booey and Krystal.UI): `HitTestLocation`, `HitTestResult`, `RenderLayer::hitTest`, `nodeAtPoint` for boxes, inline content, flex, grid and tables, `clip-path` and radii; hover and active state.
  - **G4f2, mouse events and clicks** (Booey, Krystal.UI and Krystal.PAL): `MouseEvent`, the mouse half of `EventHandler`, click counting, cursors; Krystal.UI routes the window's mouse to the page.
  - **G4g, focus and blur** (Booey, Krystal.UI and Krystal.PAL): focus on click, the window's focus (see *G4g plan*).
  - **G4h1, keyboard events, and G4h2, sequential focus navigation** (Booey, Krystal.UI and Krystal.PAL; see *G4h plan*).
- **Tests:** the Booey parts compare display list dumps of small documents (as layout compares render tree dumps); the whole pipeline is checked with page goldens, HTML drawn through Krystal.UI and compared with Chrome's rendering (`tools/golden/capture_chrome.py`, extended to pages): strictly for boxes, loosely for text. The Booey parts follow the porting checklist and get ledger entries in `docs/booey/PORTING.md`.

### G4b plan (decided 2026-10-18)

- **Module:** Krystal.UI (namespace `krys::ui`), on Booey and Gfx; the editor links it.
- **Replay:** a display list is replayed as upstream replays one, by applying its items to a `GraphicsContext`: `ui::GraphicsContextRender2D` draws through Render2D, modelled on upstream's Skia backend (`GraphicsContextSkia`, `FontCascadeSkia`). Brushes become `Paint2D`s (colours to 8-bit sRGB, `Gradient` to `Gradient2D` field for field), multiplied by the context's alpha as Skia does; drop shadows go to `SetDropShadow` and `SetShadowsIgnoreTransforms`; thickness, caps, joins, dashes and the mitre limit to `StrokeStyle`; paths to `Path2D` from their segments (arcs, ellipses and rounded rects included); transforms, clips and clip-outs to Render2D's; transparency layers to `BeginLayer`. `drawLine` (dotted and dashed borders), `drawRect`, text lines and document marker underlines follow Skia's. A fill with a rounded hole and a drop shadow draws only its shadow, as Skia's does (inset shadows).
- **Rounded rects:** Render2D's are circular per corner; elliptical corners are filled and clipped as paths.
- **Glyphs:** a `FontFaceCache` maps a Booey `Font` to a `FontFace2D` (by its `text::FreeTypeFace` and synthetic bold and oblique), the size from its platform data; advances become pen positions; vertical text is rotated as Skia does.
- **Not supported:** compositing operators other than source-over (Render2D has blend modes, not Porter-Duff operators; FIXMEs), stroked text (ignored).
- **Page host:** `ui::Page` initialises Booey's value pool, parses HTML from a string or a file, creates the frame and its view at the viewport size, resizes, and brings style and layout up to date. `Paint` records the page into a display list and replays it into a `Renderer2D`; until G4c1 ports painting a page records nothing.
- **Editor:** a page view showing a display list recorded by hand through Booey's `GraphicsContext` (boxes, a shadowed rounded card, a gradient, dashed and dotted lines, an underline, a clipped path, a translucent layer, glyphs); a page once G4c1 paints.
- **Tests (`Krystal.UI.Tests`, using Krystal.Gfx.Tests' `TestDevice`):** each item replayed and drawn directly with Render2D give the same pixels; the path, gradient, colour and stroke conversions without a GPU; a page's loading, layout at the viewport size and relayout on resize.

### G4c1 plan (decided 2026-10-18)

One part (decided by the user). Booey already has the layer tree and its z-order lists, clip rects, layer fragments, `PaintPhase`, `BorderShape`'s geometry, `clip-path` shapes as paths and `Style::CreatePlatformGradient`.

- **Paint plumbing:** `PaintInfo`; the renderers' painting (`RenderObject`, `RenderElement::paintAsInlineBlock`, `RenderBox::paintBoxDecorations`, `RenderBlock`'s `paint`, `paintObject`, `paintContents`, `paintChildren` and `paintChild`, `RenderBlockFlow::paintFloats`, `RenderView`'s `paint` and `paintBoxDecorations` with the base background); flex and grid's `paintChildren`; a thin `LocalFrameView::PaintContents` painting the render view's layer over a dirty rect. Tables wait for G4e, inline content for G4d.
- **Layer painting** (the paint half of `RenderLayer.cpp`): `paint`, `paintLayer`, `paintLayerWithEffects`, `paintLayerContents`, `paintList`, `collectFragments`, the background, foreground and outline passes per fragment, `paintLayerByApplyingTransform`, `beginTransparencyLayers` (opacity, `mix-blend-mode`, `isolation`), `clipToRect` (overflow clips, rounded by ancestors' radii) and `setupClipPath` (basic shapes and `path()`; `url()` waits for SVG).
- **Backgrounds:** `BackgroundPainter` for colours and gradients: fill layers, `background-clip`, `-origin`, `-size`, `-position` and `-repeat`, the root's background, rounded clips. Box shadows (also in `BackgroundPainter`) wait for G4c2.
- **`BorderShape`'s drawing half:** clipping to and filling its outer and inner shapes.
- **Generated images:** `Image` (drawing only), `GeneratedImage` and `GradientImage`, so gradient backgrounds go `StyleGradientImage::image` → `GradientImage` → `drawTiledImage` as upstream's do. Repeated tiles are filled one by one with the gradient, clipped to the destination, up to a cap (decided): upstream renders a tile into an image buffer and fills with it as a pattern, which waits for images (FIXME).
- **Out (FIXMEs):** filters, `backdrop-filter`, `mask-image`, reflections, compositing, scrollbars and resizers, selection, carets, outlines and focus rings (G4c2), event regions, bitmap images.
- **Repainting:** a change records the whole page again; the dirty rect is the viewport.
- **Editor:** Tab cycles three views, the first a live page loaded from an HTML file in the repository (F5 reloads it), then the Render2D dashboard and the hand-drawn Booey scene.
- **Tests:** Booey compares display list dumps of small documents (paint order, opacity layers, transforms, overflow, rounded and `clip-path` clips, blend modes, background geometry); Krystal.UI gains page goldens, HTML scenes drawn through `Page` and Render2D against Chrome's rendering (`capture_chrome.py` takes a scene directory), strictly since they have no text or borders yet.

### G4c2 plan (decided 2026-10-19)

One part. G4c1 left `BorderShape`, `BorderEdge` and bleed avoidance in place; visual overflow already includes shadows and outlines.

- **Borders:** `BorderPainter` (`paintBorder` with its fast paths for all-solid simple edges, the per-side painting with corner mitre polygons, translucent sides grouped by colour, `double`, `groove`, `ridge`, `inset` and `outset`, dotted and dashed sides along rounded paths, `drawLineForBoxSide`, the style colours), wired into `RenderBox::PaintBoxDecorations` (under the background with `BackgroundOverBorder`, over it otherwise).
- **Border images** (decided): `NinePieceImagePainter` and `Image::drawTiled` with the stretch, repeat, round and space rules, so `border-image` with generated (gradient) sources paints; bitmap sources wait for images.
- **Box shadows:** `BackgroundPainter::paintBoxShadow`, outer (blur, spread, offset, rounded, clipped out of the box) and inset, painted before and after the background as upstream.
- **Outlines:** `OutlinePainter::paintOutline` for boxes (every style, `outline-offset`) and `RenderElement::PaintOutline`, in `paintObject`'s outline steps and the layers' outline pass. `outline-style: auto` follows upstream's non-Cocoa behaviour (decided): the outline colour, drawn as a focus ring around the rects; the shrink-wrapped path (`FloatPointGraph`, `PathUtilities`) and the theme's focus ring colour come with inline outlines.
- **Later:** inline and continuation outlines (G4d, with line boxes), collapsed table borders (G4e); column rules (no multi-column layout), fieldset legend gaps and `mask-border` are FIXMEs.
- **Tests:** Booey display list dumps (each border style and corner join, translucent and rounded borders, border images, shadow geometry, outlines); a `borders` page golden against Chrome (focus rings report-only, as Chrome's are two-toned); the editor page gains borders, shadows and outlines.

### G4c3 plan (decided 2026-10-19)

Replaces G4c1's shortcut of filling each repeated gradient tile with its own gradient, which differs from Chrome in two ways the editor page shows: hard colour stops are aliased (Chrome samples a rendered tile, which softens them), and where two layers of hard-stopped tiles meet, pixel centres exactly on a stop can fall outside both (dotted seams). Upstream renders the gradient once into an image buffer and fills with it as a pattern, so every tile's pixels are the same.

- **Booey:** `ImageBuffer` (the parts painting uses: creation from a context, `context()`, the logical and backend sizes, the resolution scale, the rendering resource identifier) with a display list backend, as the display list `ImageBuffer` upstream records into; `GraphicsContext::createImageBuffer`, `createAlignedImageBuffer`, `drawImageBuffer` and `drawPattern(ImageBuffer&)`; the display list items `DrawImageBuffer` and `DrawPatternImageBuffer` (holding the buffer); `GradientImage::drawPattern` as upstream (the cached buffer at the context's scale, `Gradient::adjustParametersForTiledDrawing`). G4c1's per-tile fills, the tile cap and G4c2's whole-unit tile clips go.
- **Render2D:** an image pattern paint (a texture's source rect repeated under a transform, with a phase and spacing between tiles, sampled linearly or nearest), usable wherever paints are (rects, rounded rects, paths); and drawing into a texture during a frame, as a pass that runs before the passes sampling it (layers already work this way, but end with the frame).
- **Krystal.UI:** replay renders each image buffer's display list into a texture (once per buffer, kept while the buffer lives), then draws it with `DrawImage` or the pattern paint.
- **Out (FIXMEs):** bitmap images (`<img>`, `url()` backgrounds and border images) reuse this later, with decoding and resource loading; `NativeImage` and its items; accelerated (GPU) backends of `ImageBuffer` beyond the display list one; luminance masks.
- **Editor:** the border image demo moves to a source that isn't a sliced diagonal stripe pattern (which breaks at every tile join, in Chrome too).
- **Tests:** Booey dumps of tiled gradients and border images (the image buffer and pattern items); Render2D image pattern tests (repeat, phase, spacing, transforms, sampling) against direct per-tile drawing; the `backgrounds` and `border-images` page goldens checked against Chrome again, the latter strictly if the tiling now matches.

### G4d plan (decided 2026-10-19)

Two parts (decided by the user). Booey has the modern inline layout (the inline formatting context, its display boxes, `InlineContent` and the inline iterators), shaping (`FontCascade`, `WidthIterator`, `ComplexTextController` with HarfBuzz, `GlyphBuffer`), the bundled Liberation Sans and the text style values (`TextShadow` among them); nothing paints inline content yet. Krystal.UI already replays glyphs, text lines and drop shadows, so the parts are mostly Booey's.

- **G4d1, text and inline boxes:**
  - **Inline painting:** `LineLayout::paint` and `InlineContentPainter` (the display boxes in order, damage culled to the paint rect), from `RenderBlockFlow::paintInlineChildren` and the layers' painting of `RenderInline`s.
  - **Text:** `TextBoxPainter` (the background and foreground phases, the text origin, combined and vertical text), `TextPainter`, `TextPaintStyle` and `MarkedText` and `StyledMarkedText`, unmarked only: selection, document markers and highlights make no marks (FIXMEs, as G4 leaves them out).
  - **Drawing:** `FontCascade::drawText` and `drawGlyphBuffer`, `GraphicsContext::drawText`, through the existing glyph item.
  - **`text-shadow`:** every shadow, blurred and offset, through the context's drop shadow as upstream.
  - **`InlineBoxPainter`:** inline boxes' backgrounds, borders and box shadows, split across lines with their slices.
  - **Atomic inlines:** inline blocks and replaced placeholders paint in line order (`paintAsInlineBlock`).
  - **Krystal.UI:** `Page` registers font directories; the editor and the goldens register `third-party/liberation-fonts`, so bold and italic have their own faces.
- **G4d2, decorations, markers and ellipses:**
  - **`TextDecorationPainter`:** underlines, overlines and line-throughs in every style (wavy included), thickness and offset, decorations' shadows.
  - **`text-decoration-skip-ink`** (decided: now, not a FIXME): `FontCascade::lineSegmentsForIntersectionsWithRect` with `Font::pathForGlyph` (the glyph's FreeType outline as a `Path`) and the glyph iteration state, so underlines and overlines break around descenders.
  - **Emphasis marks:** `FontCascade::drawEmphasisMarks`.
  - **List markers:** `RenderListMarker::paint` (disc, circle, square and counters as text).
  - **Ellipses:** `EllipsisBoxPainter`, for `text-overflow: ellipsis`.
  - **Inline outlines:** inlines' focus ring and outline rects from their line boxes, continuation outlines (left from G4c2).
- **Tests:** Booey compares display list dumps of small documents (text runs and glyph positions, mixed fonts, bidi, shadows, inline boxes across lines; decorations, skip-ink, markers and ellipses in G4d2); Krystal.UI adds a `text` page golden in Liberation Sans, its Chrome comparison reported only (Chrome on Windows rasterises with DirectWrite, as for the G3b scene).
- **Editor:** the page gains text (headings, paragraphs, inline spans with backgrounds and borders, text shadows) in G4d1, and decorations, a list and an ellipsis in G4d2.

### G4e plan (decided 2026-10-19)

One part (decided by the user). Flex and grid already paint their items in order (G4c1), and no element makes a replaced renderer yet (images, media, canvases and controls come later), so the part is tables. Table layout is ported, with the cells' collapsed border values and their validity.

- **Table:** `RenderTable::paint` and `paintObject` (captions and sections in order, then the collapsed borders phase), `paintBoxDecorations`, and the collapsed borders collected and sorted by precedence for painting.
- **Sections:** `paint` and `paintObject` over the rows and columns the paint rect reaches (`dirtiedRows`, `dirtiedColumns`) and the cells overflowing their rows, `paintCell`, and the row group borders in the collapsed model.
- **Cells:** `paint`, `paintBoxDecorations`, `paintBackgroundsBehindCell` (column groups, columns, row groups and rows paint their backgrounds through their cells) and `paintCollapsedBorders`.
- **Rows:** `paint` and their outlines; columns paint nothing themselves.
- **Out (FIXMEs):** masks, selection, hit testing, fieldset legends' border gaps; `RenderReplaced::paint` waits for images (decided), when there's replaced content to paint.
- **Tests:** Booey compares dumps of small tables (separated borders and `border-spacing`, collapsed borders and their conflict resolution by width, style and owner, backgrounds of tables, column groups, columns, row groups, rows and cells, `empty-cells: hide`, spans, captions, row outlines); Krystal.UI adds a `tables` page golden of sized cells without text (decided), checked strictly against Chrome.
- **Editor:** the page gains a table card (collapsed coloured borders, striped rows, a spanning cell).

### G4f plan (decided 2026-10-19)

Two parts (decided by the user). Booey has `HitTestRequest`, `HitTestSource`, `Document::updateHoverActiveState` (the `:hover` and `:active` styles), event dispatch to native listeners and `UIEvent`; it has no `HitTestLocation`, `HitTestResult`, renderer hit testing, `MouseEvent` or `EventHandler` (upstream's is 5.7k lines, of which only the mouse handling is wanted). Krystal.PAL reports mouse input; the editor doesn't use it yet.

- **G4f1, hit testing, hover and active:**
  - **Booey:** `HitTestLocation` (points and rects) and `HitTestResult` (the inner node, the URL element, the local point, the node list of rect tests); `RenderLayer::hitTest` and the layer walk (z-order lists, transforms, clips, the foreground and background passes); `nodeAtPoint` for boxes and blocks (floats and children), inline content (`LineLayout::hitTest` and the inline content's boxes, so text and inline boxes hit), flex and grid items (`hitTestChildren`) and tables (tables, sections, rows and cells); the view and the document (`Document::hitTest`, `elementFromPoint`). `pointer-events` and `visibility` are respected, and so are border radii and `clip-path` (decided), with `Path::contains`.
  - **Hover and active:** a point hit tests and updates the hover and active chains through `updateHoverActiveState`, which restyles.
  - **Krystal.UI:** `Page::HitTest` gives the element at a point.
  - **Tests:** Booey compares hit test results of small documents (the topmost box, `z-index`, transforms, rounded corners, `clip-path`, `pointer-events: none`, text and inline boxes across lines, table cells, flex and grid items) and hover restyles.
  - **Editor:** the mouse moving over the page applies `:hover` styles.
- **G4f2, mouse events and clicks:**
  - **Booey:** `MouseEvent`, `MouseRelatedEvent` and `MouseEventInit`; a trimmed `EventHandler` (mouse move, down and up; `mouseover`, `mouseout`, `mouseenter` and `mouseleave` as the hovered element changes; `click` and `dblclick` with click counting; the active state while a button is down) fed by a `PlatformMouseEvent` that Krystal.UI fills.
  - **Cursors** (decided): the CSS `cursor` of the hovered element sets the window's cursor, through a new Krystal.PAL cursor call.
  - **Krystal.UI:** routes the window's mouse events to the page, in page coordinates.
  - **Out (FIXMEs):** focus and blur on click (decided: later), pointer events, drag and drop, selection by dragging, wheel scrolling, context menus, scripts (listeners are native).
  - **Tests:** the order and targets of move, over, out, enter, leave, down, up, click and dblclick, recorded by listeners; the cursor chosen for an element.
  - **Editor:** a native click listener toggling a class on the page.

### G4g plan (decided 2026-10-19)

One part (decided by the user); keyboard events and sequential focus navigation follow as G4h. Booey has the focus processing model (`FocusAlgorithms`: the focusing, unfocusing and focus update steps, `FireFocusEvent` with `focusin` and `focusout`, `FocusForClick`), `FocusEvent`, `tabindex`, `focus()` and `blur()`, the `:focus`, `:focus-visible` and `:focus-within` state with its restyles, `Document::SetFocusedAndActive` and `outline: auto` focus rings (G4c2); nothing calls them on a click, and Krystal.PAL doesn't report the window's focus.

- **Booey:** a `mousedown` that isn't cancelled focuses the nearest click focusable element under the mouse (or the viewport) through `FocusForClick`, in the event handler (the old element gets `blur` and `focusout`, the new one `focus` and `focusin`, between `mousedown` and `mouseup`); a cancelled `mousedown` leaves the focus. A click focus isn't focus visible (upstream: except text fields), so it shows `:focus` without a ring. The window losing or gaining system focus sets the document's focused and active flag (restyling `:focus`) and fires `blur` or `focus` at the focused element, which stays the active element (FIXME: at the `Window`, once there is one).
- **Krystal.PAL:** `WindowCallbacks::OnFocusChange`, from `WM_SETFOCUS` and `WM_KILLFOCUS` on Win32.
- **Krystal.UI:** `Page::SetFocused(bool)`; a page starts focused.
- **Out (FIXMEs):** keyboard events and sequential navigation (G4h), the focus navigation starting point, focus visible after keyboard use, `autofocus`, events at the `Window`, text fields and form controls.
- **Tests:** Booey: the order and targets of the mouse and focus events, focus moving to the nearest focusable ancestor, a press on nothing focusable clearing it, a cancelled `mousedown`, `:focus` restyling and `:focus-visible` staying off after a click, the window losing and regaining focus; Krystal.UI: a click focus and `SetFocused` through `Page`. No golden: nothing new paints.
- **Editor:** the cards take a `tabindex` and a `:focus` style, the stage a `:focus-within` style; the window's focus reaches the page.

### G4h plan (decided 2026-10-19)

Two parts (decided by the user). Booey has `UIEventWithKeyState` (G4f2) and the focus processing model (`FocusAlgorithms`, with `TabIndexValue`); it has no `KeyboardEvent`, `PlatformKeyboardEvent`, keyboard half of the event handler or sequential focus navigation. Krystal.PAL reports a `Key` and its state, without modifiers or the text the key types, and no Alt combinations (`WM_SYSKEYDOWN`); its mouse events have no modifiers either.

- **Krystal.UI's `Page` becomes `View`** (decided by the user): Krystal.UI will have its own concepts, closer to a game engine's UI, and a view shows a Booey page. Krystal.UI depends on Krystal.PAL (decided by the user) and takes its input types: a view maps `pal::Key` to the DOM's `code`, `key` and Windows key code, takes `pal::MouseButton` (the editor's `ToBooeyButton` goes) and gives its cursor as a `pal::CursorShape` (the editor's `ToCursorShape` goes).
- **G4h1, keyboard events:**
  - **Booey:** `PlatformKeyboardEvent` (raw key down, char and key up, with the text, `key`, `code`, the Windows key code, auto repeat, keypad and modifiers); `KeyboardEvent` and `KeyboardEventInit` (`key`, `code`, `location`, `repeat`, the modifiers, the legacy `keyCode`, `charCode` and `which`); the keyboard half of `EventHandler`: `keydown`, then `keypress` for keys that type text, then `keyup`, at the focused element or the body; a cancelled `keydown` suppresses `keypress`. A key press makes the focused element focus visible, as upstream.
  - **Krystal.PAL:** modifiers captured when a message arrives, on the key and mouse callbacks and events; the text a key types, from `ToUnicodeEx` on key down without changing the dead key state; `WM_SYSKEYDOWN` and `WM_SYSKEYUP`.
  - **Krystal.UI:** `View::KeyPressed` and `KeyReleased` from PAL's key events; the mouse calls take PAL's buttons and modifiers.
  - **Out (FIXMEs):** text editing and input events, IME and composition, access keys, the menu and context menu keys, activation by Space and Enter (links and form controls).
  - **Tests:** Booey: `KeyboardEvent` from a dictionary and from the platform; the order and targets of `keydown`, `keypress` and `keyup`, cancelling, focus visible after a key. Krystal.UI: key events and the key mapping through `View`.
  - **Editor:** forwards PAL's events to the view; a native key listener shows the last key pressed on the page; Escape still quits.
- **G4h2, sequential focus navigation:**
  - **Booey:** Tab and Shift+Tab move the focus as the default action of a `keydown` that isn't cancelled, in the specification's sequential navigation order, written from the specification in `FocusAlgorithms` (decided), with upstream's `FocusController` as the reference for what the specification leaves to user agents: tabindex-ordered focus navigation scopes (positive `tabindex` first, then tree order, `-1` skipped; shadow hosts, slots and, once there are popovers, open popovers as scopes; a shadow host without `tabindex` counts as 0); the focus navigation starting point, which a click sets; focus visible when moved by keyboard, so `outline: auto` rings show; wrapping at the end of the document (upstream, with no chrome to take the focus). No port of `FocusController`: it belongs to a `Page` (Booey has none yet) and duplicates the focusing steps.
  - **Out (FIXMEs):** frames, spatial navigation, keyboard focusability that differs from focusability (`tabsToLinks`, form controls), moving the focus out to the host, `autofocus`.
  - **Tests:** the navigation order (positive `tabindex`, tree order, `-1`, shadow trees and slots, wrapping, Shift+Tab), the starting point after a click, focus visible after Tab; a page golden of a focus ring.
  - **Editor:** Tab moves through the cards with a focus ring.

### G2a as built (2026-10-18)

- `Render2D/Rect2D.hpp`: `Rect2D`, a plain `{X, Y, Width, Height}` in pixels.
- `Render2D/Renderer2D.hpp`: `Renderer2D::Create(device)`, then per frame `Begin(commands, Render2DTarget)`, `FillRect(rect, SRGBA<uint8>)`, `End()`. The target is a swapchain or a texture with its size, format and an optional clear colour; `End` records one render pass with every draw in it.
- Rects are instanced: one 20-byte record each (rect, colour) in transient memory, drawn as a 4-vertex strip whose corners come from `gl_VertexID`, so there's no quad vertex buffer. A frame's rects are one draw, in painter's order. Empty and fully transparent rects are skipped.
- One pipeline per target format, created on first use. The shaders are embedded in `Renderer2D.cpp` until the shader decision (open decision 2).
- Colours are sRGB with straight alpha, premultiplied in the shader. On sRGB targets they're decoded and blend in linear light; on other targets they blend as encoded (open decision 4).
- If the frame's transient memory runs out, the frame's rects are dropped (with an assert in debug builds); growing the ring is left for later.
- Tests: `Krystal.Gfx.Tests/tests/Render2D/Renderer2D.cpp` (5 cases). The GPU tests share `CreateTestDevice` (`include/Krystal.Gfx.Tests/TestDevice.hpp`). The editor's triangle is replaced by rects drawn with Render2D.

### G2b as built (2026-10-18)

- `Render2D/RoundedRect2D.hpp`: `CornerRadii2D` (one circular radius per corner, `Uniform(r)`) and `RoundedRect2D`. `FillRoundedRect(rect, colour)`; `FillRect` is a rounded rect with zero radii, so every rect goes through the same shader and draw.
- Radii are constrained on the CPU as CSS does for `border-radius`: negative radii become zero, and if two adjacent radii are longer than their side, every radius is scaled down by the same factor.
- The fragment shader computes the signed distance to the rounded box and covers `clamp(0.5 - distance, 0, 1)` of the pixel: a one-pixel band centred on the edge. (Changed in G2f2: away from rounded corners, coverage is now the product of the vertical and horizontal edges' coverages; see *G2f2 as built*.) Edges on pixel boundaries stay sharp; edges between pixels are blended. The quad is a pixel larger than the rect on every side so the band outside the edge is drawn.
- The instance is 36 bytes (rect, radii, colour).
- Limits: corners are circular (CSS's elliptical radii, `10px / 20px`, are decided in G2e); rects thinner than a pixel come out somewhat heavier than their true coverage; the distance is in target pixels, so transforms (G2c) will need the antialiasing width scaled.
- Tests: two more cases in `tests/Render2D/Renderer2D.cpp` (edges between pixels; rounded corners, constrained radii, per-corner and negative radii). The editor's panels are rounded.

### G2c as built (2026-10-18)

- `Render2D/Transform2D.hpp`: `Transform2D`, a 2D affine transform laid out as CSS's `matrix(a, b, c, d, e, f)`, with `Identity`, `Translation`, `Scale`, `Rotation` (clockwise on screen, since y points down), composition (`T * U` applies `U` first), `Apply`, `Determinant` and `PreservesAxisAlignment`. Points are Core's `Vec2f`.
- Drawing state is a stack of (transform, clip): `Save`, `Restore`, `ConcatTransform`, `Translate`, `Scale`, `Rotate`, `SetTransform`, `GetTransform`. Each frame starts with the identity transform and the whole target as the clip; unbalanced `Save`/`Restore` asserts.
- Every instance carries its own transform (the instance grows to 60 bytes), so draws under different transforms (every scrolled element, for one) still go out in one draw; a batch only breaks when the clip changes.
- Antialiasing stays one target pixel wide under any transform: the shader measures the distance in local units and divides by how far it grows per target pixel, computed exactly from the inverse transform and the edge's normal. That factor is exactly 1 without a transform, so pixel-aligned edges stay sharp. (Screen-space derivatives were tried on paper and rejected: they smear by 1 to 2/255 at square corners.) The quad is grown by one target pixel in local units along each axis.
- `ClipRect` intersects the clip with a rect in the current coordinates and becomes the scissor, in whole target pixels: a pixel is inside when its centre is. Clip edges are hard until G2g. Under rotations and skews the clip is the rect's bounding box (a debug assert and a FIXME until G2g's clip masks). (Superseded in G2g1: see *G2g1 as built*.)
- Draws under an empty clip or a singular transform are skipped.
- Tests: `tests/Render2D/Transform2D.cpp` (3 cases); 3 more cases in `tests/Render2D/Renderer2D.cpp` (translations, scales, quarter turns; antialiasing under scale and a 45-degree turn; clip rounding, nesting, transforms, `Restore`, empty clips). The editor adds a clipped, scrolling list in the sidebar and a spinning square.

### G2d as built (2026-10-18)

- `DrawImage(texture, source, destination, ImageOptions)` and `DrawImage(texture, destination)` for the whole texture. The source is in texels with a top-left origin. `ImageOptions`: `Filter` (linear, with linear mipmaps when the texture has them, or nearest), `Alpha` (`AlphaPremultiplication`, premultiplied by default) and `Opacity`.
- One pipeline and one shader still draw everything: an image is a rounded-rect instance (with zero radii for now) that multiplies its colour by a texel. The instance gains texture coordinates and flags (textured, straight alpha, linear texels) and is 80 bytes. An image's colour is white with the opacity as alpha, so tints come for free later (masks and glyphs need them).
- Batches break on a change of clip or texture (and sampler). Untextured instances join any batch with the same clip, and a textured one adopts a batch that has no texture yet, so rects drawn between images of the same texture don't break the batch. Batches without a texture bind a 1-by-1 white texture when nothing else is bound: they never sample it, but drivers check every declared sampler's texture at each draw (NVIDIA warns, 131204).
- Texels are clamped half a texel inside the source rect, so linear filtering never reads the texels around a sub-image (atlases). Texture derivatives are taken outside the textured branch, where every fragment of a quad runs them, and sampled with `textureGrad`.
- Colours follow the colour path (open decision 4): textures in sRGB formats are decoded by the GPU; others hold sRGB-encoded values; the shader converts texels to the target's blending space (unpremultiplying around the conversion). Premultiplied data in an sRGB-format texture isn't exact, since the GPU decodes each channel before the alpha division; premultiplied images belong in unorm formats.
- Tests: five more cases in `tests/Render2D/Renderer2D.cpp` (nearest and linear scaling; clamping to the source rect; premultiplied, straight and opacity; sRGB texture into a unorm target and unorm texture into an sRGB target; images and rects interleaved across textures, in painter's order, under a transform). The editor generates a 16-by-16 checkered gradient (there are no image assets) and draws it scaled with nearest filtering and turning with linear filtering.

### G2e1 as built (2026-10-18)

- `Render2D/Path2D.hpp`: `FillRule` (`NonZero`, `EvenOdd`), `PathVerb` and `Path2D` (verbs and points; `MoveTo`, `LineTo`, `QuadTo`, `CubicTo`, `Close`; `AddRect`, `AddRoundedRect` with elliptical radii constrained as CSS does, `AddEllipse`, `AddArc`; equality for the mask cache). A segment without a current point starts a contour, and after `Close` the next one continues from the closed contour's start, as in canvas. Arcs are one cubic per quarter turn or less.
- `Render2D/PathRasteriser.hpp`: `PathRasteriser::Rasterise(path, transform, rule, clip)` returns a `CoverageMask` (its `PixelRect2D` bounds and a byte per pixel). It flattens the transformed path (quadratics and cubics are split evenly, with the segment count from the curve's largest second difference; at most 1024 lines a curve), clips the mask to the pixels the edges touch within the clip, and samples 16 sub-scanlines per row with an active edge list. Edges outside the clip still count towards the winding. Paths with non-finite points cover nothing. The rasteriser keeps its buffers between paths.
- `PixelRect2D` (in `Rect2D.hpp`): a rect of whole pixels; the renderer's own pixel rect moves onto it in G2e2.
- Tests: `tests/Render2D/Path2D.cpp` (4 cases) and `tests/Render2D/PathRasteriser.cpp` (6 cases), CPU only: whole and partial pixels (exact across a row, in sixteenths down it, diagonal edges), both fill rules with nested and reversed contours, curve areas against exact formulas (a parabola, circles and an ellipse, elliptical rounded corners, arcs) within the curved length times the tolerance, transforms, clipping, and empty and invalid paths.

### G2e2 as built (2026-10-18)

- `Render2D/MaskAtlas.hpp`: `MaskAtlas::GetMask(path, transform, rule, clip)` returns a `MaskLocation` (an R8 texture, the mask's texels in it, and the target pixels it covers), rasterising and caching the mask when it isn't cached. `MaskAtlasDesc` (in `Renderer2DDesc::Masks`): `PageSize` (1024), `MaxPages` (4), `MaxPackedSize` (256).
- The cache key is the path, the fill rule, the transform's linear part and the fractional part of its translation; the mask is rasterised with the whole-pixel part removed and placed by it when drawn, so whole-pixel moves reuse it. Masks aren't clipped; the clip is the scissor when they're drawn, so one mask serves every clip. Masks that cover nothing are cached too.
- Pages are packed into shelves whose heights are rounded up to 8 pixels; a mask takes the lowest shelf that fits, or opens a new one when the best is more than twice its height. Masks are drawn with nearest sampling at one texel per pixel, clamped inside their texels, so they need no padding.
- Reclaiming space, with `Device::FrameNumber` deciding what's safe: when every page is allocated and full, the least recently used page that the current frame hasn't drawn from is cleared, since overwriting texels that unsubmitted draws read would change them. If the current frame has drawn from every page, or the mask is larger than `MaxPackedSize`, it gets a texture of its own, destroyed once a whole frame passes without it. Masks whose control points span more than the device's largest texture (or 4096 pixels) are rasterised within the clip only, into a texture destroyed the next frame.
- Known limit: a page is only cleared whole, so a page with one mask drawn every frame keeps its stale masks (a turning path's old masks, say) until the atlas is destroyed. Freeing single masks, or compacting, can come with profiling.
- `Renderer2D::FillPath(path, rule, colour)` draws the mask as an untransformed instance on whole pixels with the `Mask` flag: the shader multiplies the colour by the texel's red channel, with no colour conversion. `Renderer2D::Create` takes a `Renderer2DDesc`; `GetStats()` returns a `Renderer2DStats` for the last frame (instances, batches, masks rasterised, mask cache hits). The renderer's clip and scissor rects are now `PixelRect2D`.
- Tests: four more cases in `tests/Render2D/Renderer2D.cpp` (pixels equal to the rasteriser's coverage; fill rules, transforms with half-pixel offsets, clips and painter's order; the cache across draws, frames, whole-pixel moves, fractional moves, fill rules and paths; eviction, masks in their own textures when the only page is in use, and masks larger than `MaxPackedSize`, with one 16-by-16 page). The editor fills a pentagram under both rules, a panel with elliptical corners, and a turning star.

### G2e3 as built (2026-10-18)

- `Renderer2D::FillRectWithRoundedHole(rect, hole, colour)`: the hole is a `RoundedRect2D` (circular corners), with its radii constrained like `FillRoundedRect`'s; an empty hole fills the whole rect. The shader multiplies the rect's coverage by one minus the hole's, each from its own signed distance and each converted to target pixels, so square corners on whole pixels stay exact. The instance gains the hole's rect and radii as fields of their own (112 bytes), rather than reusing others.
- Render2D has no elliptical-corner type. A rounded rect or hole with elliptical corners is drawn by its caller as a path: `AddRoundedRect` with the elliptical radii (and the outer rect, under `FillRule::EvenOdd`, for a hole) and `FillPath`. The Booey host (planned, G4) picks the analytic call when every corner is circular and the path otherwise.
- Tests: one more case in `tests/Render2D/Renderer2D.cpp` (square holes exact on whole pixels; a circular hole's antialiased coverage at two pixels, against values worked out by hand; constrained radii; holes under a transform; empty holes). The editor frames the image with a rounded hole and draws an elliptical border as a path.

### G2f1 as built (2026-10-18)

- `Render2D/StrokeStyle.hpp`: `LineCap`, `LineJoin` and `StrokeStyle` (`Width`, `Cap`, `Join`, `MiterLimit` of 10, `Dashes`, `DashOffset`).
- `Render2D/PathStroker.hpp`: `PathStroker::Stroke(path, style, transform)` returns the stroke's outline as a `Path2D` in the path's coordinates, to fill under non-zero with the same transform. Every piece is a polygon of its own, wound the same way: a rect along each line, a join at each corner (the style's join at the path's own corners, a bevel between the lines of a flattened curve, where the turn is too small to tell), and caps or dots at open ends. The rasteriser counts windings rather than adding areas, so overlapping pieces merge exactly. Round joins and caps are whole circles; the half inside the stroke changes nothing.
- Flattening: the transform's largest stretch turns the rasteriser's tolerance into the path's units, and curves are split until both the curve and the stroke's outer edge are within it (the turn per line is at most the square root of 8 times the tolerance over half the width). `Render2D/Flattening.hpp` holds the curve maths the rasteriser and the stroker share.
- Dashes are cut by length along the flattened contour, restarting on each contour; a closed contour's pattern runs across its closing segment, and its first and last dashes stay separate. More than 100,000 dashes on a contour strokes it solid. Zero-length contours (a line to the same point, or a move and a close) are dots under round and square caps; a lone move draws nothing. A hairline's width is one over the square root of the transform's determinant, exact for rotations and uniform scales.
- Tests: `tests/Render2D/PathStroker.cpp` (6 cases), comparing the stroke's rasterised area with exact values: caps; miter, bevel and round joins, the miter limit at a right angle and an open corner; dash patterns, odd patterns, offsets either way, dots, invalid patterns and a closed contour; rings, including a stroke wider than its circle is tight; strokes under a scale and hairlines; zero-length contours.

### G2f2 as built (2026-10-18)

- `Renderer2D::StrokePath(path, style, colour)` strokes with the renderer's `PathStroker` under the current transform and fills the outline with `FillPath` under non-zero, so the mask cache applies to the outline. The outline is worked out on every call, before the cache is looked up; caching it by path and style can come with profiling.
- `Renderer2D::StrokeLine(from, to, style, colour)` draws a straight line without a mask: in the line's own coordinates (x along it, y across it, under the current transform) each dash is a rect, extended by half the width under square and round caps and rounded by half the width under round caps, so round dots are circles and square dots squares. A zero-length line lies along x; joins and the miter limit don't apply. Hairlines are one target pixel wide, as in the stroker.
- `Render2D/Dashing.hpp`: the pattern rules (`PrepareDashes`, `StartDashes`, `DashPosition`, the 100,000-dash limit) shared by the stroker and `StrokeLine`, so both cut dashes the same way, including a dash that starts exactly where a line ends.
- **Analytic coverage at corners (changed here).** Comparing `StrokeLine` with `StrokePath` showed the shader covering half of a pixel that a square corner cuts in half both ways, where its area is a quarter: the distance to the nearest edge is 0 at the corner. Away from rounded corners the shader now multiplies the coverage of the box's vertical edges by that of its horizontal edges, which is exact for axis-aligned boxes (as Skia does for rects); around rounded corners it still uses the distance to the arc. Straight edges are unchanged, so pixel-aligned edges stay exact. On curves and diagonals the shader's ramp over the distance still differs from the true area by up to about a tenth of a pixel's coverage.
- Tests: four more cases in `tests/Render2D/Renderer2D.cpp` (a stroked path; straight lines with butt caps, hairlines, transforms and dots; `StrokeLine` against `StrokePath` for butt, square and round caps, dashes, dots and a diagonal, pixel by pixel within each antialiasing method's difference), and a quarter-covered corner in the partial-pixel case. The editor's stroke sample: the three joins on a zigzag, a wave of cubics, a dotted line, a dashed box, a dashed circle whose pattern slides round it, and a turning dashed line.

### G2g1 as built (2026-10-18)

- `ClipRect` under any transform, `ClipRoundedRect` (radii constrained as `FillRoundedRect`'s), `ClipOutRect` and `ClipOutRoundedRect`. The clip state is the scissor rect plus a clip chain: an index into the frame's chains, each the shapes in effect, each with its rounded rect, the inverse of the transform it was made under (`Transform2D::Inverse`, new) and whether it clips out. Adding a clip copies the current chain with the new shape, so `Save` and `Restore` only save the index. At most 16 shapes a chain; more asserts and the extra clip is dropped.
- A `ClipRect` whose edges land on whole pixels under a transform that keeps rects axis-aligned is the scissor alone, as before. Any other clip is a shape in the chain, and a clip-in also shrinks the scissor to the pixels its antialiased edge reaches. Clipping to an empty rect, or under a transform that flattens the plane, clips everything; clipping out an empty rect changes nothing. Fractional `ClipRect` edges are now antialiased rather than rounded to pixel centres.
- The vertex shader passes each fragment's target pixel; the fragment shader maps it into each clip's coordinates and multiplies the draw's coverage by the shape's coverage (one minus it for a clip-out), with the same coverage function as fills, so a clip to a shape matches a fill of it. Every batch binds its chain as a uniform block at slot 1 (`ClipUniforms`, 1040 bytes, std140, mirrored with `static_assert`s), an empty one included, since the shader always reads it; a batch breaks when the chain changes. Images, masks and strokes are clipped like fills.
- Tests: four more cases and one rewritten section in `tests/Render2D/Renderer2D.cpp`: clipping to a rounded rect against filling it, a rotated clip against a rotated fill, clipping out a rounded rect against `FillRectWithRoundedHole`, clipping out a rect; nesting with `Save` and `Restore` across a fill and an image; paths and lines clipped; empty clips; fractional clip edges antialiased. The editor rounds the sidebar list's clip, draws a drop shadow clipped out of the image's frame, and scrolls the image behind a turning rounded clip.

### G2g2 as built (2026-10-18)

- `ClipPath(path, rule)` and `ClipOutPath(path, rule)`. A chain keeps its path clips (path, transform, rule, in or out) and their coverage as one mask: a single path clip uses the path's cached mask from the atlas; a second nested one multiplies all the chain's path clips (one minus the coverage for clip-outs) on the CPU over the clip's bounds into a mask uploaded for this frame (`MaskAtlas::UploadTransient`, new), which is made again every frame. A clip-in also shrinks the scissor to the mask's bounds. Clipping to an empty path, or one that covers nothing, clips everything; clipping one out changes nothing.
- The clip block gains the mask's mode (none, to, out), its bounds in target pixels and its first texel (48 bytes before the shapes, 1072 in all). Masks are a texel per target pixel, so the shader reads the fragment's own texel with `texelFetch`; outside the mask's bounds it covers nothing. The mask is on texture unit 1, which always has a texture (the white one when there's no mask), since drivers check every declared sampler.
- Tests: two more cases in `tests/Render2D/Renderer2D.cpp`: pixels against the rasteriser's coverage when clipping to a path, out of one, and to two nested paths (to and out), where they multiply; a path clip with a rounded clip, even-odd and a transform; `Restore`; empty paths. The editor scrolls the image behind a turning star-shaped clip and cuts a circle out of a panel.

### G2h1 as built (2026-10-18)

- `BeginLayer(opacity)` saves the drawing state and starts a layer; `EndLayer()` restores it and adds the layer's composite to the parent, in painter's order. `End` closes any layer left open (asserting in debug builds), and `EndLayer` asserts when the saves and restores inside a layer don't match.
- Every batch records the layer it draws into, and only joins batches of the same layer. A layer's bounds grow with each draw's reach (its quad's pixels, grown by one for the antialiased edge, within the clip); at `End`, in the order layers ended (children first), each layer's bounds are cut to its clip at `BeginLayer` and added to its parent's, and a layer that shows nothing (opacity 0, empty bounds, or composited under an empty clip) gets no texture or pass.
- The composite is an instance and batch added at `EndLayer` and filled in at `End`: the layer's texels one to one at the layer's bounds, premultiplied, white at the layer's opacity, sampled nearest, with `LinearTexels` on sRGB targets (the layer holds premultiplied linear colours encoded per channel, which decode exactly). It keeps the parent's scissor but no shape or path clips: the layer's draws were already clipped, and clipping again would fade antialiased clip edges twice.
- Passes: one per layer that shows, in the order layers ended, cleared to transparent, then the target's. Every pass draws only its layer's batches from one instance upload, with its own frame uniforms: the viewport is the pass's texture and the new `u_Origin` is the target pixel at its top-left, so instances, scissors (offset by the origin) and clips stay in target pixels.
- Layer textures have the target's format, are rounded up to 64 pixels, and come from a pool: a texture this frame hasn't used and large enough is reused, the smallest first; textures unused for 120 frames are destroyed.
- Tests: two more cases in `tests/Render2D/Renderer2D.cpp`: group opacity (overlapping opaque draws in a half-opaque layer show only the top one at half); nesting (a quarter of red over blue) and painter's order around layers; empty and transparent layers; and an opaque layer with a rounded clip, translucent fills and antialiased edges drawing the same as without the layer, on unorm and sRGB targets. The editor sets two translucent circles beside two opaque ones in a layer whose opacity pulses.

### G2h2 as built (2026-10-18)

- **Encoded blending:** the shaders no longer decode colours: colours are premultiplied as they are, the frame block loses `u_DecodeSrgb` (16 bytes: viewport size and origin), and `Begin` asserts the target isn't in an sRGB format. `Render2DTarget::Format` defaults to `RGBA8Unorm`, and the editor's swapchain is `RGBA8Unorm`. For an image in an sRGB format (`LinearTexels`), the shader encodes each decoded channel back before premultiplying a straight image, so premultiplied sRGB images are now exact too. Layer composites never set `LinearTexels`.
- **`Krystal.Core/Colour/BlendMode.hpp`:** `BlendMode` and `BlendModeName`, moved from Booey's `Graphics/GraphicsTypes.hpp`, which keeps `using krys::BlendMode;` so `boo::BlendMode` (written by the generated style code) still names it.
- **API:** `SetBlendMode`, `GetBlendMode`, `BeginLayer(opacity)` (with the current blend mode) and `BeginLayer(opacity, blend)`; the blend mode is in `DrawState`, and a layer starts with `Normal`. `PlusDarker` asserts and becomes `Normal`.
- **Instances and batches:** the blend mode is in bits 8 to 12 of the instance's flags. A batch has one blend mode; for a mode that reads the backdrop (`CopiesBackdrop`: everything but `Normal`, `PlusLighter` and `PlusDarker`) it also keeps `Backdrop`, the union of its instances' reach, and an instance joins only when its reach misses that union, and never a layer's composite. The reach (the quad's pixels grown by one, within the clip) is now `Reach`, shared with layer bounds; a composite's backdrop is its layer's bounds within its scissor, filled in by `FinishLayer`.
- **Passes:** `End` sizes one backdrop texture (from the layer pool) to the largest pass that copies. In `RecordPass`, a batch that copies ends the pass, blits its backdrop (within its scissor, in pass pixels) from the pass's texture to the same place in the backdrop, counts it (`Renderer2DStats::BackdropCopies`), resumes the pass with `Load` and binds everything again. The backdrop is always bound on unit 2 (the white texture when there's none). `PlusLighter` batches use a second pipeline per format, blending `ONE, ONE`.
- **Swapchains:** when a batch drawn straight into a swapchain target copies, the target's pass draws into a pooled texture the target's size, cleared to the clear colour or transparent, and a last pass (`WindowLayer`) composites it into the window, cleared to transparent or kept. Fixed later (2026-10-18, found in the editor): that pass mapped pixels by the window's size while the viewport covered the pooled texture, which is rounded up, so everything drawn in it was stretched down against its scissors and clip masks; a pass into a texture now takes the texture's size.
- **Shader:** the fragment shader un-premultiplies the source and the backdrop texel at `floor(v_Pixel - u_Origin)`, applies the blend function (`Blend`, with `HardLight`, `ColorDodge`, `ColorBurn`, `SoftLight`, `Lum`, `ClipColour`, `SetLum`, `Sat` and `SetSat`, as Compositing and Blending Level 1 writes them), mixes it with the source colour by the backdrop's alpha and premultiplies by the source's alpha; normal blending adds the rest.
- **Known limits:** each batch that copies costs a pass split and a copy, and overlapping draws with a blend mode copy once each; reach is a bounding box, so draws near each other may not share a copy. Top-level blends on a swapchain target don't see what was in the window before the frame (see *G2h2 plan*).
- Tests: three more cases in `tests/Render2D/Renderer2D.cpp`, against the specification's formulas computed on the CPU (with `SetSat` written as the specification sorts the channels): every mode from `Multiply` to `Luminosity` over four colour pairs, opaque and translucent, sharing one copy, with untouched pixels unchanged; overlapping draws copying twice and multiplying twice, antialiased edges at half coverage, `PlusLighter` without a copy, and `Save`/`Restore` of the blend mode; a blend inside a layer offset from the target's corner, and layers composited with `Multiply` (from the state) and `Screen` at half opacity. Changed: half white over black is now 128 (it was 188 on an sRGB target); images in sRGB formats come back as stored when opaque, premultiplied and straight; the opaque-layer comparison runs on unorm only. The editor bobs four circles over the image in `multiply`, `screen`, `difference` and `color`, and composites two opaque squares in a `multiply` layer.

### G2i as built (2026-10-18)

- **`Krystal.Core/Colour/GradientSpreadMethod.hpp`:** `GradientSpreadMethod` (`Pad`, `Reflect`, `Repeat`), moved from Booey's `Graphics/GraphicsTypes.hpp`, which keeps `using krys::GradientSpreadMethod;`.
- **`Render2D/Gradient2D.hpp`:** `LinearGradient2D`, `RadialGradient2D` and `ConicGradient2D` (upstream's data, named as upstream names it: `Point0`, `Point1`, `StartRadius`, `EndRadius`, `AspectRatio`, `AngleRadians`), `GradientStop2D` (`Offset`, `ExtendedSRGBA<float>` with straight alpha) and `Gradient2D` (`Shape`, `Stops`, `Spread`, `Interpolation`, defaulting to sRGB with premultiplied alpha; `IsZeroSize`).
- **Flattening** (`detail::FlattenGradientStops`, in `Gradient2D.cpp`): stops are stable-sorted, their offsets clamped to 0 to 1 (NaN is 0). Each span between stops is sampled at 513 points with Core's `InterpolateColourComponents` in the gradient's space and converted back to sRGB, clamped and premultiplied; it's halved while any sample strays from the line by more than `GradientTolerance` (a quarter of a step), and a single sample interval is halved by its midpoint up to 8 more times, which finds the kinks where colours are clamped into sRGB. sRGB spans with premultiplied alpha (or one alpha), and stops within sRGB, aren't sampled: they're linear already. Of three or more stops at one offset, only the first and last are kept. More than `MaxGradientStops` (256) and the tolerance doubles until they fit; more than that unsplit, and the gradient is sampled at 256 even offsets. `EvaluateFlatStops` and `AverageFlatStops` read the result.
- **`Render2D/Paint2D.hpp`:** `Paint2D`, built implicitly from an `SRGBA<uint8>` or a `Gradient2D` (with an optional gradient-space transform). It refers to the gradient rather than copying it; the renderer is done with it when the draw returns.
- **API:** `FillRect`, `FillRoundedRect`, `FillRectWithRoundedHole`, `FillPath`, `StrokePath` and `StrokeLine` take a `const Paint2D &`. A line's gradient is in the current coordinates, not the line's.
- **Resolving a paint** (`ResolvePaint`): a gradient is flattened; no stops draws nothing and one stop is that colour. The shape's own space folds into the gradient space and the transform, and the inverse of that maps target pixels into it: a linear gradient's line runs along x from 0 to 1, a radial gradient's first centre is the origin (an aspect ratio other than 1 scales y by its inverse), and a conic gradient is turned by its angle less a quarter turn. A gradient space that flattens the plane draws nothing. Degenerate shapes follow Skia's thresholds (1/32768): a line of no length, or coinciding circles, is the last colour (pad) or the average (reflect, repeat), except coinciding circles with a radius, padded, which become a radial gradient from the centre that's the first colour to the radius and the last beyond. The frame keeps `GradientData` and the flattened stops; a draw whose gradient matches the last one, stops and map alike, shares it.
- **Batches:** instances painted by a gradient have a white colour and the `GradientPaint` flag (bit 5); `Batch::Gradient` names the frame's gradient. A gradient instance joins a batch with its gradient or none; solid instances join any. `RecordPass` binds a gradient's block (`GradientUniforms`, std140, 5,184 bytes: shape, spread, stop count, the inverse map, the radial geometry, 256 offsets and 256 colours) on uniform slot 2 when the batch's gradient changes.
- **Shader:** `GradientColour` maps the pixel into the shape's space and finds `t`: x for linear; for radial, the roots of the two-point conical equation (as `q / a` and `c / q`, so neither loses precision as `a` nears 0), the larger whose radius isn't negative, or transparent; for conic, the angle from x, clockwise. Then spread (`fract` for repeat, a triangle wave for reflect), a binary search for the last stop at or before `t`, and a linear mix with the next. The colour gets the ordered dither (`Bayer`, the bits of `x ^ y` and `x` interleaved and reversed) scaled by `u_Dither` and its alpha, and multiplies the instance's colour before coverage. The frame block grows to 32 bytes with `u_Dither`: a step for `R8`, `RG8`, `RGBA8` and `BGRA8Unorm` targets, 0 otherwise.
- **Accuracy:** flattened colours are within a quarter of a step of the interpolation at the samples; between them, at clamping kinks, they've measured up to about half a step (0.44, in OKLCH the long way round), so the tests allow half a step.
- **Known limits:** gradients are flattened on every draw, so a gradient in another space costs 513 interpolations per span per draw; caching them is an optimisation for later. The edges of a radial gradient's cone, where no circle reaches, aren't antialiased (Skia's aren't either). Dithering is always on for 8-bit targets.
- Tests: `tests/Render2D/Gradient2D.cpp` (7 cases): premultiplied sRGB flattens to its own stops; twelve other interpolations (straight sRGB, sRGB-linear, OKLab, Lab, OKLCH both ways, HSL with each hue method, HWB, Display P3) within the tolerance, at offsets other than the samples; hard stops; sorting, clamping and padding; the stop limit; averages; `IsZeroSize`. In `tests/Render2D/Renderer2D.cpp` (9 cases), pixels against greys worked out on the CPU, within a step: linear gradients with each spread method, down and along a reversed diagonal; hard stops; radial gradients (one centre, padded and repeated; elliptical; a focal point inside; a larger start circle, reflected; a strip with nothing beside it); conic gradients from three angles; the transform and the gradient space together, and a gradient space that flattens; paths, lines, path strokes, a rounded clip and a layer away from the corner; degenerate gradients, one stop and none; batching; the dither's proportions (a quarter of a step above 100 gives about a quarter of the pixels at 101; the exact count depends on how the GPU rounds) and none on solid colours. The editor draws a turning linear gradient, a radial gradient with a circling focal point, a turning conic gradient with a hard stop, a bar in OKLCH the long way round and a subtle grey bar.

### G2j as built (2026-10-18)

- **API:** `DropShadow2D` (`Offset`, `BlurRadius`, `Colour`) in `Renderer2D.hpp`; `SetDropShadow`, `ClearDropShadow`, `GetDropShadow`, `SetShadowsIgnoreTransforms` and `GetShadowsIgnoreTransforms`. Both are in `DrawState`, so `Save` and `Restore` keep them. Fills (`FillRect`, `FillRoundedRect`, `FillRectWithRoundedHole`, `FillPath`) and strokes (`StrokePath`, `StrokeLine`) cast shadows; `DrawImage` doesn't.
- **Placing a shadow** (`PlaceShadow`): nothing without a shadow, with a transparent result, with a non-finite offset, or with neither offset nor blur. The colour's alpha is multiplied by a solid paint's (a gradient counts as opaque). The standard deviation is half the blur radius; unless shadows ignore transforms, the offset is mapped by the transform's linear part and the deviation scaled by its average scale (the square root of its determinant) into target pixels. The shadow's transform is the shape's moved by the offset in target pixels, and its deviation in the shape's own units is the pixel deviation over the shape transform's average scale.
- **Analytic shadows:** `AddRoundedRect` (fills and each dash of `StrokeLine`) and `FillRectWithRoundedHole` add the shadow instance before the shape's: the same rect, radii and hole, the shadow's colour, the `Shadow` flag (bit 6) and the deviation in `UV[0]`, or no flag at all for a sharp shadow, which is the shape moved. The vertex shader grows a shadow's quad by three deviations, and `Reach` (batches' backdrops, layers' bounds) by as much. The fragment shader's `BlurredBox` takes a plain box exactly (the Gaussian's mass across times its mass down, with `Erf` to Abramowitz and Stegun's 7.1.26, within 1.5e-7) and a rounded box in 16 bands of rows within four deviations, each weighted by the Gaussian's mass over it and as wide as the box is halfway down it; a hole subtracts its own blur.
- **Mask shadows:** `FillPath` (and so `StrokePath`) gets the path's mask under the shadow's transform with a blur (`MaskAtlas::GetMask`'s new `blur` argument, part of the cache key), adds it through `AddMask` before the shape's, and then the shape's; the shadow comes first because it can show where the shape doesn't (as `box-shadow` draws it). Masks are blurred by `detail::BlurCoverage` (`Render2D/CoverageBlur.hpp`): below a deviation of 8 pixels, a separable kernel out to three deviations whose weights are the Gaussian's mass over each pixel (exact for coverage even across each pixel, so within a step and a half of the true blur); from 8, three box blurs sized as SVG's `feGaussianBlur` sizes them, measured within 6 to 10 steps of the Gaussian (the plan had boxes from 2, but they're that far off there too, and a kernel of 49 taps is cheap). Masks too large for a texture are rasterised as far around the clip as the blur reaches into it.
- **Known limits:** shadows of gradients are their shapes' as if opaque, and images cast none, until blurring on the GPU comes with filters. Under a transform that stretches unevenly, mask shadows blur by the average scale rather than in the shape's own coordinates as analytic ones do. A mask shadow is cached under its fractional position like any mask, so a shadow moving by fractions of a pixel (the editor's star) is rasterised and blurred every frame. The analytic rounded box is an approximation: within 2 steps of numerical integration in the tests.
- Tests: `tests/Render2D/CoverageBlur.cpp` (3 cases): extents and bounds; totals kept; within a step and a half of the exact blur of a square below a deviation of 8, and within 8 steps with boxes at 10. In `tests/Render2D/Renderer2D.cpp` (5 cases), alpha against the exact blur worked out on the CPU: a rect at three blurs within a step, a rounded rect at three within 2 steps (against rows integrated exactly across); an inset shadow through a rounded hole, drawn as `box-shadow: inset` is; sharp shadows beneath their shapes, in one batch; a translucent paint's shadow; offsets in the current coordinates and in target pixels; `Save`/`Restore`, `ClearDropShadow` and shadows that show nothing; no shadow for images; path shadows at two blurs within 2 steps, and a path stroke's shadow matching a straight line's; shadows clipped and inside a layer. The editor draws a card lifting on its shadow, a well with an inset shadow and a star whose shadow swings round it.

### G2k as built (2026-10-18)

- **`third-party/stb/stb_image_write.h`** (v1.16, nothings/stb at `f58f558`), built into the `stb` library with `stb_image_write.cpp`; Krystal.Gfx.Tests links `stb`.
- **`Krystal.Gfx.Tests/Golden.hpp`** (`src/Golden.cpp`): `GoldenImage`, `LoadPng`, `SavePng`, `CompareImages` (the largest channel difference, the pixels past a tolerance, the mean of each pixel's largest difference), `DifferenceImage` (differences times 4, on black), `GoldenDirectory` (`KRYS_GFX_TESTS_GOLDEN_DIR`, the source tree's `Krystal.Gfx.Tests/golden`), `GoldenOutputDirectory` (`golden-output` beside the test executable) and `UpdatingGoldens` (`KRYS_UPDATE_GOLDENS=1`).
- **Scenes** (`tests/Render2D/Golden.cpp`, tagged `[Golden]`), each also written as HTML in `golden/scenes/`: `shadows` (320 by 240: soft, sharp and wide outer shadows and an inset shadow, painted as `box-shadow` paints them: a shape moved out of view casting the shadow, clipped out of the box (a pixel inside an opaque box's edge, as upstream's painter does, so the shadow carries on under the edge's antialiasing) or clipped to it) and `ui` (360 by 260: a linear-gradient header, a card with a radial-gradient background and a rounded border, a conic gradient in a circle, a turned square, a rounded `overflow: hidden` clip, a group at half opacity, `mix-blend-mode: multiply` and an SVG polyline with round joins). The scenes paint opaque backgrounds, so premultiplied and straight pixels compare alike.
- **Checks:** each scene renders offscreen and is written to `golden-output/<name>.actual.png`. Against Render2D's golden (`<name>.png`, written when missing or updating, with a warning to review it), no pixel may differ by more than 2 (a difference image is written when one does). Against Chrome's (`<name>.chrome.png`), the comparison is reported, `<name>.chrome-diff.png` written, and at most 0.5% of pixels may differ by more than 24, with a mean under 1.5.
- **`tools/golden/capture_chrome.py`:** runs headless Chrome (`--headless=new --disable-gpu --force-device-scale-factor=1 --force-color-profile=srgb`, a throwaway profile) on each scene at the size its `<html data-width data-height>` gives, into `golden/<name>.chrome.png`.
- **Fidelity, as first measured:** `shadows` is within 2 to 4 steps of Chrome away from the boxes' edges (soft, sharp, wide and inset shadows alike; the inset one half a step darker on average), mean 0.39, largest 25 (curved edge pixels). `ui` matches gradients, the clip, the layer, the blend and the stroke within a step or so away from edges, mean 0.25, largest 81: on curved edges where Chrome antialiases twice (its rounded border is a clip and a fill, so an edge pixel covered 0.38 shows 0.38 squared), where Render2D's single coverage is closer to the true area. Chrome stands in for WebKit, whose painters Booey ports; such differences come from painting, not from Render2D.
- **For G4:** the Booey host's `box-shadow` painting needs upstream's rule above (clip out a pixel inside an opaque box's edge), or a light seam shows along every shadowed box's antialiased edge (it measured 39 steps in `shadows` before the scene had it).

### T1 as built (2026-10-18)

- **`Krystal.Text`** (static library, namespace `krys::text`, linking FreeType and HarfBuzz privately; Booey links it publicly): `FreeTypeFace.hpp` (`FontFileData`, `FaceStyle`, `FreeTypeFace`) and `HarfBuzz.hpp` (`HbUniquePtr` and its deleters), moved from Booey's `Graphics/` with their history (`src/FreeTypeFace.cpp`; `src/HarfBuzz.cpp` holds the deleters and `FreeTypeFace::HarfBuzzFace`, which were in Booey's `FontHarfBuzz.cpp`).
- **`FaceStyle`** replaces the face's `FontSelectionCapabilities`: `Weight` (the OS/2 weight class, clamped to 1 to 1,000, or 700 or 400 from the bold flag), `Width` (a percentage, from the OS/2 width class) and `Italic` (the italic flag or the OS/2 italic or oblique bits), read as before. Booey's `CapabilitiesOfFace` (`Graphics/FontDatabase.hpp`) builds the capabilities from it for `FontDatabase`, `FontCache` and `FontCascade`.
- **Booey** names the moved types `text::FreeTypeFace`, `text::FontFileData` and `text::HbUniquePtr`; its face test moved to Krystal.Text, and the bundled face's capabilities are checked through `CapabilitiesOfFace`.
- Tests: `Krystal.Text.Tests` (4 cases, 32 assertions) on Liberation Sans from `third-party/liberation-fonts`: names, style, units and no size set; character to glyph mapping; tables and the HarfBuzz face (the same face each time, with FreeType's glyph count and units per em); data that isn't a font, and a face index past the file's faces.

### G3a as built (2026-10-18)

- **Removed:** `Text/FontAtlas.hpp`, `Text/FontAtlasLoader.hpp` and `.cpp`, the vendored `Krystal.Gfx/third-party/msdf-atlas-gen`, `msdfgen` from `vcpkg.json` (with its second `freetype`) and Gfx's CMake. Gfx links Krystal.Text publicly and FreeType privately.
- **`Render2D/AtlasPages.hpp`:** the shelf packing and page eviction from `MaskAtlas`, unchanged in behaviour: `AtlasPagesDesc` (format, page size, most pages, name), `AtlasSlot`, `Allocate` (with a callback for a cleared page, so its owner forgets what was on it), `MarkUsed`, `Write`, `Texture`, `PageCount`, `Evictions`. `MaskAtlas` keeps its R8 pages in one.
- **`Render2D/FontFace2D.hpp`:** `FontHinting` (`None`, `Slight`, `Full`), `FontFace2DOptions` (hinting, slight by default; `SyntheticBold`, `SyntheticOblique`), `GlyphBitmap` (bounds relative to the pen's pixel, y down, the baseline at 0; coverage, or premultiplied RGBA for colour glyphs) and `FontFace2D`, made with `Create` from a Krystal.Text face. Each has its own FreeType size (`FT_New_Size`), activated and set before rasterising, and a process-unique `Id` for caches. `Rasterise(glyph, size, subpixel, linear)` loads with `FT_LOAD_TARGET_LIGHT` (slight), the default (full) or `FT_LOAD_NO_HINTING`; hinting is off under a transform or synthetic oblique. The subpixel offset and transform go through `FT_Set_Transform` (the transform flipped into FreeType's y-up space, the slant of a quarter applied first), which is cleared again since the face is shared. Synthetic bold emboldens the outline by a 24th of the em, as Skia does. `FT_Render_Glyph` makes 8-bit coverage; faces with colour (`FT_HAS_COLOR`) load with `FT_LOAD_COLOR` and give BGRA, swizzled to RGBA; others load without embedded bitmaps. `GlyphPath` decomposes the unhinted outline in font units into a `Path2D` scaled to the size (y down, with the synthetic styles); `Advance` is the unhinted advance; `GlyphIndex` maps a code point.
- **`Render2D/GlyphAtlas.hpp`:** `GlyphAtlasDesc` (page size; most R8 and RGBA pages), `GlyphLocation` (texture, texels, bounds, colour), `GlyphAtlasStats` and `GlyphAtlas`, separate from the mask atlas, with an `AtlasPages` for coverage and one for colour. `GetGlyph(face, glyph, size, subpixel, linear)` caches under the face's id, the glyph, the size, the quarter (`SubpixelPositions` is 4) and the linear part; glyphs without pixels are cached without space. Nothing comes back when FreeType can't load a glyph or there's no room (the caller draws a path). `SnapPen` splits a pen's x into its pixel and nearest quarter, rounding up into the next pixel.
- **Known limits:** colour glyphs from bitmap strikes (`CBDT`) come at their strike's size, unscaled; COLR glyphs (Segoe UI Emoji) render at any size. Faces aren't thread-safe. The glyph cache only forgets glyphs when their page is cleared.
- Tests: `tests/Render2D/FontFace2D.cpp` (7 cases, on Liberation Sans from `third-party/liberation-fonts`): coverage identical to FreeType's own rendering of three glyphs; on the baseline, a half-pixel offset moving the coverage by half a pixel, spaces empty, bad sizes refused; slight hinting snapping the H's top row (as dense as a stem row at 11 pixels, where unhinted it's partly covered) without moving it horizontally; synthetic bold thicker and oblique slanted by a quarter; quarter turns and scales; linear advances, and outlines covering what the bitmap does (area within 2%, same top-left); colour glyphs as RGBA (skipped without `C:/Windows/Fonts/seguiemj.ttf`). `tests/Render2D/GlyphAtlas.cpp` (4 cases): `SnapPen`; caching by quarter, size, transform and face, and spaces without textures; the page holding exactly the rasterised coverage; a full page refusing within a frame and clearing in a later one.

### G3b as built (2026-10-18)

- **API:** `Renderer2D::DrawGlyphs(face, size, glyphs, positions, origin, paint)`; `Renderer2DDesc::Glyphs` (`GlyphAtlasDesc`); `Renderer2DStats::GlyphsRasterised` and `GlyphCacheHits`; `LargeGlyphSize` (256). `FontFace2D` gains `HasColour`; `GlyphAtlas::GetGlyph` gains a blur, for shadows; instances gain the `AlphaMask` flag (bit 7: a texture's alpha as coverage).
- **Placing glyphs:** each pen (`origin` plus its position) goes through the transform into target pixels and snaps with `GlyphAtlas::SnapPen` across and to the nearest pixel down. Under a uniform positive scale, glyphs are rasterised at the scaled size with no transform, so hinting applies; under any other transform, at the size under the transform's linear part.
- **Instances:** a glyph is a pixel-aligned instance of its atlas texels, untransformed, sampled nearest: coverage as a mask (`Textured | Mask`, the paint's colour or gradient), a colour glyph as a premultiplied image (`Textured`, white at the paint's alpha; gradients don't tint it). Glyphs on one page share a batch.
- **Large text:** past `LargeGlyphSize` target pixels per em (and for faces without colour glyphs, which only come as bitmaps), each glyph's outline, made at the origin, is filled through the mask atlas under the transform moved to its pen (`FillPathMask`, which takes the geometry's transform apart from the paint's, so gradients stay in the current coordinates), shadows and all; the same glyph moved by whole pixels reuses its mask. A glyph the atlas has no room for is filled from its outline too.
- **Shadows:** drawn for the whole run first, from the glyphs' own bitmaps, as Skia draws them: each glyph's coverage (a colour glyph's alpha, through `AlphaMask`) in the shadow's colour, its pen moved by the offset. A blurred shadow is the glyph atlas's blurred glyph: the coverage blurred on the CPU (`BlurCoverage`, in target pixels) and cached under the blur, as coverage. Only a glyph with no room in the atlas casts its shadow from its outline.
- **Golden scene `text`** (360 by 200): Liberation Sans at 12, 16, 24 and 48 pixels and light on dark at 14, glyphs placed by advances (the HTML is SVG `<text>`, whose baseline is at `y`, with kerning and ligatures off, its font from the repository: `capture_chrome.py` now allows file access and waits for fonts). Against Chrome it's reported only (`Scene::ChromeReportOnly`): the glyphs land in the same places, but Chrome on Windows rasterises with DirectWrite, whose hinting and contrast make text heavier (mean 6.9, largest 221 on glyph edges). The contrast and gamma adjustments stay a TODO.
- **Editor:** Liberation Sans regular and synthetic bold (`KRYS_EDITOR_FONTS_DIR`); the header's title (with a text shadow) and links, the cards' titles and the tiles' labels are text, placed by advances until Booey draws the editor.
- **Known limits:** large colour glyphs stay bitmaps (up to a page); text's coverage isn't adjusted for contrast as Skia's is (see the golden scene).
- Tests: `tests/Render2D/Text.cpp` (Liberation Sans): a glyph on a whole pixel, at a fractional pen and under a uniform scale exactly as rasterised; a run in one batch, cached on the next frame; gradient, clipped and turned text (coverage kept within 3%); large text filled as an outline (one mask, coverage within 2% of the bitmap); sharp shadows as the glyph again, blurred ones exactly the glyph's coverage blurred, none for a transparent paint; large text moved by whole pixels reusing its mask while a gradient stays put; colour glyphs in their own colours, faded by the paint's alpha, casting sharp shadows as their silhouette and blurred ones keeping their alpha (within 1%; skipped without Segoe UI Emoji). 8 cases. `tests/Render2D/Golden.cpp` gains the `text` scene.

Testing: the device's hidden window gives tests a real OpenGL context (Krystal.Gfx.Tests). Render2D gets golden image tests: render offscreen, read back with `ReadTexture`, compare against a PNG with a tolerance.

### G4a as built (2026-10-18)

- Booey ports upstream's `GraphicsContext`, its state and state savers, `NullGraphicsContext` and the display lists (`DisplayList`, the items, `Recorder` and `RecorderImpl`) in `Graphics/` and `Graphics/DisplayLists/` (namespace `krys::boo::display_list`); the ledger entry is *Graphics context and display lists* in `docs/booey/PORTING.md`.
- 52 item types: state (save, restore, transforms, inline fill and stroke, the whole state, line caps, joins, dashes, mitre limit), clips (rect, rounded rect, clip-out, path, reset), fills (a rect with the current brush, a colour, a gradient or composited; a rounded rect; a rect with a rounded hole; a path; an ellipse), strokes (rect, path, ellipse), lines, text lines, document marker dots, focus rings, glyphs, nested display lists, placeholders, transparency layers, the device scale, pages and links. Image, filter and control items wait for images.
- Glyph items keep a `Ref` to Booey's `Font`, whose platform data holds the `text::FreeTypeFace` Render2D rasterises from, plus glyph IDs and advances; replay turns the advances into positions.
- Recording keeps upstream's state handling: changes are recorded just before the next drawing item, colour and thickness changes inline, anything else as the whole state, and changes back to what was last drawn with not at all.
- Display lists dump as text, in upstream's format, which the tests compare.

### G4b as built (2026-10-18)

- **Krystal.UI** (`krys::ui`, on Booey and Gfx; the editor links it): `Render2DConversions` (colours, rects, transforms, fill rules, caps and joins, rounded rects, paths and gradients), `FontFaceCache`, `GraphicsContextRender2D` and `Page`.
- **`GraphicsContextRender2D`** is a `boo::GraphicsContext` drawing through a `Renderer2D` (it saves the renderer's state on construction and restores it on destruction); a display list replays by applying its items to it. Each draw first brings the renderer's blend mode, shadows-ignore-transforms and drop shadow up to date with the context's state; paints are the brush multiplied by the context's alpha; the stroke's caps, joins, mitre limit (4 by default, as Skia's) and dashes (odd arrays repeated, invalid ones dropped) are kept in a stack of their own beside the context's, as Skia's backend keeps them. `drawLine` (filled corners and a dash pattern for dotted and dashed borders, no shadow), `drawRect` (a fill, then four side rects in the stroke colour, as the CG backend draws them; Skia strokes a region), `drawEllipse`, text lines (`ComputeRectsAndStrokeColorForLinesForText`'s rects) and the spelling and grammar underline (Skia's triangle wave) follow upstream. A transparency layer with a composite mode composites with its blend mode and draws inside it in source-over, as Skia's does. Clip bounds are tracked in target pixels for `ClipBounds`.
- **Shadows:** the shadow's blur radius is Render2D's (twice the standard deviation, as Skia's `radius / 2`); its opacity is ignored, as Skia ignores it. A rect with a rounded hole and a drop shadow draws only the shadow (Skia's inset shadow): the shape is drawn 100,000 target pixels away and its shadow offset back.
- **Rounded rects:** circular corners use Render2D's analytic shapes; elliptical ones are filled, clipped and clipped out as paths.
- **Paths:** `ToPath2D` follows Skia's path: arcs and ellipses (rotated ones made about the origin and moved into place) as Render2D's arcs, `arcTo` as the canvas draws it, continuous rounded rects as plain ones.
- **Glyphs:** `FontFaceCache` keeps one `FontFace2D` per FreeType face and synthetic bold and oblique; a glyph's pen is the sum of the advances before it; vertical text is rotated a quarter turn about its anchor. Stroked text is ignored.
- **Not yet (FIXMEs):** compositing operators other than source-over (including `clearRect`), resetting the clip, focus rings as the theme draws them (a plain outline for now; the theme's come with outlines, G4c2).
- **Render2D fix:** a closed contour that starts in a dash holds its first dash back and joins it to the last when that runs round to the start, as Skia's dasher does; before, both ended in caps there, so the start lost its join (a sharp corner's tip blinked out as a dash pattern moved over it).
- **`Page`** initialises the value pool once, parses with Booey's HTML parser (strings and files), shows the document in a `LocalFrame` with a view of the viewport's size, resizes it and updates layout. `Record` returns an empty display list until G4c1 paints the frame view (FIXME); `Replay` draws any display list with the page's fonts.
- **Editor:** Tab switches the window between the Render2D dashboard and a Booey view, a scene drawn through Booey's `GraphicsContext` (`Krystal.Editor/BooeyScene.hpp`), recorded every frame and replayed through the page: boxes with circular and elliptical corners, borders as the border painter fills them (clipped to the border box's rounded rect, a rect with a rounded hole), solid, dashed and dotted border lines (between the side rect's corners, as the border painter passes them), `drawRect`, list bullets, an elliptical clip; outset and inset shadows, underlines in each style, a line-through, sharp and blurred text shadows; linear, radial and conic gradients, a turning star clipping a gradient, an even-odd star with a moving dash, opaque, layered and multiplied circles, a clip-out and a focus ring.
- **Tests:** `Krystal.UI.Tests` (13 cases): the conversions without a GPU (colours, rounded rects, paths with `arcTo`, arcs both ways and a rotated ellipse, gradients); replayed items against the same drawing made directly with Render2D, pixel for pixel (fills, the brush and alpha, circular and elliptical corners, paths with gradients, rounded holes, dashed strokes, transforms and clips, layers with opacity and blend modes, drop shadows, text lines, glyphs), the inset shadow, a dotted border line and the clip bounds; a page's layout at the viewport's size, relayout on resize, replacing and loading documents.

### G4c1 as built (2026-10-19)

- **Booey paints** (the ledger entry is *Paint plumbing, layers and backgrounds* in `docs/booey/PORTING.md`): `LocalFrameView::PaintContents` paints the render view's layer over the dirty rect; layers paint in stacking order (background, negative `z-index`, the foreground phases, the outline, normal flow layers, positive `z-index`) with opacity and blend modes as transparency layers, transforms, overflow clips rounded by the ancestors' radii, and `clip-path` basic shapes and `path()` as clip paths; blocks paint phase by phase (`paintObject`), floats in their own phases, and flex and grid items in `order`.
- **Backgrounds:** `BackgroundPainter` paints colours and gradient images in every layer, with `background-clip`, `-origin`, `-size`, `-position` and `-repeat`, the root's background over the canvas (the body's when the root has none) with the view's base colour under it, and bleed avoidance under rounded borders (shrinking the background, painting it over the border, or a transparency layer). Gradients go `StyleGradientImage` → `GradientImage` → `drawTiledImage` as upstream's do; repeated tiles are filled one by one, clipped, up to 10,000.
- **Not yet:** borders, box shadows, outlines and focus rings (G4c2), text and inline boxes (G4d), tables (G4e); filters, masks, scrollbars, selection and bitmap images are FIXMEs.
- **Root bounds fix:** a root layer painting with transparency now covers the whole canvas (`IncludeRootBackgroundPaintingArea`, from the view's document rect); it was clipped to the root's box, so the canvas below a short document went unpainted.
- **`Page`:** `Record` paints the frame view into the recorder. `LoadFile` drops a UTF-8 byte order mark, which was becoming text at the top of the body.
- **Editor:** Tab cycles three views: the page view, first, shows `Krystal.Editor/pages/index.html` (loaded from the source tree; F5 reloads it) laid out and painted by Booey under the header (colours, gradients, tiles, opacity, transforms, stacking, clips and blend modes; no text yet), then the Render2D dashboard and the Booey scene.
- **Tests:** Booey compares display list dumps of small documents (12 cases). `Krystal.UI.Tests` adds page goldens (`tests/Golden.cpp`: `golden/scenes/backgrounds.html` and `layers.html`, painted through `Page` against Render2D's own golden and Chrome's, both within a mean channel difference of 0.21; they reuse Krystal.Gfx.Tests' golden helpers, and the test device takes a transient memory size since pages draw in many small batches), and checks that a page records its painting and that `LoadFile` drops the byte order mark (15 cases). `capture_chrome.py` takes `--golden DIR` for the page goldens.
- **Known difference:** where two layers of hard-stopped tiles meet along a diagonal, pixel centres exactly on the stop can fall outside both, leaving dotted gaps that Chrome's repeated tile doesn't (the editor page's checkerboard shows them); it goes when tiles become patterns.

### G4c2 as built (2026-10-19)

- **Borders** (the ledger entry is *Borders, shadows and outlines* in `docs/booey/PORTING.md`): `BorderPainter` paints every style, with upstream's fast paths (uniform solid or double borders as one even-odd path when rounded or translucent), per-side painting with mitred corners and side polygons, translucent sides grouped by colour into transparency layers, dotted and dashed sides as lines or, rounded, as clipped strokes of the outer path, and the inset, outset, groove and ridge shades; under the background with `BackgroundOverBorder` bleed avoidance, over it otherwise.
- **Border images:** `NinePieceImagePainter` slices, scales and tiles gradient sources with each `border-image-repeat` rule (`Image::DrawTiled` with tile rules); bitmap sources wait for images.
- **Box shadows:** outer shadows are cast by the box's shape moved out of the clip (spread grows or shrinks it, radii included) and clipped out of the box; inset shadows by a rect with the box's inner shape as its hole; a single plain shadow on an opaque background goes on the background fill itself.
- **Outlines:** every style around the box at its offset, through the border painter; `outline-style: auto` as a focus ring around the rects in the outline colour. Krystal.UI draws focus rings as the Adwaita theme does (Skia's backend draws none without it): a 2-unit ring outside the path with round joins at the colour's alpha, the inside clipped out where Adwaita clears it.
- **Fixes:** gradient pattern tiles start at their source rect from the phase (they were offset for border image pieces); `border-image-repeat: round` rounds the tile count, as the spec and Chrome; rounded rects and ellipses apply their elements as beziers without a platform path, so their length is known and rounded dotted and dashed borders dash; tiles are clipped to whole-unit edges so neighbours don't leave seams.
- **Editor:** the page gains a row of border styles, rounded and multi-coloured borders, a border image, shadows and outlines (including a focus ring), and the cards cast shadows.
- **Tests:** Booey compares dumps of each border style and corner join, translucent, rounded, dotted and dashed borders, border images, outer and inset shadows, outlines and focus rings (9 more cases). `Krystal.UI.Tests` adds the `borders` page golden (styles, multi-coloured and rounded borders, a border image, spread and blurred outer shadows, an inset shadow and outlines; a mean channel difference of 0.20 from Chrome), the `border-images` page golden (each repeat rule, `fill`, widths and outsets; its Chrome comparison is only reported, as Chrome tiles `repeat` and `space` its own way) and a focus ring replay case (18 cases).
- **Later:** inline and continuation outlines (G4d), collapsed table borders (G4e); column rules, fieldset legends' border gaps and `mask-border` are FIXMEs.

### G4c3 as built (2026-10-19)

- **Booey** (the ledger entry is *Image buffers and patterns* in `docs/booey/PORTING.md`): `ImageBuffer` with a display list backend (the only backend), `GraphicsContext::createImageBuffer`, `createAlignedImageBuffer`, `drawImageBuffer` and `drawPattern(ImageBuffer&)`, the `DrawImageBuffer` and `DrawPatternImageBuffer` items, and `GradientImage::drawPattern` as upstream: the gradient filled once into a buffer cached at the context's scale (upstream's `adjustParametersForTiledDrawing` shrinks a gradient that's constant along one axis to a one-unit strip), then drawn as a pattern. The tile-by-tile fills, the tile cap and the whole-unit tile clips are gone.
- **Render2D:** a third paint, `ImagePattern2D`: a texture repeated in pattern space (its transform into the draw's coordinates, the tile's size, the spacing after each tile), sampled linearly or nearest, faded by an opacity, repeating or clamped along each axis. Patterns have a uniform block and texture unit of their own (like gradients, per batch), so they paint rects, rounded rects, paths, strokes and glyphs alike. Without spacing, the texture is sampled continuously with a repeating sampler, so filtering wraps across tile edges and tiles join without seams. `GetCommandBuffer` and `GetDevice` let another renderer draw a frame into a texture while a frame is in progress; it's recorded at its own `End`, before the passes that draw the texture (rather than a pass inside the renderer, as planned: nothing else needed changing).
- **Krystal.UI:** `ImageBufferTextures` renders each image buffer's display list into a texture of its backend size once per replay (nested buffers through renderers of their own), with textures pooled by size and reused by later replays (the device runs its commands in order). `GraphicsContextRender2D` draws image buffers with `DrawImage` and patterns with the pattern paint, as Skia's backend does: the phase moved by the tile's place in the image, a tile that's part of the image (or has spacing) copied into a texture of its own, spacing in the destination's units, and repeating only along axes the destination reaches past the tile on. `Page` owns the textures.
- **Fixed:** hard colour stops in repeated gradients are soft where Chrome's are (the tile is sampled), and the dotted seams where layers of hard-stopped tiles met are gone (every tile is the same pixels).
- **Editor:** the border image comes off the page: its stripes, sliced at a third of a box that isn't square, skew and break at every tile join, in Chrome too.
- **Tests:** Render2D: patterns repeat as the image drawn tile by tile, move and scale with pattern space, leave spacing transparent, join without seams under fractional scales, clamp when not repeated, paint paths and fade; another renderer's frame drawn into a texture is drawn in the same frame (124 cases). Krystal.UI: image buffers and their patterns replay (19 cases); the `border-images` golden is updated (its Chrome comparison stays reported only: Chrome spaces `space` tiles its own way). Booey: the tiled gradient and border image dumps show the pattern items.

### G4d1 as built (2026-10-19)

- **Booey** (the ledger entry is *Text and inline boxes* in `docs/booey/PORTING.md`): `LineLayout::Paint` and `InlineContentPainter` paint a block's inline content from `RenderBlockFlow::PaintInlineChildren` (and an inline's layer from `RenderInline::Paint`): text boxes through `TextBoxPainter` and `TextPainter`, inline boxes through `InlineBoxPainter`, atomic inlines with `PaintAsInlineBlock` and blocks in inline as blocks. Text is unmarked text (`MarkedText`, `StyledMarkedText`, `TextPaintStyle`), drawn with `FontCascade::DrawText` into the glyph items; each `text-shadow` is drawn as upstream (`ShadowApplier`: every shadow but the last alone, clipped to where it falls with the text moved off it, the last with the text); emphasis marks are drawn with the text (moved here from G4d2). Inline boxes paint their box shadows, backgrounds and borders per slice, open where they break across lines, and as one strip for border images and backgrounds with images or radii.
- **Krystal.UI:** `Page::AddFontDirectory` registers a directory's fonts for every page; the editor and the goldens register `third-party/liberation-fonts`, which gains Liberation Sans Bold, Italic and Bold Italic (2.1.5, as the regular face), so bold and italic text have faces of their own.
- **Editor:** the page's first row is text: a heading, a paragraph with a wrapping inline box, coloured, bold, italic and larger words, and a card of soft, layered and glowing text shadows.
- **Tests:** Booey compares dumps of text runs, shadows, right-to-left and hidden text, emphasis marks and inline boxes (11 cases with the marked text tests); Krystal.UI adds the `text` page golden (a heading, a paragraph with a wrapping inline box, bold and italic words, layered shadows, an inline block and right-to-left text; the scene gives Chrome all four faces with `@font-face`), its Chrome comparison reported only: the glyphs and boxes land in the same places, but Chrome on Windows rasterises with DirectWrite (mean 8.2, as the G3b scene's 6.9).
- **Not yet:** decorations, skip-ink, list markers, ellipses and inline outlines (G4d2); selection and markers stay out of G4.

### G4d2 as built (2026-10-19)

- **Booey** (the ledger entry is *Decorations, markers and ellipses* in `docs/booey/PORTING.md`): `TextDecorationPainter` paints underlines and overlines under the text and line-throughs over it, in every style (double, dotted, dashed and wavy, the wave a stroked bezier path clipped to the line's bounds), at the decorating boxes' offsets and thicknesses, with the text's shadows. **Skip-ink** (decided: now): underlines and overlines break where glyphs cross them, `FontCascade::LineSegmentsForIntersectionsWithRect` walking each glyph's outline (`Font::PathForGlyph`, the unhinted FreeType outline, cached) for where its contours cross the line's top and bottom, CJK glyphs drawn over. List markers paint as filled or stroked circles, squares or their counter text; `text-overflow` ellipses paint after their line; inlines' outlines go round their line boxes, shrink-wrapped into one path where the lines touch (`FloatPointGraph`, the corner radii on the first and last lines), and their focus rings round the line boxes and children.
- **Krystal.UI:** nothing new: text lines, paths, ellipses and focus rings already replayed.
- **Editor:** the text row gains a card of decorations (skip-ink, wavy, struck and double overlined text), a list, an ellipsis and an outlined span.
- **Tests:** Booey compares dumps of each decoration, skip-ink, shadows, markers, the ellipsis and inline outlines, and unit tests glyph paths, line intersections and the point graph; Krystal.UI adds the `decorations` page golden (decorations, skip-ink on and off, bullets and counters, an ellipsis, an outline wrapping across lines), reported against Chrome only (text; mean 6.6): Chrome's wave is taller and its skip-ink gaps a little wider, and its ordered markers sit closer to their text.
- **Not yet:** the selection's part in all of these (G4 leaves selection out).

### G4e as built (2026-10-19)

- **Booey** (the ledger entry is *Table painting* in `docs/booey/PORTING.md`): tables paint as upstream. The table paints its background and border (beside its captions), then its captions and sections; sections paint the cells in the rows and columns the paint rect reaches (and cells overflowing their rows), each cell over the stack of backgrounds behind it (column group, column, row group, row, then the cell's own); rows with layers paint their own cells. In the collapsed model the table collects the cells' distinct borders, sorts them by precedence, and paints one pass per border, each cell drawing the sides that use it and row groups their own borders where cells are missing, from the bottom right to the top left so precedence by position holds. `empty-cells: hide`, spans, captions and row outlines paint.
- **Out:** `RenderReplaced::paint` waits for images (decided); masks, selection and hit testing stay FIXMEs.
- **Editor:** a table card (collapsed coloured borders, a dark header, striped rows, a double-bordered spanning cell).
- **Tests:** Booey compares dumps of separated and collapsed tables, the background stack, hidden empty cells, spans, captions and row outlines (7 cases); Krystal.UI adds the `tables` page golden (no text), checked strictly against Chrome (mean 0.47; the only differences are the dash phase of collapsed dashed borders and corner pixels).

### G4f1 as built (2026-10-19)

- **Booey** (the ledger entry is *Hit testing, hover and active* in `docs/booey/PORTING.md`): `HitTestLocation`, `HitTestResult` and `HitTestingTransformState`; the layer walk (positive z-order, normal flow, the layer's foreground, negative z-order, then its background, with transforms, 3D depth sorting, clip rects and `clip-path`); `nodeAtPoint` for boxes (visual overflow, `clip-path` and border radii), blocks (children, floats, inline content), inline boxes, flex and grid items in `order`, tables, sections and rows; the view and `Document::hitTest`; `elementFromPoint`, `elementsFromPoint` and `nodeFromPoint` on `DocumentOrShadowRoot`. `pointer-events` and `visibility` are respected. `Path::contains` is a winding number test with curves flattened. Hover and active state now follow renderers (it used render styles, from before the render tree), and a rect path now applies elements without a platform path.
- **Krystal.UI:** `Page::HitTest` gives the element at a point; `PointerMoved`, `PointerPressed`, `PointerReleased` and `PointerLeft` hit test and update the hover and active chains, as the mouse half of `EventHandler` will in G4f2.
- **Out (FIXMEs):** scrollbars and resizers, the frame view's scroll position, reference clip paths, continuations, the top layer, view transitions, SVG and transformed layers in paginated layers.
- **Editor:** the mouse drives the page view's pointer: cards lift on `:hover` and set down on `:active`, grid boxes, swatches and stage boxes react to hover, and table rows underline.
- **Tests:** Booey compares hit test results (12 cases: the topmost box and local point, `z-index`, transforms, rounded corners, `clip-path`, `pointer-events`, `visibility`, overflow clips, text and inline boxes, floats, tables, flex and grid items, `elementsFromPoint`, hover restyles) and `Path::contains`; Krystal.UI tests hit testing, hover and active through `Page`. No golden: hit testing paints nothing new.

### G4f2 as built (2026-10-19)

- **Booey** (the ledger entry is *Mouse events, clicks and cursors* in `docs/booey/PORTING.md`): `PlatformEvent`, `PlatformMouseEvent` and `Cursor`; `MouseEvent` on `MouseRelatedEvent` (screen, client, page, offset and layer coordinates) and `UIEventWithKeyState` (modifiers), with `MouseEventInit` and `EventModifierInit`; `Element::DispatchMouseEvent` (a `click` with a detail of 2 is followed by `dblclick`); `Document::PrepareMouseEvent`; and a trimmed `EventHandler`, owned by the `LocalFrame`, which turns presses, moves and releases into `mousedown`, `mousemove` and `mouseup`, fires `mouseover`, `mouseout`, `mouseenter` and `mouseleave` as the element under the mouse changes, clicks the nearest common ancestor of the pressed and released elements (`auxclick` for other buttons), updates the hover and active state and chooses the cursor from `cursor` (auto: the hand over links, the I-beam over text, the arrow elsewhere).
- **Krystal.PAL:** `CursorShape` and `IWindow::SetCursor`; the Win32 window shows it over its client area, at once if the mouse is there.
- **Krystal.UI:** `Page::MouseMoved`, `MousePressed`, `MouseReleased` and `MouseLeft` (replacing G4f1's pointer calls) fill platform mouse events, count clicks (500 ms and 2 pixels, as a platform does) and track the buttons held; `Page::CurrentCursor` is the cursor the page wants.
- **Out (FIXMEs):** focus and blur on click (next), pointer events (clicks are mouse events), selection, dragging, wheel scrolling, context menus, scrollbars, resizers, custom cursor images, the system's double click settings and the screen position.
- **Editor:** every button reaches the page; the window shows the page's cursor over the page view (crosshair over the grid boxes, the hand over the swatches, move over the stage boxes, the I-beam over text); a native click listener on the document toggles `picked` on the card clicked; the status table shows hover done and clicks now.
- **Tests:** Booey tests `MouseEvent` (2 cases) and the event handler's order and targets of moves, presses, releases and clicks, removal and cursors (4 cases); Krystal.UI tests mouse events, double clicks, `auxclick`, the buttons held and the cursor through `Page`. No golden: nothing new paints.

### G4g as built (2026-10-19)

- **Booey** (the ledger entry is *Focus and blur on click* in `docs/booey/PORTING.md`): a `mousedown` that isn't cancelled focuses the nearest click focusable element under the mouse, or the viewport, through `FocusAlgorithms::FocusForClick`; the old element gets `blur` and `focusout`, the new one `focus` and `focusin`, between `mousedown` and `mouseup`. A click focus isn't focus visible. `FocusAlgorithms::SystemFocusChanged` sets the document's focused and active flag (restyling `:focus`) and fires `blur` or `focus` at the focused element, which stays focused.
- **Krystal.PAL:** `WindowCallbacks::OnFocusChange` (from `WM_SETFOCUS` and `WM_KILLFOCUS`) and a `WindowFocusEvent`.
- **Krystal.UI:** `Page::SetFocused`; a page starts focused, and a page loaded while the window is unfocused starts unfocused.
- **Out (FIXMEs):** keyboard events and sequential navigation (G4h), the focus navigation starting point, focus visible after keyboard use, `autofocus`, events at the `Window`, scrollbars, selection, text fields and form controls.
- **Editor:** the cards take `tabindex=0` and show an outline when focused; the first stage's boxes take `tabindex=-1` and the stage shows `:focus-within`; the window's focus reaches the page; the status table shows clicks done and focus now.
- **Tests:** Booey tests a press focusing (the order and targets of the events, the nearest focusable ancestor, the viewport, a cancelled press, `:focus` restyling, no focus visible) and the window's focus; Krystal.UI tests a click focus, `SetFocused` and loading while unfocused through `Page`. No golden: nothing new paints.

### G4h1 as built (2026-10-19)

- **Krystal.UI:** `Page` is now `View` (`Krystal.UI/View.hpp`), and Krystal.UI depends on Krystal.PAL. `Krystal.UI/Input.hpp` converts the platform layer's input: `input::Describe` gives a `pal::Key`'s DOM `code`, its name if it doesn't type text, its legacy key identifier and its Windows key code (with the side for modifiers); `input::KeyboardEvent` builds Booey's platform key events (a press's `key` is the key's name, else the text it types, else its text with only Shift for shortcuts such as Ctrl+A; a release reports its press's); `ToBooey` converts buttons and modifiers; `ToCursorShape` moved from the editor. `View::KeyPressed` and `KeyReleased` (returning whether the page cancelled the key); the mouse calls take `pal::MouseButton` and `pal::KeyModifiers`, and `CurrentCursor` is a `pal::CursorShape`.
- **Booey** (the ledger entry is *Keyboard events* in `docs/booey/PORTING.md`): `PlatformKeyboardEvent`, the Windows key codes, `KeyboardEvent` and `KeyboardEventInit`, `Element::DispatchKeyEvent` and the keyboard half of `EventHandler`: `keydown`, then `keypress` for keys that type text unless `keydown` is cancelled, then `keyup`, at the focused element or the body; a key press (not a modifier alone) makes the focused element focus visible.
- **Krystal.PAL:** `KeyModifier`, `KeyModifiers` and `KeyText` (`KeyModifiers.hpp`); the key, mouse button and mouse move callbacks and events carry the modifiers held, and key presses the text they type (from `ToUnicodeEx` without changing the dead key state, without control characters except Enter's carriage return) and the text with only Shift and Caps Lock; Alt combinations (`WM_SYSKEYDOWN`, `WM_SYSKEYUP`) are reported and still reach `DefWindowProc`, for Alt+F4; as browsers do, `WM_SYSCHAR` doesn't (it would look for a menu mnemonic and beep), except for Alt+Space (the window menu), and Alt or F10 alone doesn't enter menu mode (`SC_KEYMENU` from the keyboard). The Win32 key mapping was fixed (see the ledger entry).
- **Out (FIXMEs):** the keyboard's default actions (Tab is G4h2), text editing and input events, input methods and composition, dead keys, access keys, the menu and context menu keys (and Insert, Print Screen, Pause, Scroll Lock and Num Lock, which the platform layer has no keys for), activation by Space and Enter.
- **Editor:** forwards key events to the view in the page view (except F2, F5 and Escape, which still quits); views switch with F2 rather than Tab, which G4h2 gives to the page; mouse events carry their modifiers; a native `keydown` listener shows the last key's value in the status table, which shows focus done and Tab next.
- **Tests:** Booey tests `KeyboardEvent` (2 cases) and the event handler's keys (1 case); Krystal.UI tests keys through `View` (values, codes, locations, legacy codes, shortcuts, named keys that type text, cancelling) and the input conversions. No golden: nothing new paints.

## Open decisions

Decided (2026-10-17): a virtual `Device` chosen at run time; the Vulkan and D3D11 stubs, the LearnOpenGL scenes and the editor's assets deleted; backends as subdirectories of Krystal.Gfx.

Decided (2026-10-18): the 2D renderer lives in Krystal.Gfx, as `Render2D/` (namespace `krys::gfx`), not in its own module.

1. **Module names:** Krystal.UI, Krystal.Assets.
2. **Shaders:** GLSL with explicit binding slots now. Later, possibly SPIR-V compiled offline, which a Vulkan backend would need.
3. **Text:** decided (2026-10-18): FreeType's 8-bit greyscale bitmaps, with hinting; MSDF is dropped (see *G3 plan*).
4. **Blending space:** decided (2026-10-18): encoded values everywhere, as browsers blend, with unorm targets and an `RGBA8Unorm` window (see *G2h2 plan*). Before, Render2D blended in linear light on sRGB targets and as encoded on other targets.
