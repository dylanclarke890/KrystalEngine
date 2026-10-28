#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/FileAPI/Blob.hpp"
#include "Krystal.Booey/Graphics/BitmapImage.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2D.hpp"
#include "Krystal.Booey/HTML/Canvas/EngineTextureRenderingContext.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/Platform/MIMETypeRegistry.hpp"
#include "Krystal.Booey/Rendering/RenderHTMLCanvas.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::html
{
  HTMLCanvasElement::HTMLCanvasElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Canvas),
        CanvasBase(IntSize(static_cast<int32>(DefaultWidth), static_cast<int32>(DefaultHeight)))
  {
  }

  HTMLCanvasElement::~HTMLCanvasElement() noexcept
  {
    _context = nullptr; // Ensure this goes away before the ImageBuffer.
    std::ignore = SetImageBuffer(nullptr);
  }

#pragma region HTMLCanvasElement

  uint32 HTMLCanvasElement::Width() const noexcept
  {
    return Reflection::Reflect<uint32, ReflectDefault<uint32> {DefaultWidth}>(*this, u8"width");
  }

  void HTMLCanvasElement::Width(uint32 value) noexcept
  {
    Reflection::Reflect<uint32, ReflectDefault<uint32> {DefaultWidth}>(*this, u8"width", value);
  }

  uint32 HTMLCanvasElement::Height() const noexcept
  {
    return Reflection::Reflect<uint32, ReflectDefault<uint32> {DefaultHeight}>(*this, u8"height");
  }

  void HTMLCanvasElement::Height(uint32 value) noexcept
  {
    Reflection::Reflect<uint32, ReflectDefault<uint32> {DefaultHeight}>(*this, u8"height", value);
  }

  RefPtr<CanvasRenderingContext> HTMLCanvasElement::GetContext(dom::DOMStringView contextId) noexcept
  {
    if (Is2dType(contextId))
    {
      return GetContext2d();
    }
    if (contextId == EngineTextureRenderingContext::ContextType)
    {
      if (_context)
      {
        return ShareRefPtr(DynamicDowncast<EngineTextureRenderingContext>(_context.get()));
      }
      return ShareRefPtr(CreateContextEngineTexture());
    }
    return nullptr;
  }

  bool HTMLCanvasElement::Is2dType(dom::DOMStringView type) noexcept
  {
    return type == u8"2d";
  }

  RefPtr<CanvasRenderingContext2D>
    HTMLCanvasElement::GetContext2d(CanvasRenderingContext2DSettings &&settings) noexcept
  {
    if (!_context)
    {
      return ShareRefPtr(CreateContext2d(krys::move(settings)));
    }
    return ShareRefPtr(DynamicDowncast<CanvasRenderingContext2D>(_context.get()));
  }

  CanvasRenderingContext2D *
    HTMLCanvasElement::CreateContext2d(CanvasRenderingContext2DSettings &&settings) noexcept
  {
    krys_debug_assert(!_context);

    auto context =
      CanvasRenderingContext2D::Create(*this, krys::move(settings), NodeDocument().InQuirksMode());
    auto *context2d = context.get();
    _context = krys::move(context);

    // Need to make sure a RenderLayer and compositing layer get created for the Canvas.
    InvalidateStyleAndLayerComposition();
    return context2d;
  }

  EngineTextureRenderingContext *HTMLCanvasElement::CreateContextEngineTexture() noexcept
  {
    krys_debug_assert(!_context);
    auto context = CreateUnique<EngineTextureRenderingContext>(*this);
    auto *engineTextureContext = context.get();
    _context = krys::move(context);

    // Need to make sure a RenderLayer and compositing layer get created for the Canvas.
    InvalidateStyleAndLayerComposition();
    if (auto *box = GetRenderBox())
    {
      box->ContentChanged(ContentChangeType::Canvas);
    }
    return engineTextureContext;
  }

#pragma endregion

