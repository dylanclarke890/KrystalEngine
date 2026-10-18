#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Colour/AlphaPremultiplication.hpp"
#include "Krystal.Core/Colour/ColourTypes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Gfx/CommandBuffer.hpp"
#include "Krystal.Gfx/Device.hpp"
#include "Krystal.Gfx/Formats.hpp"
#include "Krystal.Gfx/Handles.hpp"
#include "Krystal.Gfx/Render2D/MaskAtlas.hpp"
#include "Krystal.Gfx/Render2D/Path2D.hpp"
#include "Krystal.Gfx/Render2D/PathStroker.hpp"
#include "Krystal.Gfx/Render2D/Rect2D.hpp"
#include "Krystal.Gfx/Render2D/RoundedRect2D.hpp"
#include "Krystal.Gfx/Render2D/StrokeStyle.hpp"
#include "Krystal.Gfx/Render2D/Transform2D.hpp"
#include "Krystal.Gfx/Sampler.hpp"

namespace krys::gfx
{
  /// @brief What a 2D frame draws into: a swapchain or a texture.
  struct Render2DTarget
  {
    /// @brief Used instead of `Texture` when valid.
    SwapchainHandle Swapchain {};
    TextureHandle Texture {};
    uint32 Width {0u};
    uint32 Height {0u};
    TextureFormat Format {TextureFormat::RGBA8Srgb};
    /// @brief The target is cleared to this value first, in the target's encoding; otherwise its contents
    /// are kept.
    Maybe<Array<float, 4>> ClearColour {};
  };

  /// @brief How `Renderer2D::DrawImage` samples and blends an image.
  struct ImageOptions
  {
    SamplerFilter Filter {SamplerFilter::Linear};
    /// @brief Whether the texture's colours are already multiplied by their alpha.
    AlphaPremultiplication Alpha {AlphaPremultiplication::Premultiplied};
    /// @brief Multiplies the image's alpha, from 0 to 1.
    float Opacity {1.f};
  };

  struct Renderer2DDesc
  {
    /// @brief The atlas that caches the coverage masks of paths.
    MaskAtlasDesc Masks {};
  };

  /// @brief What `Renderer2D` drew in the last frame `End` recorded.
  struct Renderer2DStats
  {
    uint32 Instances {0u};
    uint32 Batches {0u};
    /// @brief Paths rasterised because their masks weren't cached.
    uint32 MasksRasterised {0u};
    /// @brief Paths drawn from cached masks.
    uint32 MaskCacheHits {0u};
  };

  /// @brief Draws 2D primitives through the RHI, batched in painter's order.
  ///
  /// Coordinates are pixels with a top-left origin, y pointing down. Colours are sRGB with straight alpha;
  /// they're premultiplied for blending, in linear light on sRGB targets and as encoded on other targets.
  /// Images are converted the same way: textures in sRGB formats are decoded by the GPU, others hold
  /// sRGB-encoded values.
  /// Edges are antialiased over one target pixel, centred on the edge, so edges on pixel boundaries stay
  /// sharp.
  ///
  /// Drawing state (the transform and the clip) is a stack: `Save` pushes a copy, `Restore` pops it. Draws
  /// are made in the current transform's coordinates and clipped to the current clip. Clip edges are
  /// antialiased like any other edge: each draw's coverage is multiplied by how much of the pixel its clips
  /// let through.
  ///
  /// A frame, between `Device::BeginFrame` and `Device::Submit`:
  /// @code
  /// renderer.Begin(commands, target);
  /// renderer.FillRect({10, 10, 100, 50}, SRGBA<uint8> {255, 0, 0});
  /// renderer.Save();
  /// renderer.Translate(10, 70);
  /// renderer.ClipRect({0, 0, 50, 50});
  /// renderer.FillRect({0, 0, 100, 100}, SRGBA<uint8> {0, 0, 255});
  /// renderer.Restore();
  /// renderer.End();
  /// @endcode
  class Renderer2D
  {
    KRYS_NON_COPY_MOVABLE(Renderer2D);

