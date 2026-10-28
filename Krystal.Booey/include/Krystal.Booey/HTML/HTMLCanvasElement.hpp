#pragma once

#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2DSettings.hpp"
#include "Krystal.Booey/HTML/CanvasBase.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class GraphicsContext;
}

namespace krys::boo::html
{
  class CanvasRenderingContext2D;
  class EngineTextureRenderingContext;

  /// @brief A `<canvas>`: a replaced element whose pixels a rendering context draws. Its children are
  /// fallback content, which isn't rendered.
  /// @see https://html.spec.whatwg.org/#htmlcanvaselement
  class HTMLCanvasElement : public HTMLElement, public CanvasBase
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLCanvasElement);

  private:
    bool _isSnapshotting {false};
    mutable bool _didClearImageBuffer {false};
    UniquePtr<CanvasRenderingContext> _context;

  public:
    // These values come from the HTML standard.
    static constexpr uint32 DefaultWidth = 300;
    static constexpr uint32 DefaultHeight = 150;

    HTMLCanvasElement(dom::Document &document) noexcept;
    ~HTMLCanvasElement() noexcept override;

    void AddRef() const noexcept final
    {
      HTMLElement::AddRef();
    }
    void SubRef() const noexcept final
    {
      HTMLElement::SubRef();
    }
    KRYS_NODISCARD uint32 GetRefCount() const noexcept final
    {
      return HTMLElement::GetRefCount();
    }

    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept final
    {
      return false;
    }
    KRYS_NODISCARD bool CanStartSelection() const noexcept final
    {
      return false;
    }

    /// @brief A canvas is always replaced: the host draws into it, as scripts would.
    KRYS_NODISCARD bool IsReplaced(const RenderStyle * = nullptr) const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD bool IsHTMLCanvasElement() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD CanvasRenderingContext *RenderingContext() const noexcept final
    {
      return _context.get();
    }

#pragma region HTMLCanvasElement - https://html.spec.whatwg.org/#htmlcanvaselement

    /// @see https://html.spec.whatwg.org/#dom-canvas-width
    KRYS_NODISCARD uint32 Width() const noexcept final;

    /// @see https://html.spec.whatwg.org/#dom-canvas-width
    void Width(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-canvas-height
    KRYS_NODISCARD uint32 Height() const noexcept final;

    /// @see https://html.spec.whatwg.org/#dom-canvas-height
    void Height(uint32 value) noexcept;

    /// @brief The canvas's context of type @p contextId, made on the first call; null if the canvas already
    /// has a context of another type, or there's no such type.
    /// @see https://html.spec.whatwg.org/#dom-canvas-getcontext
    /// @note The engine texture context is `"krystal-texture"`.
    KRYS_NODISCARD RefPtr<CanvasRenderingContext> GetContext(dom::DOMStringView contextId) noexcept;

    /// @brief The canvas's 2D context, made with @p settings on the first call; null if the canvas has a
    /// context of another type.
    /// @see https://html.spec.whatwg.org/#dom-canvas-getcontext
    KRYS_NODISCARD RefPtr<CanvasRenderingContext2D>
      GetContext2d(CanvasRenderingContext2DSettings &&settings = {}) noexcept;

    KRYS_NODISCARD static bool Is2dType(dom::DOMStringView type) noexcept;

    // TODO(HTMLCANVAS, HTML): toDataURL() and toBlob() (B14e); transferControlToOffscreen() isn't planned.

#pragma endregion

    void DidDraw(const Maybe<FloatRect> &rect) noexcept final;

    KRYS_NODISCARD const css::ParserContext &CssParserContext() const noexcept final;

    /// @brief Paints the context's pixels into @p rect, when they aren't shown as the layer's contents.
    void Paint(GraphicsContext &context, const LayoutRect &rect) noexcept;

    void SetIsSnapshotting(bool isSnapshotting) noexcept
    {
      _isSnapshotting = isSnapshotting;
    }
    KRYS_NODISCARD bool IsSnapshotting() const noexcept
    {
      return _isSnapshotting;
    }

  protected:
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept override;

  private:
    KRYS_NODISCARD CanvasRenderingContext2D *
      CreateContext2d(CanvasRenderingContext2DSettings &&settings) noexcept;
    KRYS_NODISCARD EngineTextureRenderingContext *CreateContextEngineTexture() noexcept;

    void CreateImageBuffer() const noexcept final;
    void ClearImageBuffer() const noexcept;

    void DidUpdateSizeProperties() noexcept;

    /// @brief Whether the context's pixels are shown as the canvas's layer contents.
    KRYS_NODISCARD bool UsesContentsAsLayerContents() const noexcept;

    KRYS_NODISCARD Maybe<FloatRect>
      ComputeDirtyRectangleIfNeeded(const Maybe<FloatRect> &rect) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLCanvasElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLCanvasElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::html::HTMLElement>(target);
    return element && element->IsHTMLCanvasElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::html::CanvasBase &target) noexcept
  {
    return target.IsHTMLCanvasElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
