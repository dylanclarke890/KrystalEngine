# Graphics redesign (draft)

Status: draft for review, 2026-10-17. Nothing here is implemented.

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
Krystal.Core                colour types (moved from Booey), CommandList, handles' building blocks
  └─ Krystal.Gfx            the RHI: device, handles, descriptions, command buffers. No files, no caches.
      ├─ Krystal.Gfx.OpenGL the OpenGL 4.5 backend
      └─ Krystal.Render2D   2D drawing on the RHI: rects, rounded rects, borders, images, glyphs, clips, layers
Krystal.Booey               paints into a display list; no dependency on Gfx
Krystal.UI                  the Booey host: pages, display list replay into Render2D, glyph atlas, input routing
Krystal.Assets (later)      VFS loading, decoding, path caching, reference-counted asset handles
Krystal.Samples (optional)  the LearnOpenGL scenes, if kept
```

Booey never depends on Krystal.Gfx. Krystal.UI is the only module that knows both.

## Colour

Decision (2026-10-17): the engine uses Booey's typed colours, moved into Krystal.Core, spelled "Colour", in the `krys` namespace.

- Move Booey's colour maths from `Krystal.Booey/Graphics/Color/` to `Krystal.Core/Colour/`: the components, models, types (`SRGBA<T>`, `LinearSRGBA<T>` and the other colour spaces), transfer functions, matrices, conversion, colour spaces, `PackedColour`, and the utilities (blending, interpolation, luminance, normalisation). These headers already depend only on Krystal.Core and each other, so the move is mechanical.
- Rename "Color" to "Colour" in the moved files and names (`ColourComponents`, `ColourSpace`, `ColourMatrix`, `PackedColour`, `ColourConversion.hpp`, ...), and move them from `krys::boo` to `krys`.
- Booey keeps the CSS-facing `Color` value type and its serialisation, built on the moved types. It keeps the spelling `Color`, matching the CSS specifications (decision, 2026-10-17).
- Delete Gfx's `Colour.hpp`, `ColourFormatter.hpp` and `ColourHasher.hpp`. They're a plain RGBA struct, used only inside the Gfx modules.

Where each layer gets its colours:
- **The RHI has no colour type.** A clear value is four numbers in the target's encoding (`Array<float, 4>`, or integers for integer formats). Whether a value is sRGB or linear depends on the texture's `Format`, not on a colour class.
- **Render2D and Krystal.UI** use the typed colours: vertex colours are `PackedColour::RGBA` (premultiplied `SRGBA<uint8>`), and blending happens in a colour space chosen by type.

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
└─ Resource layout   "uniform block at slot 0, texture at slot 0, 64 bytes of push constants"
```

Binding one is a single call at draw time, and nothing is left for the driver to work out.

**Dynamic state** isn't in the pipeline: the viewport and scissor rectangle, which buffers and textures are bound (the pipeline only fixes the slots and their kinds), the blend constant, the stencil reference, and clear colours (which belong to the render pass).