    /// @brief One rounded rect, as the shader reads it per instance.
    struct RectInstance
    {
      Array<float, 4> Rect;
      /// @brief Top-left, top-right, bottom-right, bottom-left.
      Array<float, 4> Radii;
      /// @brief The transform's A, B, C and D.
      Array<float, 4> Linear;
      /// @brief The transform's E and F.
      Array<float, 2> Translation;
      /// @brief The texture coordinates at the rect's top-left and bottom-right corners.
      Array<float, 4> UV;
      /// @brief A rounded rect cut out of this one, in the same coordinates: x, y, width and height.
      Array<float, 4> HoleRect;
      /// @brief The hole's top-left, top-right, bottom-right and bottom-left radii.
      Array<float, 4> HoleRadii;
      Array<uint8, 4> Colour;
      /// @brief `InstanceFlags`.
      uint32 Flags;
    };

    enum InstanceFlags : uint32
    {
      /// @brief The colour is multiplied by a texel of the batch's texture.
      Textured = 1u << 0u,
      /// @brief The texture's colours aren't premultiplied.
      StraightAlpha = 1u << 1u,
      /// @brief The texture is in an sRGB format, so the GPU decodes it to linear light.
      LinearTexels = 1u << 2u,
      /// @brief The texture holds coverage in its red channel, which multiplies the colour.
      Mask = 1u << 3u,
      /// @brief The hole rect is cut out.
      Hole = 1u << 4u,
    };

    /// @brief A chain's state when it has no shape clips.
    constexpr static uint32 NoChain = ~0u;
    /// @brief The most shape clips a chain holds.
    constexpr static uint32 MaxClips = 16u;

    struct DrawState
    {
      Transform2D Transform {};
      /// @brief The clip's bounds, in target pixels; draws outside them are discarded by the scissor.
      PixelRect2D Clip {};
      /// @brief The shape clips in effect, an index into the frame's chains, or `NoChain`.
      uint32 Chain {NoChain};
    };

    /// @brief A clip to a rounded rect, or out of one, in the coordinates of the transform it was made under.
    struct ClipShape
    {
      RoundedRect2D Shape {};
      /// @brief Maps target pixels into the shape's coordinates.
      Transform2D Inverse {};
      bool Out {false};
    };

    /// @brief The shape clips in effect at once; they multiply.
    /// @brief A clip to a path, or out of one, under the transform it was made under.
    struct PathClip
    {
      Path2D Path {};
      Transform2D Transform {};
      FillRule Rule {FillRule::NonZero};
      bool Out {false};
    };

    /// @brief The clips in effect at once; they multiply.
    struct ClipChain
    {
      SmallList<ClipShape, 4> Shapes {};
      List<PathClip> Paths {};
      /// @brief The path clips' coverage: one path's cached mask, or several multiplied for this frame.
      MaskLocation Mask {};
      /// @brief Whether the mask is clipped out rather than to.
      bool MaskOut {false};
    };

    enum ClipMaskMode : uint32
    {
      NoClipMask = 0u,
      ClipToMask = 1u,
      ClipOutMask = 2u,
    };

    /// @brief One shape clip, as the shader reads it (std140).
    struct ClipData
    {
      Array<float, 4> Rect;
      Array<float, 4> Radii;
      /// @brief The inverse transform's A, B, C and D.
      Array<float, 4> InverseLinear;
      /// @brief The inverse transform's E and F, then 1 for a clip-out.
      Array<float, 4> InverseTranslation;
    };

    struct ClipUniforms
    {
      uint32 Count;
      /// @brief `ClipMaskMode`.
      uint32 MaskMode;
      Array<uint32, 2> Padding;
      /// @brief The target pixels the mask covers: x, y, width and height.
      Array<int32, 4> MaskBounds;
      /// @brief The mask's first texel in its texture, then padding.
      Array<int32, 4> MaskOrigin;
      Array<ClipData, MaxClips> Clips;
    };
    static_assert(sizeof(ClipData) == 64u, "ClipData mirrors the shader's std140 Clip.");
    static_assert(sizeof(ClipUniforms) == 48u + 64u * MaxClips,
                  "ClipUniforms mirrors the shader's std140 block.");

