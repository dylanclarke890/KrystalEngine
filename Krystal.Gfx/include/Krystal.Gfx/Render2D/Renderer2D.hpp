#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Colour/AlphaPremultiplication.hpp"
#include "Krystal.Core/Colour/BlendMode.hpp"
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
#include "Krystal.Gfx/Render2D/FontFace2D.hpp"
#include "Krystal.Gfx/Render2D/GlyphAtlas.hpp"
#include "Krystal.Gfx/Render2D/Gradient2D.hpp"
#include "Krystal.Gfx/Render2D/MaskAtlas.hpp"
#include "Krystal.Gfx/Render2D/Paint2D.hpp"
#include "Krystal.Gfx/Render2D/Path2D.hpp"
#include "Krystal.Gfx/Render2D/PathStroker.hpp"
#include "Krystal.Gfx/Render2D/Rect2D.hpp"
#include "Krystal.Gfx/Render2D/RoundedRect2D.hpp"
#include "Krystal.Gfx/Render2D/StrokeStyle.hpp"
#include "Krystal.Gfx/Render2D/Transform2D.hpp"
#include "Krystal.Gfx/Sampler.hpp"

namespace krys::gfx
{
  /// @brief What a 2D frame draws into: a swapchain or a texture, in a unorm format.
  struct Render2DTarget
  {
    /// @brief Used instead of `Texture` when valid.
    SwapchainHandle Swapchain {};
    TextureHandle Texture {};
    uint32 Width {0u};
    uint32 Height {0u};
    TextureFormat Format {TextureFormat::RGBA8Unorm};
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

  /// @brief A shadow cast by fills and strokes: a blurred, offset copy of each shape in one colour, drawn
  /// beneath it.
  struct DropShadow2D
  {
    /// @brief In the current coordinates, or in target pixels when shadows ignore transforms.
    Vec2f Offset {};
    /// @brief Twice the standard deviation of the Gaussian blur, as CSS's shadow blur radius is; 0 for a
    /// sharp shadow.
    float BlurRadius {0.f};
    /// @brief sRGB with straight alpha; multiplied by the alpha of a solid paint.
    SRGBA<uint8> Colour {};
  };

  struct Renderer2DDesc
  {
    /// @brief The atlas that caches the coverage masks of paths.
    MaskAtlasDesc Masks {};
    /// @brief The atlas that caches rasterised glyphs.
    GlyphAtlasDesc Glyphs {};
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
    /// @brief Glyphs rasterised because they weren't cached.
    uint32 GlyphsRasterised {0u};
    /// @brief Glyphs drawn from the glyph atlas's cache.
    uint32 GlyphCacheHits {0u};
    /// @brief Copies of what's beneath draws with blend modes.
    uint32 BackdropCopies {0u};
  };

  /// @brief Draws 2D primitives through the RHI, batched in painter's order.
  ///
  /// Coordinates are pixels with a top-left origin, y pointing down. Colours are sRGB with straight alpha;
  /// they're premultiplied and blended as encoded values, so targets are in unorm formats. Images in sRGB
  /// formats are decoded by the GPU and encoded back by the shader; others hold encoded values.
  /// Edges are antialiased over one target pixel, centred on the edge, so edges on pixel boundaries stay
  /// sharp.
  ///
  /// Drawing state (the transform, the clip, the blend mode and the shadow) is a stack: `Save` pushes a copy,
  /// `Restore` pops it. Draws are made in the current transform's coordinates and clipped to the current
  /// clip. Clip edges are antialiased like any other edge: each draw's coverage is multiplied by how much of
  /// the pixel its clips let through.
  ///
  /// Layers draw a group into an offscreen texture and composite it as one image: `BeginLayer` starts one
  /// (saving the drawing state) and `EndLayer` composites it at its opacity (restoring the state). At `End`,
  /// each layer gets a texture the size of what was drawn in it, within the clip, and its pass is recorded
  /// before the pass that composites it.
  ///
  /// Fills and strokes take a `Paint2D`: a colour, or a gradient. A gradient's stops are flattened on the CPU
  /// into stops linear in premultiplied encoded sRGB, and the shader finds each pixel's place in it; on 8-bit
  /// targets, gradients are dithered.
  ///
  /// A shadow is drawn beneath each fill and stroke while one is set. Rects, rounded rects and rounded holes
  /// blur analytically in the shader; paths blur their coverage masks on the CPU, cached with the masks.
  ///
  /// Blend modes mix a draw with what's beneath it. Most read it: the pass is split before the draw and the
  /// pixels it reaches are copied. A swapchain can't be copied from, so in a frame that blends straight onto
  /// one, the frame is drawn into a texture first; what was in the window before isn't blended with.
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
      /// @brief The texture is in an sRGB format: the GPU decodes it, so the shader encodes it back.
      LinearTexels = 1u << 2u,
      /// @brief The texture holds coverage in its red channel, which multiplies the colour.
      Mask = 1u << 3u,
      /// @brief The hole rect is cut out.
      Hole = 1u << 4u,
      /// @brief The colour is multiplied by the batch's gradient.
      GradientPaint = 1u << 5u,
      /// @brief A shadow: the rounded rect (less any hole) blurred by a Gaussian whose standard deviation, in
      /// local units, is the first texture coordinate.
      Shadow = 1u << 6u,
    };
    /// @brief The instance's `BlendMode` is in the flags from this bit, for five bits.
    constexpr static uint32 BlendModeShift = 8u;

