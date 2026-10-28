#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/Canvas/GPUBasedCanvasRenderingContext.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/Rendering/RenderHTMLCanvas.hpp"

namespace krys::boo::html
{
  HTMLCanvasElement::HTMLCanvasElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Canvas),
        CanvasBase(IntSize(static_cast<int32>(DefaultWidth), static_cast<int32>(DefaultHeight)))
  {
  }

  HTMLCanvasElement::~HTMLCanvasElement() noexcept = default;

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

  RefPtr<CanvasRenderingContext> HTMLCanvasElement::GetContext(dom::DOMStringView) noexcept
  {
    return nullptr;
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
    IntSize oldSize = Size();
    IntSize newSize(static_cast<int32>(Width()), static_cast<int32>(Height()));

    SetSize(newSize);

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
    }
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

  void HTMLCanvasElement::Paint(GraphicsContext &context, const LayoutRect &) noexcept
  {
    if (!_context || context.PaintingDisabled())
    {
      return;
    }

    // A composited context's pixels are the layer's contents, unless they're printed or snapshotted.
    if (UsesContentsAsLayerContents() && !NodeDocument().Printing() && !_isSnapshotting)
    {
      return;
    }

    // TODO(HTMLCANVAS, HTML): Draw the context's display buffer, once a context has one (B13c, B14).
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
}