    /// @brief A run of instances drawn with one scissor and texture. Untextured instances join any batch.
    struct Batch
    {
      uint32 FirstInstance {0u};
      uint32 InstanceCount {0u};
      PixelRect2D Scissor {};
      uint32 Chain {NoChain};
      TextureHandle Texture {};
      SamplerHandle Sampler {};
    };

    struct FrameUniforms
    {
      Array<float, 2> ViewportSize;
      uint32 DecodeSrgb;
      uint32 Padding;
    };

    struct TargetPipeline
    {
      TextureFormat Format {TextureFormat::Undefined};
      PipelineHandle Pipeline {};
    };

  private:
    Device *_device;
    ShaderHandle _vertexShader {};
    ShaderHandle _fragmentShader {};
    SamplerHandle _linearSampler {};
    SamplerHandle _nearestSampler {};
    /// @brief Bound for batches without a texture of their own.
    TextureHandle _whiteTexture {};
    MaskAtlas _masks;
    PathStroker _stroker {};
    /// @brief Combines nested path clips.
    PathRasteriser _rasteriser {};
    List<float> _combinedClip {};
    List<float> _dashes {};
    Renderer2DStats _stats {};
    SmallList<TargetPipeline, 2> _pipelines {};
    CommandBuffer *_commands {nullptr};
    Render2DTarget _target {};
    DrawState _state {};
    List<DrawState> _savedStates {};
    List<RectInstance> _rects {};
    List<Batch> _batches {};
    List<ClipChain> _chains {};

  public:
    /// @brief Compiles the shaders; pipelines are created per target format on first use.
    KRYS_NODISCARD static Expected<UniquePtr<Renderer2D>> Create(Device &device,
                                                                 const Renderer2DDesc &desc = {}) noexcept;

    /// @brief Use `Create`.
    Renderer2D(Device &device, const Renderer2DDesc &desc) noexcept;
    ~Renderer2D() noexcept;

    /// @brief Starts drawing into a target. Draws are recorded into `commands` on `End`.
    void Begin(CommandBuffer &commands, const Render2DTarget &target) noexcept;
    /// @brief Records the frame: one render pass into the target, then the draws in the order they were
    /// made.
    void End() noexcept;

    /// @brief Pushes a copy of the drawing state.
    void Save() noexcept;
    /// @brief Pops the drawing state pushed by the matching `Save`.
    void Restore() noexcept;

    /// @brief Applies `transform` before the current transform: later draws are in its coordinates.
    void ConcatTransform(const Transform2D &transform) noexcept;
    void Translate(float x, float y) noexcept;
    void Scale(float x, float y) noexcept;
    /// @brief Rotates by `radians`, clockwise on screen.
    void Rotate(float radians) noexcept;
    /// @brief Replaces the current transform, relative to the target's pixels.
    void SetTransform(const Transform2D &transform) noexcept;
    KRYS_NODISCARD const Transform2D &GetTransform() const noexcept;

    /// @brief Intersects the clip with a rect in the current coordinates. A rect whose edges land on whole
    /// target pixels, under a transform that keeps rects axis-aligned, is the scissor alone.
    void ClipRect(const Rect2D &rect) noexcept;
    /// @brief Intersects the clip with a rounded rect in the current coordinates, its radii constrained as
    /// `FillRoundedRect`'s are.
    void ClipRoundedRect(const RoundedRect2D &rect) noexcept;
    /// @brief Clips out a rect in the current coordinates: later draws skip it.
    void ClipOutRect(const Rect2D &rect) noexcept;
    /// @brief Clips out a rounded rect in the current coordinates.
    void ClipOutRoundedRect(const RoundedRect2D &rect) noexcept;
    /// @brief Intersects the clip with a path in the current coordinates, through its coverage mask.
    /// @note One path clip at a time uses its cached mask; nesting a second multiplies their coverage on the
    /// CPU into a mask over the clip's bounds, made again every frame.
    void ClipPath(const Path2D &path, FillRule rule) noexcept;
    /// @brief Clips out a path in the current coordinates.
    void ClipOutPath(const Path2D &path, FillRule rule) noexcept;

