#include "Krystal.Booey/Compositing/TextureMapper.hpp"
#include "Krystal.Booey/GraphicsContextRender2D.hpp"
#include "Krystal.Booey/ImageBufferTextures.hpp"
#include "Krystal.Booey/Render2DConversions.hpp"
#include "Krystal.Core/Colour/ColourMatrix.hpp"
#include "Krystal.Core/Containers/Array.hpp"
#include "Krystal.Core/Geometry/Quad.hpp"
#include "Krystal.Web/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Web/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Web/Graphics/Filters/FilterOperations.hpp"
#include "Krystal.Web/Graphics/Images/Image.hpp"
#include "Krystal.Web/Graphics/Layers/GraphicsLayer.hpp"
#include "Krystal.Web/Graphics/Paths/Path.hpp"
#include <algorithm>
#include <string>

namespace krys::boo
{
  namespace
  {
    /// @brief A matrix's rows as a layer filter's colour matrix.
    template <size_t ColumnCount, size_t RowCount>
    KRYS_NODISCARD Array<float, 20> RowsOf(const ColourMatrix<ColumnCount, RowCount> &matrix) noexcept
    {
      const ColourMatrix<5, 4> rowMatrix(matrix);
      const auto &data = rowMatrix.data();
      Array<float, 20> rows {};
      std::copy(data.begin(), data.end(), rows.begin());
      return rows;
    }

    /// @brief The layer filter `operation` is, if Render2D can apply it, at @p scale target pixels a layer
    /// pixel (its blurs and offsets are in target pixels).
    KRYS_NODISCARD Maybe<gfx::LayerFilter2D> LayerFilterFor(const web::FilterOperation &operation,
                                                            gfx::EdgeMode2D blurEdgeMode,
                                                            float scale) noexcept
    {
      using Type = web::FilterOperation::Type;
      switch (operation.GetType())
      {
        case Type::Grayscale:
        case Type::Sepia:
        case Type::Saturate:
        case Type::HueRotate:
        {
          auto amount =
            static_cast<float>(Downcast<web::BasicColorMatrixFilterOperation>(operation).Amount());
          switch (operation.GetType())
          {
            case Type::Grayscale:
              return gfx::LayerFilter2D::ColourMatrix(RowsOf(GrayscaleColourMatrix(amount)));
            case Type::Sepia: return gfx::LayerFilter2D::ColourMatrix(RowsOf(SepiaColourMatrix(amount)));
            case Type::Saturate:
              return gfx::LayerFilter2D::ColourMatrix(RowsOf(SaturationColourMatrix(amount)));
            default: return gfx::LayerFilter2D::ColourMatrix(RowsOf(HueRotateColourMatrix(amount)));
          }
        }
        case Type::Invert:
        case Type::Opacity:
        case Type::Brightness:
        case Type::Contrast:
        {
          auto amount =
            static_cast<float>(Downcast<web::BasicComponentTransferFilterOperation>(operation).Amount());
          switch (operation.GetType())
          {
            case Type::Invert:  return gfx::LayerFilter2D::ColourMatrix(RowsOf(InvertColourMatrix(amount)));
            case Type::Opacity: return gfx::LayerFilter2D::ColourMatrix(RowsOf(OpacityColourMatrix(amount)));
            case Type::Brightness:
              return gfx::LayerFilter2D::ColourMatrix(RowsOf(BrightnessColourMatrix(amount)));
            default: return gfx::LayerFilter2D::ColourMatrix(RowsOf(ContrastColourMatrix(amount)));
          }
        }
        case Type::Blur:
        {
          // A layer filter's blur radius is twice the standard deviation.
          return gfx::LayerFilter2D::Blur(
            Downcast<web::BlurFilterOperation>(operation).StdDeviation() * 2.f * scale, blurEdgeMode);
        }
        case Type::DropShadow:
        {
          const auto &shadow = Downcast<web::DropShadowFilterOperation>(operation);
          return gfx::LayerFilter2D::DropShadow(
            {.Offset = {static_cast<float>(shadow.X()) * scale, static_cast<float>(shadow.Y()) * scale},
             .BlurRadius = static_cast<float>(shadow.StdDeviation()) * 2.f * scale,
             .Colour = ToColour(shadow.GetColor())});
        }
        default:
          // FIXME: Reference filters; the rest are identities.
          return null;
      }
    }
  }

