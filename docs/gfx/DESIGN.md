# Graphics redesign

Status (2026-10-18): C0 (the colour types) and G1 (the RHI and its OpenGL backend) are done; Render2D (G2) is in progress (G2a, solid rects, G2b, rounded rects and antialiasing, G2c, transforms and rect clips, and G2d, images, are done); text (G3) and the Booey integration (G4) are drafts.

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
- **Render2D and Krystal.UI** use the typed colours: vertex colours are `PackedColour::RGBA` (premultiplied `SRGBA<uint8>`), and blending happens in a colour space chosen by type.

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
- Beyond the draft: `ReadTexture` (synchronous readback, for tests and screenshots), `GetTextureDesc`, `SetVSync`, `WaitIdle`, `Limits`.
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
    void FillRect(const Rect2D &rect, const Paint &paint) noexcept;
    void FillRoundedRect(const RoundedRect2D &rect, const Paint &paint) noexcept;
    void FillRectWithRoundedHole(const Rect2D &rect, const RoundedRect2D &hole, const Paint &paint) noexcept;
    void FillPath(const Path2D &path, FillRule rule, const Paint &paint) noexcept;
    void StrokePath(const Path2D &path, const StrokeStyle &stroke, const Paint &paint) noexcept; // width, caps, joins, dashes

    // Content.
    void DrawImage(TextureHandle texture, const Rect2D &source, const Rect2D &destination, const ImageOptions &options) noexcept;
    void DrawGlyphs(Span<const GlyphQuad> glyphs, const Paint &paint) noexcept;
  };
}
```

`Paint` is a solid colour, a linear, radial or conic gradient (a colour ramp in a texture), or an image pattern. Shadows follow upstream's model (a shadow is graphics state applied to the next fill: a blurred copy of its coverage, offset and coloured), and are designed with that part.

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
| G2e | Paths and coverage masks: `Path2D` (lines, arcs, quadratic and cubic Béziers), curve flattening, a CPU coverage rasteriser (non-zero and even-odd), the mask atlas and its cache; `FillPath`, `FillRectWithRoundedHole`. Elliptical corner radii are decided here (analytic or through masks). |
| G2f | Strokes: a stroker that turns a stroke (width, caps, joins, dashes) into an outline for G2e; `StrokePath`. |
| G2g | Clips: rounded-rect and path clips as antialiased clip masks; clip-out. |
| G2h | Layers: offscreen layers with opacity and blend modes. |
| G2i | Gradients (linear, radial, conic) as `Paint`. |
| G2j | Shadows: blurred coverage, following upstream's shadow state. |

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
- The fragment shader computes the signed distance to the rounded box and covers `clamp(0.5 - distance, 0, 1)` of the pixel: a one-pixel band centred on the edge. Edges on pixel boundaries stay sharp; edges between pixels are blended. The quad is a pixel larger than the rect on every side so the band outside the edge is drawn.
- The instance is 36 bytes (rect, radii, colour).
- Limits: corners are circular (CSS's elliptical radii, `10px / 20px`, are decided in G2e); rects thinner than a pixel come out somewhat heavier than their true coverage; the distance is in target pixels, so transforms (G2c) will need the antialiasing width scaled.
- Tests: two more cases in `tests/Render2D/Renderer2D.cpp` (edges between pixels; rounded corners, constrained radii, per-corner and negative radii). The editor's panels are rounded.

### G2c as built (2026-10-18)

- `Render2D/Transform2D.hpp`: `Transform2D`, a 2D affine transform laid out as CSS's `matrix(a, b, c, d, e, f)`, with `Identity`, `Translation`, `Scale`, `Rotation` (clockwise on screen, since y points down), composition (`T * U` applies `U` first), `Apply`, `Determinant` and `PreservesAxisAlignment`. Points are Core's `Vec2f`.
- Drawing state is a stack of (transform, clip): `Save`, `Restore`, `ConcatTransform`, `Translate`, `Scale`, `Rotate`, `SetTransform`, `GetTransform`. Each frame starts with the identity transform and the whole target as the clip; unbalanced `Save`/`Restore` asserts.
- Every instance carries its own transform (the instance grows to 60 bytes), so draws under different transforms (every scrolled element, for one) still go out in one draw; a batch only breaks when the clip changes.
- Antialiasing stays one target pixel wide under any transform: the shader measures the distance in local units and divides by how far it grows per target pixel, computed exactly from the inverse transform and the edge's normal. That factor is exactly 1 without a transform, so pixel-aligned edges stay sharp. (Screen-space derivatives were tried on paper and rejected: they smear by 1 to 2/255 at square corners.) The quad is grown by one target pixel in local units along each axis.
- `ClipRect` intersects the clip with a rect in the current coordinates and becomes the scissor, in whole target pixels: a pixel is inside when its centre is. Clip edges are hard until G2g. Under rotations and skews the clip is the rect's bounding box (a debug assert and a FIXME until G2g's clip masks).
- Draws under an empty clip or a singular transform are skipped.
- Tests: `tests/Render2D/Transform2D.cpp` (3 cases); 3 more cases in `tests/Render2D/Renderer2D.cpp` (translations, scales, quarter turns; antialiasing under scale and a 45-degree turn; clip rounding, nesting, transforms, `Restore`, empty clips). The editor adds a clipped, scrolling list in the sidebar and a spinning square.

### G2d as built (2026-10-18)

- `DrawImage(texture, source, destination, ImageOptions)` and `DrawImage(texture, destination)` for the whole texture. The source is in texels with a top-left origin. `ImageOptions`: `Filter` (linear, with linear mipmaps when the texture has them, or nearest), `Alpha` (`AlphaPremultiplication`, premultiplied by default) and `Opacity`.
- One pipeline and one shader still draw everything: an image is a rounded-rect instance (with zero radii for now) that multiplies its colour by a texel. The instance gains texture coordinates and flags (textured, straight alpha, linear texels) and is 80 bytes. An image's colour is white with the opacity as alpha, so tints come for free later (masks and glyphs need them).
- Batches break on a change of clip or texture (and sampler). Untextured instances join any batch with the same clip, and a textured one adopts a batch that has no texture yet, so rects drawn between images of the same texture don't break the batch. Batches without a texture bind a 1-by-1 white texture when nothing else is bound: they never sample it, but drivers check every declared sampler's texture at each draw (NVIDIA warns, 131204).
- Texels are clamped half a texel inside the source rect, so linear filtering never reads the texels around a sub-image (atlases). Texture derivatives are taken outside the textured branch, where every fragment of a quad runs them, and sampled with `textureGrad`.
- Colours follow the colour path (open decision 4): textures in sRGB formats are decoded by the GPU; others hold sRGB-encoded values; the shader converts texels to the target's blending space (unpremultiplying around the conversion). Premultiplied data in an sRGB-format texture isn't exact, since the GPU decodes each channel before the alpha division; premultiplied images belong in unorm formats.
- Tests: five more cases in `tests/Render2D/Renderer2D.cpp` (nearest and linear scaling; clamping to the source rect; premultiplied, straight and opacity; sRGB texture into a unorm target and unorm texture into an sRGB target; images and rects interleaved across textures, in painter's order, under a transform). The editor generates a 16-by-16 checkered gradient (there are no image assets) and draws it scaled with nearest filtering and turning with linear filtering.

Testing: the device's hidden window gives tests a real OpenGL context (Krystal.Gfx.Tests). Render2D gets golden image tests: render offscreen, read back with `ReadTexture`, compare against a PNG with a tolerance.

## Open decisions

Decided (2026-10-17): a virtual `Device` chosen at run time; the Vulkan and D3D11 stubs, the LearnOpenGL scenes and the editor's assets deleted; backends as subdirectories of Krystal.Gfx.

Decided (2026-10-18): the 2D renderer lives in Krystal.Gfx, as `Render2D/` (namespace `krys::gfx`), not in its own module.

1. **Module names:** Krystal.UI, Krystal.Assets.
2. **Shaders:** GLSL with explicit binding slots now. Later, possibly SPIR-V compiled offline, which a Vulkan backend would need.
3. **Text:** FreeType greyscale bitmaps into the glyph atlas first (recommended), or MSDF.
4. **Blending space:** Render2D blends in linear light on sRGB targets and in sRGB-encoded values on other targets, so translucent colours look different by target format. Browsers blend encoded values everywhere; emulating them means blending encoded values on every target and an RGBA8Unorm window. Leaning that way, to be checked (2026-10-18).