Pipelines are cheap to bind and expensive to create (that's when shaders compile and link), so they're created up front: one per combination of shaders and state actually used. A 2D UI renderer needs about 5 to 8. Pipelines that differ only in state share shaders: today's `CompositeRenderTarget` and `CompositeRenderTargetWithOpacity` become two pipelines with the same shaders and different blend state.

The output formats are part of the pipeline because blending and the conversion to the target's format are compiled in. The same shaders drawing into the window (BGRA8 sRGB) and into an RGBA16F texture are two pipelines; Render2D hides this with a cache keyed by the description and the formats.

#### Binding slots

Shaders declare numbered slots instead of being set by uniform name, and the program fills buffers and binds them to those numbers:

```glsl
#version 450
layout(binding = 0, std140) uniform Frame      // slot 0: per-frame data, one buffer for the whole frame
{
  mat4 u_Projection;
  vec2 u_ViewportSize;
};
layout(push_constant) uniform Draw            // small per-draw data (emulated with a uniform buffer in OpenGL)
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

### Files

```
Krystal.Gfx/include/Krystal.Gfx/
  Handles.hpp          handle types
  Pool.hpp             generational slot storage (used by backends)
  Formats.hpp          Format, IndexType, VertexFormat
  Buffer.hpp           BufferDesc, BufferUsage, MemoryUsage
  Texture.hpp          TextureDesc, TextureUsage, TextureRegion
  Sampler.hpp          SamplerDesc
  Shader.hpp           ShaderDesc, ShaderStage
  Pipeline.hpp         PipelineDesc and its state blocks
  Swapchain.hpp        SwapchainDesc
  RenderPass.hpp       RenderPassDesc, LoadOp, StoreOp
  CommandBuffer.hpp    the recorder
  Device.hpp           the device interface
Krystal.Gfx.OpenGL/   GLDevice and the per-resource wrappers
```

### Handles and the pool

```cpp
/// @brief A typed reference to a device resource: a 24-bit slot index and an 8-bit generation. 0 is invalid.
template <typename TTag>
struct Handle
{
  uint32 Value {0u};

  KRYS_NODISCARD constexpr bool IsValid() const noexcept { return Value != 0u; }
  KRYS_NODISCARD constexpr uint32 Index() const noexcept { return (Value & 0xFF'FFFFu) - 1u; }
  KRYS_NODISCARD constexpr uint8 Generation() const noexcept { return static_cast<uint8>(Value >> 24); }
  constexpr auto operator<=>(const Handle &) const noexcept = default;
};

using BufferHandle = Handle<struct BufferTag>;
using TextureHandle = Handle<struct TextureTag>;
using SamplerHandle = Handle<struct SamplerTag>;
using ShaderHandle = Handle<struct ShaderTag>;
using PipelineHandle = Handle<struct PipelineTag>;
using SwapchainHandle = Handle<struct SwapchainTag>;
```

The index is stored plus one, so slot 0 with generation 0 is still a valid handle.

```cpp
/// @brief Generational storage for one resource type. Stale handles return null; nothing throws.
template <typename T, typename THandle>
class Pool
{
public:
  KRYS_NODISCARD THandle Add(T &&value) noexcept;
  KRYS_NODISCARD T *TryGet(THandle handle) noexcept;       // null if stale or invalid
  KRYS_NODISCARD T &Get(THandle handle) noexcept;          // asserts
  KRYS_NODISCARD Maybe<T> Remove(THandle handle) noexcept; // hands the object back, for deferred destruction
  template <typename TFunc> void ForEach(TFunc &&func) noexcept; // leak reports at shutdown
};
```

Ownership:
- Whoever creates a resource owns it and destroys it explicitly. The RHI doesn't reference count.
- `Destroy` is deferred until the GPU has finished the frames that use the resource.
- `Owned<THandle>` is an optional move-only wrapper that destroys on drop.
- At shutdown, in debug builds, the device reports leaked resources by debug name.
- Reference counting and caching by path belong to the asset layer.

### Descriptions

Plain structs with defaults, made with designated initialisers. Each carries a `DebugName`, used for the OpenGL object label and in leak reports.

```cpp
enum class BufferUsage : uint8 { Vertex = 1, Index = 2, Uniform = 4, Storage = 8, CopyDestination = 16 }; // flags
enum class MemoryUsage : uint8 { GpuOnly, Dynamic };

struct BufferDesc
{
  uint64 Size {0u};
  BufferUsage Usage {BufferUsage::Vertex};
  MemoryUsage Memory {MemoryUsage::GpuOnly};
  Span<const byte> InitialData {};
  utf8_stringview DebugName {};
};

enum class TextureDimension : uint8 { Texture1D, Texture2D, Texture3D, Cube };
enum class TextureUsage : uint8 { Sampled = 1, RenderTarget = 2, DepthStencil = 4, Storage = 8 }; // flags

struct TextureDesc
{
  TextureDimension Dimension {TextureDimension::Texture2D};
  Format Format {Format::RGBA8Unorm};
  uint32 Width {1u}, Height {1u}, Depth {1u};
  uint32 Layers {1u};
  uint32 MipLevels {1u};
  uint32 Samples {1u};
  TextureUsage Usage {TextureUsage::Sampled};
  utf8_stringview DebugName {};
};

struct ShaderDesc
{
  ShaderStage Stage {ShaderStage::Vertex};
  utf8_stringview Source {}; // GLSL for now; the caller loads it, the RHI never touches files
  utf8_stringview EntryPoint {"main"};
  utf8_stringview DebugName {};
};

struct VertexAttribute
{
  uint32 Location;
  VertexFormat Format;
  uint32 Offset;
};

struct VertexBufferLayout
{
  uint32 Stride;
  VertexInputRate Rate {VertexInputRate::PerVertex};
  FixedVector<VertexAttribute, 16> Attributes;
};

struct BlendState
{
  bool Enabled {false};
  BlendFactor SourceColour {BlendFactor::One}, DestinationColour {BlendFactor::OneMinusSourceAlpha};
  BlendFactor SourceAlpha {BlendFactor::One}, DestinationAlpha {BlendFactor::OneMinusSourceAlpha};
  BlendOp ColourOp {BlendOp::Add}, AlphaOp {BlendOp::Add};
  ColourWriteMask WriteMask {ColourWriteMask::All};

  KRYS_NODISCARD constexpr static BlendState PremultipliedAlpha() noexcept { return {.Enabled = true}; }
};

struct DepthStencilState
{
  bool DepthTest {false};
  bool DepthWrite {false};
  CompareOp DepthCompare {CompareOp::Less};
  StencilState Stencil {};
};

struct RasterState
{
  CullMode Cull {CullMode::None};
  FrontFace Front {FrontFace::CounterClockwise};
  bool Scissor {false};
};

struct PipelineDesc
{
  ShaderHandle VertexShader {};
  ShaderHandle FragmentShader {};
  FixedVector<VertexBufferLayout, 4> VertexBuffers {};
  PrimitiveTopology Topology {PrimitiveTopology::Triangles};
  BlendState Blend {};
  DepthStencilState DepthStencil {};
  RasterState Raster {};
  FixedVector<Format, 8> ColourFormats {};
  Format DepthFormat {Format::Undefined};
  uint32 PushConstantSize {0u}; // at most 128 bytes
  utf8_stringview DebugName {};
};
```

| Resource | Replaces |
|---|---|
| Buffer | `BufferRegistry` |
| Texture (with a default view) | `Image`, `ImageView`, `Texture`, and render target attachments |
| Sampler (deduplicated by the device) | `SamplerRegistry` and its cache |
| Shader | the compile half of `ShaderRegistry` |
| Pipeline | `Shader::SetUniform` by name, `MaterialParameter`, the global state calls in the renderer |
| Swapchain (one per window) | `IContext::Present`, the screen render target |

There are no render target objects: a render pass names its textures directly. Texture views as separate resources are left out until something needs them.

### Command buffers

```cpp
struct ColourAttachment
{
  Variant<TextureHandle, SwapchainHandle> Target;
  LoadOp Load {LoadOp::Clear};
  StoreOp Store {StoreOp::Store};
  Array<float, 4> Clear {0.f, 0.f, 0.f, 1.f}; // in the target's encoding
};

struct RenderPassDesc
{
  FixedVector<ColourAttachment, 8> Colour {};
  Maybe<DepthAttachment> Depth {};
  utf8_stringview DebugName {};
};

class CommandBuffer
{
public:
  void BeginRenderPass(const RenderPassDesc &desc) noexcept;
  void EndRenderPass() noexcept;

  void SetViewport(const Viewport &viewport) noexcept;
  void SetScissor(const ScissorRect &rect) noexcept;

  void BindPipeline(PipelineHandle pipeline) noexcept;
  void BindVertexBuffer(uint32 slot, BufferHandle buffer, uint64 offset = 0u) noexcept;
  void BindIndexBuffer(BufferHandle buffer, IndexType type, uint64 offset = 0u) noexcept;
  void BindUniformBuffer(uint32 slot, BufferHandle buffer, uint64 offset, uint64 size) noexcept;
  void BindTexture(uint32 slot, TextureHandle texture, SamplerHandle sampler) noexcept;
  template <typename T> void PushConstants(const T &data) noexcept; // static_assert(sizeof(T) <= 128)

  void Draw(uint32 vertexCount, uint32 instanceCount = 1u, uint32 firstVertex = 0u, uint32 firstInstance = 0u) noexcept;
  void DrawIndexed(uint32 indexCount, uint32 instanceCount = 1u, uint32 firstIndex = 0u, int32 vertexOffset = 0,
                   uint32 firstInstance = 0u) noexcept;

  void CopyBufferToTexture(BufferHandle source, uint64 offset, TextureHandle destination, const TextureRegion &region) noexcept;
  void BlitTexture(TextureHandle source, const Rect &from, TextureHandle destination, const Rect &to, Filter filter) noexcept;

  void PushDebugGroup(utf8_stringview name) noexcept; // shows up in RenderDoc
  void PopDebugGroup() noexcept;

private:
  CommandList _commands;
};
```

In debug builds the recorder checks usage as it records: a draw outside a render pass, a draw without a pipeline, or nested render passes assert at the mistake rather than at submit.

### The device

```cpp
class Device
{
  KRYS_NON_COPY_MOVABLE(Device);

public:
  KRYS_NODISCARD static Expected<UniquePtr<Device>> Create(const DeviceDesc &desc) noexcept;
  virtual ~Device() noexcept = default;

  KRYS_NODISCARD virtual Expected<BufferHandle> CreateBuffer(const BufferDesc &desc) noexcept = 0;
  KRYS_NODISCARD virtual Expected<TextureHandle> CreateTexture(const TextureDesc &desc) noexcept = 0;
  KRYS_NODISCARD virtual Expected<SamplerHandle> CreateSampler(const SamplerDesc &desc) noexcept = 0;
  KRYS_NODISCARD virtual Expected<ShaderHandle> CreateShader(const ShaderDesc &desc) noexcept = 0;       // error: compile log
  KRYS_NODISCARD virtual Expected<PipelineHandle> CreatePipeline(const PipelineDesc &desc) noexcept = 0; // error: link log
  KRYS_NODISCARD virtual Expected<SwapchainHandle> CreateSwapchain(const SwapchainDesc &desc) noexcept = 0;

  virtual void Destroy(BufferHandle handle) noexcept = 0; // one overload per handle type; deferred
  virtual void ResizeSwapchain(SwapchainHandle swapchain, uint32 width, uint32 height) noexcept = 0;

  virtual void WriteBuffer(BufferHandle buffer, uint64 offset, Span<const byte> data) noexcept = 0;
  virtual void WriteTexture(TextureHandle texture, const TextureRegion &region, Span<const byte> data) noexcept = 0;
  virtual void GenerateMipmaps(TextureHandle texture) noexcept = 0;
  KRYS_NODISCARD virtual TransientAllocation AllocateTransient(uint64 size, uint32 alignment = 256u) noexcept = 0;

  virtual void BeginFrame() noexcept = 0;
  KRYS_NODISCARD virtual CommandBuffer &GetCommandBuffer() noexcept = 0;
  virtual void Submit(CommandBuffer &commands) noexcept = 0;
  virtual void Present(SwapchainHandle swapchain) noexcept = 0;
  virtual void EndFrame() noexcept = 0;

  KRYS_NODISCARD virtual const DeviceLimits &Limits() const noexcept = 0;
};

/// @brief Per-frame memory from the device's ring buffer, valid until EndFrame.
struct TransientAllocation
{
  BufferHandle Buffer {};
  uint64 Offset {0u};
  Span<byte> Data {};
};
```

`Device` is virtual, but its calls are coarse; per-draw work goes through the command buffer.

Errors:
- Creation returns `Expected<THandle>` with a message. Shader and pipeline failures include the compile or link log.
- Using an invalid handle asserts in debug builds and does nothing in release builds.
- The OpenGL debug output stays as it is.

### Using it

Setup:

```cpp
auto device = gfx::Device::Create({.EnableValidation = true});
auto swapchain = device->CreateSwapchain({.Window = window.GetWindowHandle(), .Width = w, .Height = h, .VSync = true});

auto vs = device->CreateShader({.Stage = ShaderStage::Vertex, .Source = vertexSource, .DebugName = "Triangle.vert"});
auto fs = device->CreateShader({.Stage = ShaderStage::Fragment, .Source = fragmentSource, .DebugName = "Triangle.frag"});
if (!vs || !fs)
{
  KRYS_LOG_ERROR("{}", !vs ? vs.error() : fs.error()); // the compile log, with line numbers
  return;
}

auto pipeline = device->CreatePipeline({
  .VertexShader = *vs,
  .FragmentShader = *fs,
  .VertexBuffers = {{.Stride = sizeof(Vertex),
                     .Attributes = {{0, VertexFormat::Float2, offsetof(Vertex, Position)},
                                    {1, VertexFormat::UByte4Norm, offsetof(Vertex, Colour)}}}},
  .Blend = BlendState::PremultipliedAlpha(),
  .ColourFormats = {Format::BGRA8Srgb},
  .DebugName = "Triangle",
});

auto vertices = device->CreateBuffer({.Size = sizeof(triangle), .Usage = BufferUsage::Vertex,
                                      .InitialData = AsBytes(triangle), .DebugName = "TriangleVertices"});
```

Each frame:

```cpp
device->BeginFrame();
auto &cmd = device->GetCommandBuffer();

auto frame = device->AllocateTransient(sizeof(FrameUniforms));
WriteInto(frame.Data, FrameUniforms {.Projection = Ortho(0, w, h, 0), .Time = t});

cmd.BeginRenderPass({.Colour = {{.Target = *swapchain, .Clear = {0.1f, 0.1f, 0.1f, 1.f}}}, .DebugName = "Main"});
cmd.BindPipeline(*pipeline);
cmd.BindUniformBuffer(0, frame.Buffer, frame.Offset, sizeof(FrameUniforms));
cmd.BindVertexBuffer(0, *vertices);
cmd.Draw(3);
cmd.EndRenderPass();

device->Submit(cmd);
device->Present(*swapchain);
device->EndFrame();
```

Rendering to a texture (an in-world UI screen) is the same, with a texture as the target:

```cpp
auto screen = device->CreateTexture({.Format = Format::RGBA8Srgb, .Width = 1024, .Height = 768,
                                     .Usage = TextureUsage::RenderTarget | TextureUsage::Sampled,
                                     .DebugName = "TerminalScreen"});
cmd.BeginRenderPass({.Colour = {{.Target = *screen}}});
// ... Render2D draws the page ...
cmd.EndRenderPass();
// later, in the 3D pass: cmd.BindTexture(0, *screen, linearSampler);
```

### The OpenGL backend

| RHI | OpenGL 4.5 |
|---|---|
| Buffer | `glCreateBuffers` + `glNamedBufferStorage` (immutable); `Dynamic` adds `GL_DYNAMIC_STORAGE_BIT`. |
| Texture | `glCreateTextures` + `glTextureStorage*` (the current `Image` wrapper). |
| Sampler | A sampler object (`glCreateSamplers`), deduplicated by hashing its description. |
| Shader | `glCreateShader` and compile; failure returns the info log. |
| Pipeline | A linked program, a VAO built from the vertex layout (`glVertexArrayAttribFormat`, `glVertexArrayAttribBinding`), and the blend, depth and raster state as data. |
| `BindPipeline` | `glUseProgram`, bind the VAO, apply only the state that differs from the last pipeline (a cached state shadow). |
| `BindVertexBuffer` | `glVertexArrayVertexBuffer(vao, slot, buffer, offset, stride)`, so one VAO per pipeline serves any buffer. |
| `BindUniformBuffer`, `BindTexture` | `glBindBufferRange(GL_UNIFORM_BUFFER, slot, ...)`; `glBindTextureUnit` + `glBindSampler`. |
| `PushConstants` | `glNamedBufferSubData` into a reserved uniform buffer at binding 15. |
| Render pass | A framebuffer object from a cache keyed by the attachments (framebuffer 0 for a swapchain), `glClearNamedFramebuffer*` for `Clear`, `glInvalidateNamedFramebufferData` for `DontCare` stores. |
| Transient memory | One persistently mapped buffer (`glNamedBufferStorage` with `GL_MAP_PERSISTENT_BIT`), split into a region per frame in flight, each guarded by a `glFenceSync`. |
| Deferred destruction | `Destroy` moves the object from the pool into the current frame's graveyard, emptied when that frame's fence signals. |
| Swapchain | The WGL context and the window's default framebuffer; `Present` is `SwapBuffers`. |
| Debug names and groups | `glObjectLabel`, `glPushDebugGroup`. |

The current `Buffer`, `Image`, `Sampler` and `Shader` wrappers carry over almost unchanged. The new parts are pipelines, the framebuffer cache, the state shadow and the transient ring.

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
| `ResourceManager` | Rewritten as `Pool` (no exceptions, 24/8-bit handles). |
| `ResourceHandleCache` | Deleted; caching moves to the asset layer. |
| `I*Registry` and the OpenGL registries | Replaced by `Device`. The OpenGL wrappers (`Buffer`, `Image`, `Sampler`, `Shader`) are reused inside the backend. |
| `IContext`, `IRenderer`, `Commands.hpp` | Replaced by `Device`, `CommandBuffer` and `Renderer2D`. |
| `Colour.hpp`, `ColourFormatter.hpp`, `ColourHasher.hpp` | Deleted; the colour types move from Booey to `Krystal.Core/Colour/`. |
| `Context.cpp`'s demo scenes and their shaders (`7/` to `11/`, shadows, SSAO, PBR, IBL) | Moved to Krystal.Samples, or deleted. |
| `Mesh`, `Material`, `FlatColourMaterials`, `Light`, `ICamera`, `FirstPersonCamera`, `MeshDataUtils`, `InstanceData`, `Vertex.hpp` | Moved out of Gfx, to the future 3D renderer (or the samples). |
| `FontAtlasLoader`, msdf-atlas-gen | Kept, unused until text in 3D. |
| `ShaderReflector` | Kept, to check pipeline binding layouts in debug builds. |
| `Krystal.Gfx.Vulkan`, `Krystal.Gfx.D3D11` | Deleted or parked: they're tutorial contexts that don't implement `IContext`. |
| `Handle.hpp` in Krystal.Core (namespace `krys::Gfx`) | Replaced by `Krystal.Gfx/Handles.hpp`. |
| `Texture.hpp` in Krystal.Gfx (namespace `Gfx::OpenGL`) | Deleted with the registries. |

## Slices

| Slice | Content | Done when |
|---|---|---|
| C0 | Move the colour types from Booey to `Krystal.Core/Colour/`, renamed to "Colour" in `krys`; delete Gfx's colour headers. | Booey and its tests build and pass against the moved types. |
| G1 | The RHI: `Device`, `Pool` and handles, buffers, textures, samplers, shaders, pipelines, command buffers, swapchains, transient memory; the OpenGL backend. The editor moves onto it. | The editor clears its window and draws a triangle through the RHI. The old registries are gone. |
| G2 | Render2D: rects, rounded rects, borders, box shadows, images, gradients, clips, transforms, layers. | A test app draws a UI-like scene; golden image tests pass. |
| G3 | Text: `GlyphAtlas`, `DrawGlyphs`, FreeType rasterisation. | Text renders crisply at several sizes and scales. |
| G4 | Booey: `GraphicsContext`, the display list recorder, painting (paint order, backgrounds, borders, text); Krystal.UI replays it. | The vertical slice: a page loads, renders, resizes and responds to a click. |

Testing: a fixture with a hidden window gives the RHI a real OpenGL context. Render2D gets golden image tests: render offscreen, read back, compare against a PNG with a tolerance.

## Open decisions

1. **Backend selection:** a virtual `Device` chosen at run time (as drafted), or one backend chosen at compile time with no virtual calls.
2. **Old backends and samples:** delete the Vulkan and D3D11 stubs, or keep them parked. Keep the LearnOpenGL scenes as Krystal.Samples, or delete them.
3. **Module names:** Krystal.Gfx for the RHI, Krystal.Render2D, Krystal.UI, Krystal.Assets.
4. **Shaders:** GLSL with explicit binding slots now. Later, possibly SPIR-V compiled offline, which a Vulkan backend would need.
5. **Text:** FreeType greyscale bitmaps first (recommended), or MSDF.