  Expected<UniquePtr<TextureMapper>> TextureMapper::Create(gfx::Device &device, FontFaceCache &fonts) noexcept
  {
    auto contentsRenderer = gfx::Renderer2D::Create(device);
    if (!contentsRenderer)
    {
      return Unexpected<string>(contentsRenderer.error());
    }
    return UniquePtr<TextureMapper>(new TextureMapper(device, fonts, krys::move(*contentsRenderer)));
  }

  TextureMapper::TextureMapper(gfx::Device &device, FontFaceCache &fonts,
                               UniquePtr<gfx::Renderer2D> &&contentsRenderer) noexcept
      : _device(device), _fonts(fonts), _contentsRenderer(krys::move(contentsRenderer)), _texturePool(device)
  {
  }

  TextureMapper::~TextureMapper() noexcept = default;

  void TextureMapper::BeginPainting(gfx::Renderer2D &renderer, const RectI &bounds) noexcept
  {
    krys_debug_assert(!_renderer);
    _renderer = &renderer;
    _baseTransform = renderer.GetTransform();
    _clipBounds = {bounds};
    _groupLayerCounts.clear();
    _texturePool.ReleaseUnusedTextures();
  }

  void TextureMapper::EndPainting() noexcept
  {
    krys_debug_assert(_renderer);
    krys_debug_assert(_clipBounds.size() == 1u);
    krys_debug_assert(_groupLayerCounts.empty());
    _renderer = nullptr;
    _clipBounds.clear();
  }

  gfx::TextureHandle TextureMapper::RenderImageBuffer(ImageBufferRender2DBackend &backend) noexcept
  {
    krys_debug_assert(_renderer);
    auto *commands = _renderer ? _renderer->GetCommandBuffer() : nullptr;
    if (!commands || !_imageBuffers)
    {
      return {};
    }
    return _imageBuffers->Render(backend, *commands);
  }

  void TextureMapper::SetTransform(const TransformationMatrix &matrix) noexcept
  {
    _renderer->SetTransform(_baseTransform * matrix.ToAffineTransform());
  }

  void TextureMapper::DrawBorder(const web::Color &color, float width, const RectF &targetRect,
                                 const TransformationMatrix &modelViewMatrix) noexcept
  {
    if (ClipBounds().IsEmpty())
    {
      return;
    }

    web::Path path;
    path.AddRect(targetRect);
    _renderer->Save();
    SetTransform(modelViewMatrix);
    _renderer->StrokePath(ToPath2D(path), {.Width = width}, gfx::Paint2D(ToColour(color)));
    _renderer->Restore();
  }

  namespace
  {
    // The segments lit for the digits 0 to 9: bit 0 is the top one, then clockwise (right top, right bottom,
    // bottom, left bottom, left top), and bit 6 is the middle one.
    constexpr Array<uint8, 10> DigitSegments {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
  }

