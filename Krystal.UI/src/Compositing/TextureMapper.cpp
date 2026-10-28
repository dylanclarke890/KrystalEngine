#include "Krystal.UI/Compositing/TextureMapper.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperations.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayer.hpp"
#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Core/Colour/ColourMatrix.hpp"
#include "Krystal.UI/GraphicsContextRender2D.hpp"
#include "Krystal.UI/ImageBufferTextures.hpp"
#include "Krystal.UI/Render2DConversions.hpp"

namespace krys::ui
{
  namespace
  {
    KRYS_NODISCARD gfx::Transform2D ToTransform2D(const boo::TransformationMatrix &matrix) noexcept
    {
      // FIXME: Perspective, once there are 3D rendering contexts: a layer's 3D transform is flattened.
      return ui::ToTransform2D(matrix.ToAffineTransform());
    }

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

    /// @brief The layer filter `operation` is, if Render2D can apply it.
    KRYS_NODISCARD Maybe<gfx::LayerFilter2D> LayerFilterFor(const boo::FilterOperation &operation,
                                                            gfx::EdgeMode2D blurEdgeMode) noexcept
    {
      using Type = boo::FilterOperation::Type;
      switch (operation.GetType())
      {
        case Type::Grayscale:
        case Type::Sepia:
        case Type::Saturate:
        case Type::HueRotate:
        {
          auto amount =
            static_cast<float>(Downcast<boo::BasicColorMatrixFilterOperation>(operation).Amount());
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
            static_cast<float>(Downcast<boo::BasicComponentTransferFilterOperation>(operation).Amount());
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
          return gfx::LayerFilter2D::Blur(Downcast<boo::BlurFilterOperation>(operation).StdDeviation() * 2.f,
                                          blurEdgeMode);
        }
        case Type::DropShadow:
        {
          const auto &shadow = Downcast<boo::DropShadowFilterOperation>(operation);
          return gfx::LayerFilter2D::DropShadow(
            {.Offset = {static_cast<float>(shadow.X()), static_cast<float>(shadow.Y())},
             .BlurRadius = static_cast<float>(shadow.StdDeviation()) * 2.f,
             .Colour = ToColour(shadow.GetColor())});
        }
        default:
          // FIXME: Reference filters, once there are SVG filters; the rest are identities.
          return null;
      }
    }
  }

  Expected<UniquePtr<TextureMapper>> TextureMapper::Create(gfx::Device &device, FontFaceCache &fonts) noexcept
  {
    auto contentsRenderer = gfx::Renderer2D::Create(device);
    if (!contentsRenderer)
    {
      return Unexpected(contentsRenderer.error());
    }
    return UniquePtr<TextureMapper>(new TextureMapper(device, fonts, krys::move(*contentsRenderer)));
  }

  TextureMapper::TextureMapper(gfx::Device &device, FontFaceCache &fonts,
                               UniquePtr<gfx::Renderer2D> &&contentsRenderer) noexcept
      : _device(device), _fonts(fonts), _contentsRenderer(krys::move(contentsRenderer)), _texturePool(device)
  {
  }

  TextureMapper::~TextureMapper() noexcept = default;

  void TextureMapper::BeginPainting(gfx::Renderer2D &renderer, const boo::IntRect &bounds) noexcept
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

  void TextureMapper::SetTransform(const boo::TransformationMatrix &matrix) noexcept
  {
    _renderer->SetTransform(_baseTransform * ToTransform2D(matrix));
  }

  void TextureMapper::DrawBorder(const boo::Color &color, float width, const boo::FloatRect &targetRect,
                                 const boo::TransformationMatrix &modelViewMatrix) noexcept
  {
    if (ClipBounds().IsEmpty())
    {
      return;
    }

    boo::Path path;
    path.AddRect(targetRect);
    _renderer->Save();
    SetTransform(modelViewMatrix);
    _renderer->StrokePath(ToPath2D(path), {.Width = width}, gfx::Paint2D(ToColour(color)));
    _renderer->Restore();
  }

  void TextureMapper::DrawNumber(int32, const boo::Color &, const boo::FloatPoint &,
                                 const boo::TransformationMatrix &) noexcept
  {
    // FIXME: Repaint counters, which draw the count as text.
  }

  void TextureMapper::DrawTexture(const BitmapTexture &texture, const boo::FloatRect &targetRect,
                                  const boo::TransformationMatrix &modelViewMatrix, float opacity,
                                  AllEdgesExposed) noexcept
  {
    if (!texture.Handle().IsValid() || ClipBounds().IsEmpty())
    {
      return;
    }

    // FIXME: Repeating textures (WrapMode::Repeat, the pattern transform), once contents layers are tiled.
    _renderer->Save();
    SetTransform(modelViewMatrix);
    _renderer->DrawImage(texture.Handle(), ToRect2D(targetRect), {.Opacity = opacity});
    _renderer->Restore();
  }

  void TextureMapper::DrawTexture(gfx::TextureHandle texture, const boo::FloatRect &targetRect,
                                  const boo::TransformationMatrix &modelViewMatrix, float opacity,
                                  AlphaPremultiplication alpha) noexcept
  {
    if (!texture.IsValid() || ClipBounds().IsEmpty())
    {
      return;
    }

    _renderer->Save();
    SetTransform(modelViewMatrix);
    _renderer->DrawImage(texture, ToRect2D(targetRect), {.Alpha = alpha, .Opacity = opacity});
    _renderer->Restore();
  }

  void TextureMapper::DrawSolidColor(const boo::FloatRect &rect, const boo::TransformationMatrix &matrix,
                                     const boo::Color &color, bool) noexcept
  {
    if (ClipBounds().IsEmpty())
    {
      return;
    }

    _renderer->Save();
    SetTransform(matrix);
    _renderer->FillRect(ToRect2D(rect), gfx::Paint2D(ToColour(color)));
    _renderer->Restore();
  }

