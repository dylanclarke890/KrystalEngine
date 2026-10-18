# Graphics redesign

Status (2026-10-18): C0 (the colour types) and G1 (the RHI and its OpenGL backend) are done; Render2D (G2) is in progress (G2a, solid rects, G2b, rounded rects and antialiasing, G2c, transforms and rect clips, G2d, images, G2e, paths, masks and rounded holes, G2f, strokes, G2g, clips, G2h, layers and blend modes, G2i, gradients, and G2j, shadows, are done; G2k, golden image tests, is next); text (G3) and the Booey integration (G4) are drafts.

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
  └─ Krystal.Gfx            the RHI: device, handles, descriptions, command buffers. No files, no caches.
                            Backends are subdirectories (`OpenGL/`, namespace `krys::gfx::opengl`).
                            Render2D/: 2D drawing on the RHI (fills, strokes, paths, images, glyphs, clips, layers)
Krystal.Booey               paints into a display list; no dependency on Gfx
Krystal.UI                  the Booey host: pages, display list replay into Render2D, glyph rasterisation, input routing
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
  Text/                FontAtlas, FontAtlasLoader (kept from the old module, unused until text in 3D)
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
- Glyphs are rasterised with FreeType through Booey's `FreeTypeFace`, keyed by (face, glyph, size, subpixel offset), and cached in Render2D's glyph atlas. The MSDF atlas loader can come back later for text in 3D, where scaling matters.
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
| `FontAtlasLoader`, msdf-atlas-gen | Kept, moved to `Text/`, unused until text in 3D. |
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
| G3 | Text: the glyph atlas (R8 pages, RGBA pages for colour glyphs), `DrawGlyphs`, FreeType rasterisation. It doesn't depend on G2e's mask atlas. | Text renders crisply at several sizes and scales. |
| G4 | Booey: `GraphicsContext`, the display list recorder, painting (paint order, backgrounds, borders, text); Krystal.UI replays it. | The vertical slice: a page loads, renders, resizes and responds to a click. |

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
| G2k | Golden image tests: UI-like scenes against Render2D's own reviewed images and against Chrome's (see *G2k plan*). |

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
- **Swapchains:** when a batch drawn straight into a swapchain target copies, the target's pass draws into a pooled texture the target's size, cleared to the clear colour or transparent, and a last pass (`WindowLayer`) composites it into the window, cleared to transparent or kept.
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

Testing: the device's hidden window gives tests a real OpenGL context (Krystal.Gfx.Tests). Render2D gets golden image tests: render offscreen, read back with `ReadTexture`, compare against a PNG with a tolerance.

## Open decisions

Decided (2026-10-17): a virtual `Device` chosen at run time; the Vulkan and D3D11 stubs, the LearnOpenGL scenes and the editor's assets deleted; backends as subdirectories of Krystal.Gfx.

Decided (2026-10-18): the 2D renderer lives in Krystal.Gfx, as `Render2D/` (namespace `krys::gfx`), not in its own module.

1. **Module names:** Krystal.UI, Krystal.Assets.
2. **Shaders:** GLSL with explicit binding slots now. Later, possibly SPIR-V compiled offline, which a Vulkan backend would need.
3. **Text:** FreeType greyscale bitmaps into the glyph atlas first (recommended), or MSDF.
4. **Blending space:** decided (2026-10-18): encoded values everywhere, as browsers blend, with unorm targets and an `RGBA8Unorm` window (see *G2h2 plan*). Before, Render2D blended in linear light on sRGB targets and as encoded on other targets.