    /// @brief A chain's state when it has no shape clips.
    constexpr static uint32 NoChain = ~0u;
    /// @brief The layer of draws made into the target itself.
    constexpr static uint32 NoLayer = ~0u;
    /// @brief The pass that composites the target, drawn offscreen, into its swapchain.
    constexpr static uint32 WindowLayer = NoLayer - 1u;
    /// @brief The most shape clips a chain holds.
    constexpr static uint32 MaxClips = 16u;
    /// @brief A batch's state when it has no gradient.
    constexpr static uint32 NoGradient = ~0u;

    struct DrawState
    {
      Transform2D Transform {};
      /// @brief The clip's bounds, in target pixels; draws outside them are discarded by the scissor.
      PixelRect2D Clip {};
      /// @brief The shape clips in effect, an index into the frame's chains, or `NoChain`.
      uint32 Chain {NoChain};
      BlendMode Blend {BlendMode::Normal};
      Maybe<DropShadow2D> DropShadow {};
      /// @brief Whether shadows' offsets and blurs are in target pixels rather than the current coordinates.
      bool ShadowsIgnoreTransforms {false};
    };

    /// @brief A clip to a rounded rect, or out of one, in the coordinates of the transform it was made under.
    struct ClipShape
    {
      RoundedRect2D Shape {};
      /// @brief Maps target pixels into the shape's coordinates.
      Transform2D Inverse {};
      bool Out {false};
    };

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

    enum GradientShape : uint32
    {
      LinearShape = 1u,
      RadialShape = 2u,
      ConicShape = 3u,
    };

    /// @brief A gradient as a frame keeps it, its stops in the frame's list.
    struct GradientData
    {
      /// @brief `GradientShape`.
      uint32 Shape {LinearShape};
      GradientSpreadMethod Spread {GradientSpreadMethod::Pad};
      /// @brief Maps target pixels into the shape's own space: along the line for a linear gradient (offset
      /// 0 to 1 along x), centred on the first circle for a radial one, centred and turned to start along x
      /// for a conic one.
      Transform2D Inverse {};
      /// @brief A radial gradient's second centre, relative to its first, then its start and end radii.
      Array<float, 4> Radial {};
      uint32 FirstStop {0u};
      uint32 StopCount {0u};
    };

    /// @brief One gradient, as the shader reads it (std140).
    struct GradientUniforms
    {
      uint32 Shape;
      /// @brief `GradientSpreadMethod`.
      uint32 Spread;
      uint32 StopCount;
      uint32 Padding;
      Array<float, 4> InverseLinear;
      /// @brief The inverse transform's E and F, then padding.
      Array<float, 4> InverseTranslation;
      Array<float, 4> Radial;
      Array<float, detail::MaxGradientStops> Offsets;
      Array<Array<float, 4>, detail::MaxGradientStops> Colours;
    };
    static_assert(sizeof(GradientUniforms) == 64u + 20u * detail::MaxGradientStops,
                  "GradientUniforms mirrors the shader's std140 block.");

    /// @brief Where a shape's shadow goes: the shape's transform moved by the offset, and the blur.
    struct ShadowPlacement
    {
      Transform2D Transform {};
      /// @brief The standard deviation in the shape's local units, and in target pixels.
      float Sigma {0.f};
      float PixelSigma {0.f};
      Array<uint8, 4> Colour {};
    };