  void TextureMapper::BeginClip(const boo::TransformationMatrix &modelViewMatrix,
                                const boo::FloatRoundedRect &targetRect) noexcept
  {
    _renderer->Save();
    SetTransform(modelViewMatrix);
    if (!targetRect.IsRounded())
    {
      // A rectilinear clip is a scissor around the pixels it touches, so a clip at a fractional position
      // doesn't fade the edge it cuts.
      auto quad = modelViewMatrix.ProjectQuad(boo::FloatQuad(targetRect.Rect()));
      auto rect = quad.EnclosingBoundingBox();
      if (modelViewMatrix.IsAffine() && quad.IsRectilinear() && !rect.IsEmpty())
      {
        SetTransform(boo::TransformationMatrix());
        _renderer->ClipRect(ToRect2D(boo::FloatRect(rect)));
        SetTransform(modelViewMatrix);
      }
      else
      {
        _renderer->ClipRect(ToRect2D(targetRect.Rect()));
      }
    }
    else if (auto circular = ToCircularRoundedRect(targetRect))
    {
      _renderer->ClipRoundedRect(*circular);
    }
    else
    {
      boo::Path path;
      path.AddRoundedRect(targetRect);
      _renderer->ClipPath(ToPath2D(path), gfx::FillRule::NonZero);
    }

    auto bounds =
      boo::EnclosingIntRect(modelViewMatrix.MapQuad(boo::FloatQuad(targetRect.Rect())).BoundingBox());
    bounds.Intersect(ClipBounds());
    _clipBounds.push_back(bounds);
  }

  void TextureMapper::EndClip() noexcept
  {
    krys_debug_assert(_clipBounds.size() > 1u);
    _clipBounds.pop_back();
    _renderer->Restore();
  }

  boo::IntRect TextureMapper::ClipBounds() const noexcept
  {
    return _clipBounds.empty() ? boo::IntRect() : _clipBounds.back();
  }

  void TextureMapper::SetMaskMode(bool maskMode) noexcept
  {
    _renderer->SetCompositeOperator(maskMode ? gfx::CompositeOperator2D::DestinationIn
                                             : gfx::CompositeOperator2D::SourceOver);
  }

  void TextureMapper::BeginGroup(float opacity, BlendMode blendMode, const boo::FilterOperations &filters,
                                 gfx::EdgeMode2D blurEdgeMode) noexcept
  {
    // The group is composited at its opacity after its filters: its layer is outermost, then a layer per
    // filter, the last outermost.
    _renderer->BeginLayer(opacity, blendMode);
    uint32 layers = 1u;
    for (auto it = filters.rbegin(); it != filters.rend(); ++it)
    {
      if (auto filter = LayerFilterFor(**it, blurEdgeMode))
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

  boo::IntSize TextureMapper::MaxTextureSize() const noexcept
  {
    auto size = static_cast<int32>(_device.Limits().MaxTextureSize);
    return {size, size};
  }

  Ref<BitmapTexture> TextureMapper::AcquireTextureFromPool(const boo::IntSize &size,
                                                           EnumSet<BitmapTexture::Flags> flags) noexcept
  {
    return _texturePool.AcquireTexture(size, flags);
  }

  template <typename TPaint>
  void TextureMapper::PaintIntoTexture(BitmapTexture &texture, const boo::IntRect &targetRect,
                                       TPaint &&paint) noexcept
  {
    auto *commands = _renderer ? _renderer->GetCommandBuffer() : nullptr;
    if (!commands || !texture.Handle().IsValid() || targetRect.IsEmpty())
    {
      return;
    }

    // Part of a texture is painted into a surface of its own, cleared, and copied over that part: what was
    // there is replaced, as drawing over it would blend with it.
    const bool wholeTexture = targetRect == boo::IntRect({}, texture.Size());
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
    boo::display_list::RecorderImpl recorder(
      boo::FloatSize(static_cast<float>(size.Width()), static_cast<float>(size.Height())));
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
        boo::FloatRect(0, 0, static_cast<float>(size.Width()), static_cast<float>(size.Height())),
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

  void TextureMapper::UpdateContents(BitmapTexture &texture, const boo::GraphicsLayer &sourceLayer,
                                     const boo::IntRect &targetRect, const boo::IntPoint &offset,
                                     float scale) noexcept
  {
    PaintIntoTexture(texture, targetRect,
                     [&](boo::GraphicsContext &context)
                     {
                       context.SetTextDrawingMode(boo::TextDrawingMode::Fill);

                       boo::IntRect sourceRect(targetRect);
                       sourceRect.SetLocation(offset);
                       sourceRect.Scale(1 / scale);
                       context.ApplyDeviceScaleFactor(scale);
                       context.Translate(static_cast<float>(-sourceRect.X()),
                                         static_cast<float>(-sourceRect.Y()));

                       sourceLayer.PaintGraphicsLayerContents(context, boo::FloatRect(sourceRect));
                     });
  }

  void TextureMapper::UpdateContents(BitmapTexture &texture, boo::Image &image,
                                     const boo::IntRect &targetRect,
                                     const boo::IntPoint &sourceOffset) noexcept
  {
    PaintIntoTexture(texture, targetRect,
                     [&](boo::GraphicsContext &context)
                     {
                       auto size = boo::FloatSize(targetRect.Size());
                       context.DrawImage(image, boo::FloatRect({}, size),
                                         boo::FloatRect(boo::FloatPoint(sourceOffset), size));
                     });
  }
}