  void TextureMapper::DrawNumber(int32 number, const web::Color &color, const PointF &targetPoint,
                                 const TransformationMatrix &modelViewMatrix) noexcept
  {
    if (ClipBounds().IsEmpty())
    {
      return;
    }

    // White seven-segment digits on a box of the colour (black when the layer shows no border), so the
    // compositor needs no font.
    constexpr float DigitWidth = 5;
    constexpr float DigitHeight = 9;
    constexpr float Stroke = 1;
    constexpr float Gap = 2;
    constexpr float Padding = 2;
    constexpr float Half = (DigitHeight - Stroke) / 2;

    auto digits = std::to_string(std::max(number, 0));
    auto count = static_cast<float>(digits.size());
    float width = 2 * Padding + count * DigitWidth + (count - 1) * Gap;
    float height = 2 * Padding + DigitHeight;

    _renderer->Save();
    SetTransform(modelViewMatrix);
    auto boxColor = color.IsVisible() ? color : web::Color(web::Color::black);
    _renderer->FillRect({targetPoint.X(), targetPoint.Y(), width, height}, gfx::Paint2D(ToColour(boxColor)));

    gfx::Paint2D white(SRGBA<uint8> {255, 255, 255, 255});
    float x = targetPoint.X() + Padding;
    float y = targetPoint.Y() + Padding;
    for (char digit : digits)
    {
      const Array<RectF, 7> segments {{
        {x, y, DigitWidth, Stroke},
        {x + DigitWidth - Stroke, y, Stroke, Half + Stroke},
        {x + DigitWidth - Stroke, y + Half, Stroke, Half + Stroke},
        {x, y + DigitHeight - Stroke, DigitWidth, Stroke},
        {x, y + Half, Stroke, Half + Stroke},
        {x, y, Stroke, Half + Stroke},
        {x, y + Half, DigitWidth, Stroke},
      }};
      auto lit = DigitSegments[static_cast<size_t>(digit - '0')];
      for (size_t segment = 0; segment < segments.size(); ++segment)
      {
        if (lit & (1u << segment))
        {
          _renderer->FillRect(segments[segment], white);
        }
      }
      x += DigitWidth + Gap;
    }
    _renderer->Restore();
  }

  namespace
  {
    KRYS_NODISCARD constexpr gfx::SamplerFilter
      ToSamplerFilter(web::GraphicsLayerScalingFilter filter) noexcept
    {
      // Render2D has no mipmaps, so trilinear filtering is linear.
      return filter == web::GraphicsLayerScalingFilter::Nearest ? gfx::SamplerFilter::Nearest
                                                                : gfx::SamplerFilter::Linear;
    }
  }

  gfx::SamplerFilter TextureMapper::ScalingFilter(const SizeI &textureSize, const RectF &targetRect,
                                                  const TransformationMatrix &modelViewMatrix) const noexcept
  {
    if (_minificationFilter == _magnificationFilter)
    {
      return ToSamplerFilter(_magnificationFilter);
    }
    auto shown = modelViewMatrix.MapRect(targetRect);
    bool magnified = shown.Width() >= static_cast<float>(textureSize.Width())
                     && shown.Height() >= static_cast<float>(textureSize.Height());
    return ToSamplerFilter(magnified ? _magnificationFilter : _minificationFilter);
  }

  void TextureMapper::DrawTexture(const BitmapTexture &texture, const RectF &targetRect,
                                  const TransformationMatrix &modelViewMatrix, float opacity,
                                  AllEdgesExposed) noexcept
  {
    if (!texture.Handle().IsValid() || ClipBounds().IsEmpty())
    {
      return;
    }

    // FIXME: Repeating textures (WrapMode::Repeat, the pattern transform), once contents layers are tiled.
    _renderer->Save();
    SetTransform(modelViewMatrix);
    _renderer->DrawImage(
      texture.Handle(), targetRect,
      {.Filter = ScalingFilter(texture.Size(), targetRect, modelViewMatrix), .Opacity = opacity});
    _renderer->Restore();
  }

  void TextureMapper::DrawTexture(gfx::TextureHandle texture, const SizeI &textureSize,
                                  const RectF &targetRect, const TransformationMatrix &modelViewMatrix,
                                  float opacity, AlphaPremultiplication alpha) noexcept
  {
    if (!texture.IsValid() || ClipBounds().IsEmpty())
    {
      return;
    }

    _renderer->Save();
    SetTransform(modelViewMatrix);
    _renderer->DrawImage(texture, targetRect,
                         {.Filter = ScalingFilter(textureSize, targetRect, modelViewMatrix),
                          .Alpha = alpha,
                          .Opacity = opacity});
    _renderer->Restore();
  }