    /// @brief A paint ready for an instance: its colour (white for a gradient) and gradient.
    struct ResolvedPaint
    {
      Array<uint8, 4> Colour {};
      uint32 Gradient {NoGradient};
    };

    /// @brief A run of instances drawn into one layer with one scissor, texture, gradient and blend mode.
    /// Untextured, solid instances join any batch.
    struct Batch
    {
      uint32 FirstInstance {0u};
      uint32 InstanceCount {0u};
      PixelRect2D Scissor {};
      uint32 Chain {NoChain};
      TextureHandle Texture {};
      SamplerHandle Sampler {};
      /// @brief The layer the batch draws into, or `NoLayer` for the target.
      uint32 Layer {NoLayer};
      /// @brief The layer this batch composites, whose texture is only known at `End`; or `NoLayer`.
      uint32 Composite {NoLayer};
      BlendMode Blend {BlendMode::Normal};
      /// @brief For a blend mode that reads what's beneath: the target pixels its instances reach, copied
      /// before it's drawn.
      PixelRect2D Backdrop {};
      /// @brief The gradient its gradient instances read, an index into the frame's gradients, or
      /// `NoGradient`.
      uint32 Gradient {NoGradient};
    };

    struct Layer
    {
      /// @brief The layer it's composited into, or `NoLayer` for the target.
      uint32 Parent {NoLayer};
      float Opacity {1.f};
      BlendMode Blend {BlendMode::Normal};
      /// @brief The clip when it began: nothing in it shows outside.
      PixelRect2D Clip {};
      /// @brief The target pixels its draws reach; at `End`, within the clip, its texture's extent.
      PixelRect2D Bounds {};
      /// @brief How many states were saved when it began, after its own save.
      size_t SaveDepth {0u};
      /// @brief Its composite instance in the parent, and that instance's batch.
      uint32 CompositeInstance {0u};
      uint32 CompositeBatch {0u};
      bool HasComposite {false};
      TextureHandle Texture {};
    };

    /// @brief A layer texture, reused by later frames.
    struct LayerTexture
    {
      TextureHandle Texture {};
      uint32 Width {0u};
      uint32 Height {0u};
      TextureFormat Format {TextureFormat::Undefined};
      uint64 LastUsedFrame {0u};
    };

    /// @brief Where a pass draws, and which layer's batches it draws.
    struct Pass
    {
      ColourAttachment Attachment {};
      /// @brief The swapchain's size; a pass into a texture takes the texture's, which the viewport covers.
      uint32 Width {0u};
      uint32 Height {0u};
      /// @brief The target pixel at the pass's top-left.
      int32 OriginX {0};
      int32 OriginY {0};
      uint32 Layer {NoLayer};
    };

    struct FrameUniforms
    {
      Array<float, 2> ViewportSize;
      /// @brief The target pixel at the viewport's top-left.
      Array<float, 2> Origin;
      /// @brief How far gradients are dithered: a colour step of the target's format, or 0.
      float Dither;
      Array<float, 3> Padding;
    };

    /// @brief What every pass of a frame draws with.
    struct FrameResources
    {
      PipelineHandle Pipeline {};
      /// @brief For `BlendMode::PlusLighter`.
      PipelineHandle AdditivePipeline {};
      TransientAllocation Instances {};
      /// @brief Where blend modes copy what's beneath them, laid out as the pass they're in.
      TextureHandle Backdrop {};
    };

    struct TargetPipeline
    {
      TextureFormat Format {TextureFormat::Undefined};
      /// @brief Whether it adds colours rather than blending them over.
      bool Additive {false};
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
    GlyphAtlas _glyphs;
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
    List<Layer> _layers {};
    /// @brief Layers in the order they ended: children before their parents.
    List<uint32> _layerOrder {};
    uint32 _currentLayer {NoLayer};
    List<LayerTexture> _layerTextures {};
    /// @brief The backdrop copies `End` has recorded this frame.
    uint32 _backdropCopies {0u};
    List<GradientData> _gradients {};
    List<detail::FlatGradientStop> _gradientStops {};
    List<detail::FlatGradientStop> _flattened {};

  public:
    /// @brief Past this many target pixels per em, glyphs are filled as outlines, as Skia draws large text.
    constexpr static float LargeGlyphSize = 256.f;

    /// @brief Compiles the shaders; pipelines are created per target format on first use.
    KRYS_NODISCARD static Expected<UniquePtr<Renderer2D>> Create(Device &device,
                                                                 const Renderer2DDesc &desc = {}) noexcept;