#pragma region Drawing

  Maybe<FloatRect>
    HTMLCanvasElement::ComputeDirtyRectangleIfNeeded(const Maybe<FloatRect> &rect) const noexcept
  {
    if (!rect)
    {
      return null;
    }

    if (UsesContentsAsLayerContents())
    {
      return null;
    }

    FloatRect destRect;
    auto *renderer = GetRenderBox();
    if (auto *renderReplaced = DynamicDowncast<RenderReplaced>(*renderer))
    {
      destRect = FloatRect(renderReplaced->ReplacedContentRect());
    }
    else
    {
      destRect = FloatRect(renderer->ContentBoxRect());
    }

    FloatRect dirtyRect = MapRect(*rect, FloatRect {{}, FloatSize(Size())}, destRect);
    dirtyRect.Intersect(destRect);
    if (dirtyRect.IsEmpty())
    {
      return null;
    }
    return dirtyRect;
  }

  void HTMLCanvasElement::DidDraw(const Maybe<FloatRect> &rect) noexcept
  {
    ClearCopiedImage();
    if (auto *renderer = GetRenderBox())
    {
      auto dirtyRect = ComputeDirtyRectangleIfNeeded(rect);
      if (UsesContentsAsLayerContents())
      {
        renderer->ContentChanged(ContentChangeType::CanvasPixels, dirtyRect);
      }
      else if (dirtyRect)
      {
        renderer->RepaintRectangle(LayoutRect(EnclosingIntRect(*dirtyRect)));
      }
    }

    CanvasBase::DidDraw(rect);
  }

  void HTMLCanvasElement::DidUpdateSizeProperties() noexcept
  {
    const bool hadImageBuffer = HasCreatedImageBuffer();

    if (auto *context = DynamicDowncast<CanvasRenderingContext2D>(_context.get()))
    {
      context->Reset();
    }
    else
    {
      ResetGraphicsContextState();
    }

    IntSize oldSize = Size();
    IntSize newSize(static_cast<int32>(Width()), static_cast<int32>(Height()));
    // If the size of an existing buffer matches, we can just clear it instead of reallocating.
    // This optimization is only done for 2D canvases for now.
    if (HasCreatedImageBuffer() && oldSize == newSize && _context && _context->Is2d() && Buffer())
    {
      if (!_didClearImageBuffer)
      {
        ClearImageBuffer();
      }
      return;
    }

    SetSize(newSize);
    SetHasCreatedImageBuffer(false);
    std::ignore = SetImageBuffer(nullptr);
    ClearCopiedImage();

    if (auto *context = DynamicDowncast<GPUBasedCanvasRenderingContext>(_context.get()))
    {
      context->Reshape();
    }

    if (auto *canvasRenderer = DynamicDowncast<RenderHTMLCanvas>(Renderer()))
    {
      if (oldSize != Size())
      {
        canvasRenderer->CanvasSizeChanged();
        if (canvasRenderer->HasAcceleratedCompositing())
        {
          canvasRenderer->ContentChanged(ContentChangeType::Canvas);
        }
      }
      if (hadImageBuffer)
      {
        canvasRenderer->Repaint();
      }
    }
  }

  void HTMLCanvasElement::CreateImageBuffer() const noexcept
  {
    krys_debug_assert(!HasCreatedImageBuffer());

    SetHasCreatedImageBuffer(true);
    _didClearImageBuffer = true;
    std::ignore = SetImageBuffer(AllocateImageBuffer());

    if (_context && _context->Is2d())
    {
      // Recalculate compositing requirements if acceleration state changed.
      const_cast<HTMLCanvasElement *>(this)->InvalidateStyleAndLayerComposition();
      if (auto *renderer = GetRenderBox())
      {
        if (UsesContentsAsLayerContents())
        {
          renderer->ContentChanged(ContentChangeType::Canvas);
        }
      }
    }
  }

  bool HTMLCanvasElement::NeedsPreparationForDisplay() const noexcept
  {
    return _context && _context->NeedsPreparationForDisplay();
  }

  void HTMLCanvasElement::PrepareForDisplay() noexcept
  {
    krys_debug_assert(NeedsPreparationForDisplay());
    if (_context)
    {
      _context->PrepareForDisplay();
    }
  }

  Image *HTMLCanvasElement::CopiedImage() const noexcept
  {
    if (!_copiedImage)
    {
      auto buffer = const_cast<HTMLCanvasElement *>(this)->MakeRenderingResultsAvailable();
      if (buffer)
      {
        _copiedImage = BitmapImage::Create(buffer->CopyNativeImage());
      }
    }
    return _copiedImage.get();
  }

  void HTMLCanvasElement::ClearCopiedImage() const noexcept
  {
    _copiedImage = nullptr;
    _didClearImageBuffer = false;
  }

  void HTMLCanvasElement::ClearImageBuffer() const noexcept
  {
    krys_debug_assert(HasCreatedImageBuffer());
    krys_debug_assert(!_didClearImageBuffer);
    krys_debug_assert(_context);

    _didClearImageBuffer = true;

    if (auto *canvas = DynamicDowncast<CanvasRenderingContext2D>(_context.get()))
    {
      // No need to undo transforms/clip/etc. because we are called right after the context is reset.
      canvas->ClearRect(0, 0, Width(), Height());
    }
  }

  const css::ParserContext &HTMLCanvasElement::CssParserContext() const noexcept
  {
    return NodeDocument().CssParserContext();
  }

  dom::Document *HTMLCanvasElement::CanvasDocument() const noexcept
  {
    return &NodeDocument();
  }

  bool HTMLCanvasElement::UsesContentsAsLayerContents() const noexcept
  {
    auto *renderBox = GetRenderBox();
    if (!renderBox)
    {
      return false;
    }
    if (!_context)
    {
      return false;
    }
    return renderBox->HasAcceleratedCompositing() && _context->DelegatesDisplay();
  }

  void HTMLCanvasElement::Paint(GraphicsContext &context, const LayoutRect &rect) noexcept
  {
    if (!_context)
    {
      return;
    }
    _context->ClearAccumulatedDirtyRect();

    if (context.PaintingDisabled())
    {
      return;
    }

    // A composited context's pixels are the layer's contents, unless they're printed or snapshotted.
    if (UsesContentsAsLayerContents() && !NodeDocument().Printing() && !_isSnapshotting)
    {
      return;
    }

    if (_context->CompositingResultsNeedUpdating())
    {
      _context->PrepareForDisplay();
    }

    if (_context->IsSurfaceBufferTransparentBlack(CanvasRenderingContext::SurfaceBuffer::DisplayBuffer))
    {
      const bool skipTransparentBlackDraw =
        context.GetCompositeMode() == CompositeMode {CompositeOperator::SourceOver, BlendMode::Normal};
      if (!skipTransparentBlackDraw)
      {
        context.FillRect(FloatRect(SnappedIntRect(rect)), Color::transparentBlack);
      }
      return;
    }

    if (_context->Is2d())
    {
      if (auto buffer =
            _context->SurfaceBufferToImageBuffer(CanvasRenderingContext::SurfaceBuffer::DisplayBuffer))
      {
        context.DrawImageBuffer(*buffer, FloatRect(SnappedIntRect(rect)),
                                ImagePaintingOptions(context.CompositeOperation()));
      }
      return;
    }

    if (auto image =
          _context->SurfaceBufferToNativeImage(CanvasRenderingContext::SurfaceBuffer::DisplayBuffer))
    {
      context.DrawNativeImage(*image, FloatRect(SnappedIntRect(rect)),
                              FloatRect({}, FloatSize(image->Size())),
                              ImagePaintingOptions(context.CompositeOperation()));
    }
  }