  void TextureMapper::DrawSolidColor(const RectF &rect, const TransformationMatrix &matrix,
                                     const web::Color &color, bool) noexcept
  {
    if (ClipBounds().IsEmpty())
    {
      return;
    }

    _renderer->Save();
    SetTransform(matrix);
    _renderer->FillRect(rect, gfx::Paint2D(ToColour(color)));
    _renderer->Restore();
  }

  void TextureMapper::BeginClip(const TransformationMatrix &modelViewMatrix,
                                const web::FloatRoundedRect &targetRect) noexcept
  {
    _renderer->Save();
    SetTransform(modelViewMatrix);
    if (!targetRect.IsRounded())
    {
      // A rectilinear clip is a scissor around the pixels it touches, so a clip at a fractional position
      // doesn't fade the edge it cuts.
      auto quad = modelViewMatrix.ProjectQuad(QuadF(targetRect.Rect()));
      auto rect = quad.EnclosingBoundingBox();
      if (modelViewMatrix.IsAffine() && quad.IsRectilinear() && !rect.IsEmpty())
      {
        SetTransform(TransformationMatrix());
        _renderer->ClipRect(RectF(rect));
        SetTransform(modelViewMatrix);
      }
      else
      {
        _renderer->ClipRect(targetRect.Rect());
      }
    }
    else if (auto circular = ToCircularRoundedRect(targetRect))
    {
      _renderer->ClipRoundedRect(*circular);
    }
    else
    {
      web::Path path;
      path.AddRoundedRect(targetRect);
      _renderer->ClipPath(ToPath2D(path), gfx::FillRule::NonZero);
    }

    auto bounds = EnclosingRectI(modelViewMatrix.MapQuad(QuadF(targetRect.Rect())).BoundingBox());
    bounds.Intersect(ClipBounds());
    _clipBounds.push_back(bounds);
  }

  void TextureMapper::EndClip() noexcept
  {
    krys_debug_assert(_clipBounds.size() > 1u);
    _clipBounds.pop_back();
    _renderer->Restore();
  }

  RectI TextureMapper::ClipBounds() const noexcept
  {
    return _clipBounds.empty() ? RectI() : _clipBounds.back();
  }

  void TextureMapper::SetMaskMode(bool maskMode) noexcept
  {
    _renderer->SetCompositeOperator(maskMode ? gfx::CompositeOperator2D::DestinationIn
                                             : gfx::CompositeOperator2D::SourceOver);
  }

  void TextureMapper::BeginGroup(float opacity, BlendMode blendMode, const web::FilterOperations &filters,
                                 gfx::EdgeMode2D blurEdgeMode) noexcept
  {
    // The group is composited at its opacity after its filters: its layer is outermost, then a layer per
    // filter, the last outermost.
    _renderer->BeginLayer(opacity, blendMode);
    uint32 layers = 1u;
    // The filters' lengths are the layer's, which the transform scales (by the device and page scales).
    float scale = std::sqrt(std::abs(static_cast<float>(_renderer->GetTransform().Determinant())));
    for (auto it = filters.rbegin(); it != filters.rend(); ++it)
    {
      if (auto filter = LayerFilterFor(**it, blurEdgeMode, scale))
      {
        _renderer->BeginLayer(1.f, BlendMode::Normal, *filter);
        ++layers;
      }
    }
    _groupLayerCounts.push_back(layers);
  }

  void TextureMapper::EndGroup() noexcept
  {
    krys_debug_assert(!_groupLayerCounts.empty());
    for (uint32 i = 0; i < _groupLayerCounts.back(); ++i)
    {
      _renderer->EndLayer();
    }
    _groupLayerCounts.pop_back();
  }

  SizeI TextureMapper::MaxTextureSize() const noexcept
  {
    auto size = static_cast<int32>(_device.Limits().MaxTextureSize);
    return {size, size};
  }

  Ref<BitmapTexture> TextureMapper::AcquireTextureFromPool(const SizeI &size,
                                                           EnumSet<BitmapTexture::Flags> flags) noexcept
  {
    return _texturePool.AcquireTexture(size, flags);
  }