    /// @brief Use `Create`.
    Renderer2D(Device &device, const Renderer2DDesc &desc) noexcept;
    ~Renderer2D() noexcept;

    /// @brief Starts drawing into a target. Draws are recorded into `commands` on `End`.
    void Begin(CommandBuffer &commands, const Render2DTarget &target) noexcept;
    /// @brief Records the frame: a render pass per layer, children first, then the target's, each with its
    /// draws in the order they were made.
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

    void FillRect(const Rect2D &rect, const Paint2D &paint) noexcept;
    /// @brief Fills a rounded rect. Negative radii are treated as zero, and radii that don't fit are scaled
    /// down together until they do, as CSS does for `border-radius`.
    void FillRoundedRect(const RoundedRect2D &rect, const Paint2D &paint) noexcept;
    /// @brief Fills a rect except for a rounded rect inside it: a border's usual shape. Its radii are
    /// constrained as `FillRoundedRect`'s are.
    /// @note Holes have circular corners. For elliptical ones, fill a path with the rect and the hole under
    /// `FillRule::EvenOdd`.
    void FillRectWithRoundedHole(const Rect2D &rect, const RoundedRect2D &hole,
                                 const Paint2D &paint) noexcept;

    /// @brief Draws the `source` texels of a texture into `destination`, scaling to fit.
    void DrawImage(TextureHandle texture, const Rect2D &source, const Rect2D &destination,
                   const ImageOptions &options = {}) noexcept;
    /// @brief Fills a path under the fill rule. The path is rasterised on the CPU into a coverage mask, which
    /// is cached, so drawing the same path again (or moved by whole pixels) costs only the draw.
    void FillPath(const Path2D &path, FillRule rule, const Paint2D &paint) noexcept;

    /// @brief Strokes a path. The stroke's outline is filled through a cached coverage mask, as `FillPath`
    /// fills; the outline is worked out on every call.
    void StrokePath(const Path2D &path, const StrokeStyle &style, const Paint2D &paint) noexcept;
    /// @brief Strokes a straight line, with its caps and dashes, without a mask: each dash is a rect (a
    /// rounded one under round caps, so dots are circles) under the line's transform. The style's join
    /// and miter limit don't apply. A gradient is in the current coordinates, not the line's.
    void StrokeLine(Vec2f from, Vec2f to, const StrokeStyle &style, const Paint2D &paint) noexcept;

    /// @brief Draws glyphs of `face` at `size` pixels per em (in the current coordinates), each with its pen
    /// at `origin` plus its position, on the baseline. Pens snap to a quarter of a target pixel across and a
    /// whole one down; glyphs come from the glyph atlas, rasterised under the transform (at the scaled size
    /// under a uniform scale, so hinting applies). Text over `LargeGlyphSize` target pixels per em is filled
    /// as outlines instead. Colour glyphs keep their colours, taking only the paint's alpha.
    /// @note Shadows follow each glyph: sharp ones are the glyph again, blurred ones its outline's blurred
    /// mask, unhinted.
    void DrawGlyphs(const FontFace2D &face, float size, Span<const uint32> glyphs,
                    Span<const Vec2f> positions, Vec2f origin, const Paint2D &paint) noexcept;

    /// @brief Draws the whole of a texture into `destination`.
    void DrawImage(TextureHandle texture, const Rect2D &destination,
                   const ImageOptions &options = {}) noexcept;

    /// @brief Sets the shadow later fills and strokes cast; images cast none. A shadow with no offset and no
    /// blur casts nothing.
    /// @note A gradient's shadow is its shape's, as if the gradient were opaque.
    void SetDropShadow(const DropShadow2D &shadow) noexcept;
    void ClearDropShadow() noexcept;
    KRYS_NODISCARD const Maybe<DropShadow2D> &GetDropShadow() const noexcept;
    /// @brief Whether shadows' offsets and blurs are in target pixels (as canvas has them) rather than the
    /// current coordinates.
    void SetShadowsIgnoreTransforms(bool ignore) noexcept;
    KRYS_NODISCARD bool GetShadowsIgnoreTransforms() const noexcept;

    /// @brief Sets the blend mode later draws mix with what's beneath them by.
    /// @note `BlendMode::PlusDarker` isn't supported: it asserts and draws normally.
    void SetBlendMode(BlendMode mode) noexcept;
    KRYS_NODISCARD BlendMode GetBlendMode() const noexcept;