#pragma endregion

#pragma region Element

  void HTMLCanvasElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                             Maybe<dom::DOMStringView> value,
                                             dom::DOMStringAtom namespaceURI) noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null() && (localName == u8"width" || localName == u8"height"))
    {
      DidUpdateSizeProperties();
    }
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
  }

  RenderPtr<RenderElement> HTMLCanvasElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderHTMLCanvas>(*this, krys::move(style));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLCanvasElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                             dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null() && (localName == u8"width" || localName == u8"height"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLCanvasElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                                 dom::DOMStringAtom namespaceURI,
                                                                 dom::DOMStringView value,
                                                                 css::MutableStyleProperties &style) noexcept
  {
    using namespace presentational_hints;
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"width")
    {
      ApplyAspectRatioWithoutDimensionalRulesFromWidthAndHeightAttributesToStyle(
        value, AttributeOrEmpty(u8"height"), style);
      return;
    }
    if (localName == u8"height")
    {
      ApplyAspectRatioWithoutDimensionalRulesFromWidthAndHeightAttributesToStyle(AttributeOrEmpty(u8"width"),
                                                                                 value, style);
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion

  KRYS_NODISCARD static dom::DOMString ToEncodingMimeType(dom::DOMStringView mimeType) noexcept
  {
    if (!MIMETypeRegistry::IsSupportedImageMIMETypeForEncoding(mimeType))
    {
      return u8"image/png";
    }
    return text::ToASCIILower(mimeType);
  }

  // https://html.spec.whatwg.org/multipage/canvas.html#a-serialisation-of-the-bitmap-as-a-file
  KRYS_NODISCARD static Maybe<double> QualityInRange(Maybe<double> quality) noexcept
  {
    if (!quality || *quality < 0 || *quality > 1)
    {
      return null;
    }
    return quality;
  }

  dom::DOMString HTMLCanvasElement::ToDataURL(dom::DOMStringView mimeType, Maybe<double> quality) noexcept
  {
    if (Size().IsEmpty())
    {
      return u8"data:,";
    }

    auto encodingMIMEType = ToEncodingMimeType(mimeType);
    auto buffer = MakeRenderingResultsAvailable();
    if (!buffer)
    {
      return u8"data:,";
    }
    return buffer->ToDataURL(encodingMIMEType, QualityInRange(quality));
  }

  void HTMLCanvasElement::ToBlob(Func<void(RefPtr<Blob> &&)> &&callback, dom::DOMStringView mimeType,
                                 Maybe<double> quality) noexcept
  {
    auto &document = NodeDocument();
    auto scheduleCallback = [&](RefPtr<Blob> &&blob)
    {
      EventLoop::Current().QueueTask(TaskSource::CanvasBlobSerialization, document,
                                     [callback = krys::move(callback), blob = krys::move(blob)]() mutable
                                     { callback(krys::move(blob)); });
    };

    if (Size().IsEmpty())
    {
      scheduleCallback(nullptr);
      return;
    }

    auto encodingMIMEType = ToEncodingMimeType(mimeType);
    auto buffer = MakeRenderingResultsAvailable();
    if (!buffer)
    {
      scheduleCallback(nullptr);
      return;
    }

    auto blobData = buffer->ToData(encodingMIMEType, QualityInRange(quality));
    RefPtr<Blob> blob;
    if (!blobData.empty())
    {
      blob = Blob::Create(krys::move(blobData), encodingMIMEType);
    }
    scheduleCallback(krys::move(blob));
  }
}