  template <typename TPaint>
  void TextureMapper::PaintIntoTexture(BitmapTexture &texture, const RectI &targetRect, TPaint paint) noexcept
  {
    auto *commands = _renderer ? _renderer->GetCommandBuffer() : nullptr;
    if (!commands || !texture.Handle().IsValid() || targetRect.IsEmpty())
    {
      return;
    }

    // Part of a texture is painted into a surface of its own, cleared, and copied over that part: what was
    // there is replaced, as drawing over it would blend with it.
    const bool wholeTexture = targetRect == RectI({}, texture.Size());
    RefPtr<BitmapTexture> surface;
    if (!wholeTexture)
    {
      surface = AcquireTextureFromPool(targetRect.Size(), {BitmapTexture::Flags::SupportsAlpha});
      if (!surface->Handle().IsValid())
      {
        return;
      }
    }
    auto &target = wholeTexture ? texture : *surface;

    const auto size = targetRect.Size();
    web::display_list::RecorderImpl recorder(
      SizeF(static_cast<float>(size.Width()), static_cast<float>(size.Height())));
    paint(recorder);
    auto displayList = recorder.TakeDisplayList();

    if (!_imageBuffers)
    {
      _imageBuffers = CreateUnique<ImageBufferTextures>(_device, _fonts);
    }
    _contentsRenderer->Begin(*commands, {.Texture = target.Handle(),
                                         .Width = static_cast<uint32>(size.Width()),
                                         .Height = static_cast<uint32>(size.Height()),
                                         .Format = gfx::TextureFormat::RGBA8Unorm,
                                         .ClearColour = Array<float, 4> {0.f, 0.f, 0.f, 0.f}});
    {
      GraphicsContextRender2D context(
        *_contentsRenderer, _fonts,
        RectF(0, 0, static_cast<float>(size.Width()), static_cast<float>(size.Height())),
        _imageBuffers.get());
      context.DrawDisplayList(*displayList);
    }
    _contentsRenderer->End();
    _imageBuffers->Release();

    if (!wholeTexture)
    {
      commands->BlitTexture(surface->Handle(),
                            {.X = 0u,
                             .Y = 0u,
                             .Width = static_cast<uint32>(size.Width()),
                             .Height = static_cast<uint32>(size.Height())},
                            texture.Handle(),
                            {.X = static_cast<uint32>(targetRect.X()),
                             .Y = static_cast<uint32>(targetRect.Y()),
                             .Width = static_cast<uint32>(size.Width()),
                             .Height = static_cast<uint32>(size.Height())},
                            gfx::SamplerFilter::Nearest);
    }
  }

  void TextureMapper::UpdateContents(BitmapTexture &texture, const web::GraphicsLayer &sourceLayer,
                                     const RectI &targetRect, const PointI &offset, float scale) noexcept
  {
    PaintIntoTexture(texture, targetRect,
                     [&](web::GraphicsContext &context)
                     {
                       context.SetTextDrawingMode(web::TextDrawingMode::Fill);

                       RectI sourceRect(targetRect);
                       sourceRect.SetLocation(offset);
                       sourceRect.Scale(1 / scale);
                       context.ApplyDeviceScaleFactor(scale);
                       context.Translate(static_cast<float>(-sourceRect.X()),
                                         static_cast<float>(-sourceRect.Y()));

                       sourceLayer.PaintGraphicsLayerContents(context, RectF(sourceRect));
                     });
  }

  void TextureMapper::UpdateContents(BitmapTexture &texture, web::Image &image, const RectI &targetRect,
                                     const PointI &sourceOffset) noexcept
  {
    PaintIntoTexture(texture, targetRect,
                     [&](web::GraphicsContext &context)
                     {
                       auto size = SizeF(targetRect.Size());
                       context.DrawImage(image, RectF({}, size), RectF(PointF(sourceOffset), size));
                     });
  }
}