    /// @brief Saves the drawing state and starts drawing into a layer, composited by `EndLayer` at
    /// `opacity` (from 0 to 1) with the current blend mode. Inside the layer, the blend mode is `Normal`.
    void BeginLayer(float opacity) noexcept;
    /// @brief Starts a layer as `BeginLayer(opacity)` does, composited with `blend`.
    void BeginLayer(float opacity, BlendMode blend) noexcept;
    /// @brief Composites the current layer into what's beneath it and restores the drawing state saved by
    /// `BeginLayer`.
    void EndLayer() noexcept;

    KRYS_NODISCARD const Renderer2DStats &GetStats() const noexcept
    {
      return _stats;
    }

  private:
    KRYS_NODISCARD Expected<void> Initialise() noexcept;
    KRYS_NODISCARD Expected<PipelineHandle> GetPipeline(TextureFormat format, bool additive) noexcept;
    /// @brief Adds a rounded rect in `transform`'s coordinates, its radii constrained.
    void AddRoundedRect(const RoundedRect2D &rect, const ResolvedPaint &paint,
                        const Transform2D &transform) noexcept;
    /// @brief Fills in the instance's transform and adds it to the batch it can join, or a new one; skips it
    /// when nothing would show.
    void AddInstance(RectInstance instance, const Transform2D &transform, TextureHandle texture,
                     SamplerHandle sampler, uint32 gradient = NoGradient) noexcept;

    /// @brief The paint's colour, or its gradient added to the frame under the current transform; nothing
    /// when it shows nothing. A gradient that comes down to one colour is that colour.
    KRYS_NODISCARD Maybe<ResolvedPaint> ResolvePaint(const Paint2D &paint) noexcept;
    /// @brief Where the shadow of a shape drawn under `transform` with `paint` goes; nothing when there's no
    /// shadow to show.
    KRYS_NODISCARD Maybe<ShadowPlacement> PlaceShadow(const Transform2D &transform,
                                                      const ResolvedPaint &paint) const noexcept;
    /// @brief Adds a mask's instance, drawn on whole target pixels, unless it's empty or outside the clip.
    void AddMask(const MaskLocation &mask, Array<uint8, 4> colour, uint32 gradient) noexcept;
    /// @brief Adds a glyph's instance from the atlas, its pen's pixel at (`x`, `y`) in target pixels.
    void AddGlyph(const GlyphLocation &glyph, int32 x, int32 y, const ResolvedPaint &paint) noexcept;
    /// @brief Whether a paint certainly shows nothing, before the work of resolving it.
    KRYS_NODISCARD static bool IsInvisible(const Paint2D &paint) noexcept;

    /// @brief Works out a finished layer's bounds and texture, and fills in its composite.
    void FinishLayer(uint32 index) noexcept;
    /// @brief A layer texture at least `width` by `height` that no earlier pass of this frame uses.
    KRYS_NODISCARD TextureHandle AcquireLayerTexture(uint32 width, uint32 height) noexcept;
    /// @brief Records one pass: its layer's batches into its attachment, split before each batch that
    /// copies what's beneath it.
    void RecordPass(CommandBuffer &commands, const Pass &pass, const FrameResources &resources) noexcept;

    /// @brief Adds a shape clip to the current chain, in a new chain.
    void AddClip(RoundedRect2D shape, bool out) noexcept;
    /// @brief Adds a path clip to the current chain, in a new chain.
    void AddPathClip(const Path2D &path, FillRule rule, bool out) noexcept;

    /// @brief The target pixels an instance's quad reaches under `transform` (grown by a pixel for its
    /// antialiased edge), within the clip.
    KRYS_NODISCARD PixelRect2D Reach(const RectInstance &instance,
                                     const Transform2D &transform) const noexcept;
    /// @brief Whether a blend mode reads what's beneath the draw, so it's copied first.
    KRYS_NODISCARD static bool CopiesBackdrop(BlendMode mode) noexcept;

    /// @brief The target pixels whose centres are inside the rect from (left, top) to (right, bottom).
    KRYS_NODISCARD static PixelRect2D ToPixelRect(float left, float top, float right, float bottom) noexcept;
    KRYS_NODISCARD static PixelRect2D Intersect(const PixelRect2D &a, const PixelRect2D &b) noexcept;
    /// @brief The smallest rect holding both; an empty one adds nothing.
    KRYS_NODISCARD static PixelRect2D Union(const PixelRect2D &a, const PixelRect2D &b) noexcept;
  };
}
