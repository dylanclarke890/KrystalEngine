# Graphics redesign

Status (2026-10-18): C0 (the colour types) and G1 (the RHI and its OpenGL backend) are done; Render2D (G2) is in progress (G2a, solid rects, is done); text (G3) and the Booey integration (G4) are drafts.

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
      │                     Backends are subdirectories (`OpenGL/`, namespace `krys::gfx::opengl`).
      └─ Krystal.Render2D   2D drawing on the RHI: rects, rounded rects, borders, images, glyphs, clips, layers
Krystal.Booey               paints into a display list; no dependency on Gfx
Krystal.UI                  the Booey host: pages, display list replay into Render2D, glyph atlas, input routing
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

## Layer 2: the 2D renderer (Krystal.Render2D)

A generic, immediate-mode 2D API on the RHI. It knows nothing about Booey; game HUD code and debug overlays can use it directly.

```cpp
namespace krys::render2d
{
  class Renderer2D
  {
  public:
    explicit Renderer2D(gfx::Device &device) noexcept;

    void Begin(gfx::CommandBuffer &commands, const Target &target, float deviceScaleFactor) noexcept;
    void End() noexcept; // flushes the last batch

    // State, as stacks.
    void Save() noexcept;
    void Restore() noexcept;
    void Transform(const AffineTransform &transform) noexcept;
    void ClipRect(const Rect &rect) noexcept;
    void ClipRoundedRect(const RoundedRect &rect) noexcept;
    void ClipOutRoundedRect(const RoundedRect &rect) noexcept;
    void BeginLayer(float opacity, BlendMode blend = BlendMode::Normal) noexcept;
    void EndLayer() noexcept;

    // Drawing.
    void FillRect(const Rect &rect, const Paint &paint) noexcept;
    void FillRoundedRect(const RoundedRect &rect, const Paint &paint) noexcept;
    void FillRoundedRectWithHole(const RoundedRect &outer, const RoundedRect &inner, const Paint &paint) noexcept;
    void DrawBorder(const RoundedRect &outer, const RoundedRect &inner, const BorderSides &sides) noexcept;
    void DrawBoxShadow(const RoundedRect &rect, const BoxShadow &shadow) noexcept;
    void DrawImage(gfx::TextureHandle texture, const Rect &source, const Rect &destination, const ImageOptions &options) noexcept;
    void DrawGlyphs(const GlyphAtlasPage &page, Span<const GlyphQuad> glyphs, const SRGBA<uint8> &colour) noexcept;
    void FillPath(const Path &path, const Paint &paint) noexcept; // tessellated on the CPU
    void StrokeLine(const Point &from, const Point &to, const StrokeStyle &stroke) noexcept;
  };
}
```

`Paint` is a solid colour, a linear, radial or conic gradient (a colour ramp in a texture), or an image pattern.

Implementation notes:
- One shader handles most primitives. Each vertex carries position, UV, colour (`PackedColour::RGBA`) and per-primitive parameters (the rect, corner radii, primitive kind). The fragment shader computes a signed distance to the rounded rect: antialiased edges and rounded corners without tessellation, and box shadows from an analytic blur.
- Draws are batched in painter's order; a batch breaks only on a change of texture, pipeline or clip kind.
- Pipelines (shapes, greyscale glyphs, colour glyphs, images, layer composite, stencil clip) are created at startup and cached per target format.
- Axis-aligned clips use the scissor. Rounded clips use a stencil mask, or a mask layer when transformed.
- Layers render to pooled offscreen textures and composite with their opacity and blend mode.
- Coordinates have a top-left origin, y pointing down, in CSS pixels times the device scale factor. Colours are premultiplied.
- `GlyphAtlas` is a separate helper: shelf packing into R8 pages for greyscale glyphs and RGBA pages for colour glyphs. The caller rasterises glyphs and adds them; Render2D only draws quads from the pages.

## Layer 3: the Booey integration (Krystal.UI)

- Booey ports upstream's `GraphicsContext` and `DisplayList::Recorder` (`platform/graphics/GraphicsContext.h`, `displaylists/`: 63 item types). Painting records into a display list, so Booey stays testable without a GPU, and tests can compare display lists.
- The first items to port are the ones boxes and text need:
  - state: `Save`, `Restore`, the transforms;
  - clips: `Clip`, `ClipRoundedRect`, `ClipOut`, `ClipOutRoundedRect`;
  - fills: `FillRect` (colour, gradient), `FillRoundedRect`, `FillRectWithRoundedHole` (borders, inset shadows);
  - strokes: `StrokeRect`, `DrawLine`, `DrawLinesForText` (underlines);
  - content: `DrawGlyphs`, `DrawNativeImage`;
  - layers: `BeginTransparencyLayer`, `EndTransparencyLayer`.