    void FillRect(const Rect2D &rect, SRGBA<uint8> colour) noexcept;
    /// @brief Fills a rounded rect. Negative radii are treated as zero, and radii that don't fit are scaled
    /// down together until they do, as CSS does for `border-radius`.
    void FillRoundedRect(const RoundedRect2D &rect, SRGBA<uint8> colour) noexcept;
    /// @brief Fills a rect except for a rounded rect inside it: a border's usual shape. Its radii are
    /// constrained as `FillRoundedRect`'s are.
    /// @note Holes have circular corners. For elliptical ones, fill a path with the rect and the hole under
    /// `FillRule::EvenOdd`.
    void FillRectWithRoundedHole(const Rect2D &rect, const RoundedRect2D &hole, SRGBA<uint8> colour) noexcept;

    /// @brief Draws the `source` texels of a texture into `destination`, scaling to fit.
    void DrawImage(TextureHandle texture, const Rect2D &source, const Rect2D &destination,
                   const ImageOptions &options = {}) noexcept;
    /// @brief Fills a path under the fill rule. The path is rasterised on the CPU into a coverage mask, which
    /// is cached, so drawing the same path again (or moved by whole pixels) costs only the draw.
    void FillPath(const Path2D &path, FillRule rule, SRGBA<uint8> colour) noexcept;

    /// @brief Strokes a path. The stroke's outline is filled through a cached coverage mask, as `FillPath`
    /// fills; the outline is worked out on every call.
    void StrokePath(const Path2D &path, const StrokeStyle &style, SRGBA<uint8> colour) noexcept;
    /// @brief Strokes a straight line, with its caps and dashes, without a mask: each dash is a rect (a
    /// rounded one under round caps, so dots are circles) under the line's transform. The style's join
    /// and miter limit don't apply.
    void StrokeLine(Vec2f from, Vec2f to, const StrokeStyle &style, SRGBA<uint8> colour) noexcept;

    /// @brief Draws the whole of a texture into `destination`.
    void DrawImage(TextureHandle texture, const Rect2D &destination,
                   const ImageOptions &options = {}) noexcept;

    KRYS_NODISCARD const Renderer2DStats &GetStats() const noexcept
    {
      return _stats;
    }

  private:
    KRYS_NODISCARD Expected<void> Initialise() noexcept;
    KRYS_NODISCARD Expected<PipelineHandle> GetPipeline(TextureFormat format) noexcept;
    /// @brief Adds a rounded rect in `transform`'s coordinates, its radii constrained.
    void AddRoundedRect(const RoundedRect2D &rect, Array<uint8, 4> colour,
                        const Transform2D &transform) noexcept;
    /// @brief Fills in the instance's transform and adds it to the batch it can join, or a new one; skips it
    /// when nothing would show.
    void AddInstance(RectInstance instance, const Transform2D &transform, TextureHandle texture,
                     SamplerHandle sampler) noexcept;

    /// @brief Adds a shape clip to the current chain, in a new chain.
    void AddClip(RoundedRect2D shape, bool out) noexcept;
    /// @brief Adds a path clip to the current chain, in a new chain.
    void AddPathClip(const Path2D &path, FillRule rule, bool out) noexcept;

    /// @brief The target pixels whose centres are inside the rect from (left, top) to (right, bottom).
    KRYS_NODISCARD static PixelRect2D ToPixelRect(float left, float top, float right, float bottom) noexcept;
    KRYS_NODISCARD static PixelRect2D Intersect(const PixelRect2D &a, const PixelRect2D &b) noexcept;
  };
}