- Krystal.UI replays a display list into Render2D, mostly one call per item.
- Glyphs are rasterised with FreeType through Booey's `FreeTypeFace`, keyed by (face, glyph, size, subpixel offset), and cached in a `GlyphAtlas`. The MSDF atlas loader can come back later for text in 3D, where scaling matters.
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
| G2 | Render2D: rects, rounded rects, borders, box shadows, images, gradients, clips, transforms, layers. Reviewed one part at a time (see *G2 parts*). | A test app draws a UI-like scene; golden image tests pass. |
| G3 | Text: `GlyphAtlas`, `DrawGlyphs`, FreeType rasterisation. | Text renders crisply at several sizes and scales. |
| G4 | Booey: `GraphicsContext`, the display list recorder, painting (paint order, backgrounds, borders, text); Krystal.UI replays it. | The vertical slice: a page loads, renders, resizes and responds to a click. |

### G2 parts

Decided (2026-10-18): one part at a time, each reviewed before the next.

| Part | Content |
|---|---|
| G2a | Done (2026-10-18). The `Render2D/` skeleton, `Begin`/`End`, solid `FillRect`, batching; an offscreen readback test; the editor draws rects. |
| G2b | Rounded rects and antialiasing (the signed distance shader). |
| G2c | Transforms, the `Save`/`Restore` stack, scissor clips. |
| G2d | Images. |
| G2e | Borders and box shadows. |
| G2f | Gradients. |
| G2g | Rounded clips (stencil) and layers (offscreen, opacity, blend modes). |

### G2a as built (2026-10-18)

- `Render2D/Rect2D.hpp`: `Rect2D`, a plain `{X, Y, Width, Height}` in pixels.
- `Render2D/Renderer2D.hpp`: `Renderer2D::Create(device)`, then per frame `Begin(commands, Render2DTarget)`, `FillRect(rect, SRGBA<uint8>)`, `End()`. The target is a swapchain or a texture with its size, format and an optional clear colour; `End` records one render pass with every draw in it.
- Rects are instanced: one 20-byte record each (rect, colour) in transient memory, drawn as a 4-vertex strip whose corners come from `gl_VertexID`, so there's no quad vertex buffer. A frame's rects are one draw, in painter's order. Empty and fully transparent rects are skipped.
- One pipeline per target format, created on first use. The shaders are embedded in `Renderer2D.cpp` until the shader decision (open decision 2).
- Colours are sRGB with straight alpha, premultiplied in the shader. On sRGB targets they're decoded and blend in linear light; on other targets they blend as encoded (open decision 4).
- If the frame's transient memory runs out, the frame's rects are dropped (with an assert in debug builds); growing the ring is left for later.
- Tests: `Krystal.Gfx.Tests/tests/Render2D/Renderer2D.cpp` (5 cases). The GPU tests share `CreateTestDevice` (`include/Krystal.Gfx.Tests/TestDevice.hpp`). The editor's triangle is replaced by rects drawn with Render2D.

Testing: the device's hidden window gives tests a real OpenGL context (Krystal.Gfx.Tests). Render2D gets golden image tests: render offscreen, read back with `ReadTexture`, compare against a PNG with a tolerance.

## Open decisions

Decided (2026-10-17): a virtual `Device` chosen at run time; the Vulkan and D3D11 stubs, the LearnOpenGL scenes and the editor's assets deleted; backends as subdirectories of Krystal.Gfx.

Decided (2026-10-18): the 2D renderer lives in Krystal.Gfx, as `Render2D/` (namespace `krys::gfx`), not in its own module.

1. **Module names:** Krystal.UI, Krystal.Assets.
2. **Shaders:** GLSL with explicit binding slots now. Later, possibly SPIR-V compiled offline, which a Vulkan backend would need.
3. **Text:** FreeType greyscale bitmaps first (recommended), or MSDF.
4. **Blending space:** Render2D blends in linear light on sRGB targets and in sRGB-encoded values on other targets, so translucent colours look different by target format. Browsers blend encoded values everywhere. To be checked (2026-10-18).
