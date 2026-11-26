/*
 * Copyright (C) 2004-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2010 Torch Mobile (Beijing) Co. Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Web/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Web/Html/Canvas/CanvasBase.hpp"
#include "Krystal.Web/Html/Canvas/CanvasRenderingContext.hpp"
#include "Krystal.Web/Html/Canvas/CanvasRenderingContext2DSettings.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web::html
{
  class CanvasRenderingContext2D;
  class EngineTextureRenderingContext;
}

namespace krys::web
{
  class GraphicsContext;

  /// @brief A `<canvas>`: a replaced element whose pixels a rendering context draws. Its children are
  /// fallback content, which isn't rendered.
  /// @see https://html.spec.whatwg.org/multipage/canvas.html#htmlcanvaselement
  class HTMLCanvasElement : public HTMLElement, public html::CanvasBase
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLCanvasElement);

  private:
    bool _isSnapshotting {false};
    mutable bool _didClearImageBuffer {false};
    UniquePtr<html::CanvasRenderingContext> _context;
    mutable RefPtr<Image> _copiedImage;

  public:
    // These values come from the HTML standard.
    constexpr static uint32 DefaultWidth = 300;
    constexpr static uint32 DefaultHeight = 150;

    HTMLCanvasElement(Document &document) noexcept;

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

    KRYS_NODISCARD html::CanvasRenderingContext *RenderingContext() const noexcept final
    {
      return _context.get();
    }

#pragma region HTMLCanvasElement - https://html.spec.whatwg.org/multipage/canvas.html#htmlcanvaselement

    /// @see https://html.spec.whatwg.org/multipage/canvas.html#dom-canvas-width
    KRYS_NODISCARD uint32 Width() const noexcept final;

    /// @see https://html.spec.whatwg.org/multipage/canvas.html#dom-canvas-width
    void Width(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/canvas.html#dom-canvas-height
    KRYS_NODISCARD uint32 Height() const noexcept final;

    /// @see https://html.spec.whatwg.org/multipage/canvas.html#dom-canvas-height
    void Height(uint32 value) noexcept;

    /// @brief The canvas's context of type @p contextId, made on the first call; null if the canvas already
    /// has a context of another type, or there's no such type.
    /// @see https://html.spec.whatwg.org/multipage/canvas.html#dom-canvas-getcontext
    /// @note The engine texture context is `"krystal-texture"`.
    KRYS_NODISCARD RefPtr<html::CanvasRenderingContext> GetContext(DOMStringView contextId) noexcept;

    /// @brief The canvas's 2D context, made with @p settings on the first call; null if the canvas has a
    /// context of another type.
    /// @see https://html.spec.whatwg.org/multipage/canvas.html#dom-canvas-getcontext
    KRYS_NODISCARD RefPtr<html::CanvasRenderingContext2D>
      GetContext2d(html::CanvasRenderingContext2DSettings &&settings = {}) noexcept;

    KRYS_NODISCARD static bool Is2dType(DOMStringView type) noexcept;

    /// @brief The canvas's pixels as a `data:` URL of an image of @p mimeType (PNG, JPEG or WebP; PNG for any
    /// other), at @p quality (0 to 1) for JPEG and WebP; `data:,` if the canvas has no pixels.
    /// @see https://html.spec.whatwg.org/multipage/canvas.html#dom-canvas-todataurl
    KRYS_NODISCARD DOMString ToDataUrl(DOMStringView mimeType = {}, Maybe<double> quality = {}) noexcept;
    // transferControlToOffscreen() isn't planned.

#pragma endregion

    void DidDraw(const Maybe<RectF> &rect) noexcept final;

    KRYS_NODISCARD const css::ParserContext &CssParserContext() const noexcept final;

    KRYS_NODISCARD Document *CanvasDocument() const noexcept final;

    /// @brief Whether the context has work to do before its pixels are composited.
    KRYS_NODISCARD bool NeedsPreparationForDisplay() const noexcept;

    void PrepareForDisplay() noexcept;

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
    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

    void OnAdopt(Document &oldDocument) noexcept override;

    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept override;

    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept override;

  private:
    KRYS_NODISCARD html::CanvasRenderingContext2D *
      CreateContext2d(html::CanvasRenderingContext2DSettings &&settings) noexcept;

    KRYS_NODISCARD html::EngineTextureRenderingContext *CreateContextEngineTexture() noexcept;

    void CreateImageBuffer() const noexcept final;

    void ClearImageBuffer() const noexcept;

    KRYS_NODISCARD Image *CopiedImage() const noexcept final;

    void ClearCopiedImage() const noexcept final;

    void DidUpdateSizeProperties() noexcept;

    /// @brief Whether the context's pixels are shown as the canvas's layer contents.
    KRYS_NODISCARD bool UsesContentsAsLayerContents() const noexcept;

    KRYS_NODISCARD Maybe<RectF> ComputeDirtyRectangleIfNeeded(const Maybe<RectF> &rect) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLCanvasElement)
  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLCanvasElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::web::HTMLElement>(target);
    return element && element->IsHTMLCanvasElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::web::html::CanvasBase &target) noexcept
  {
    return target.IsHTMLCanvasElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
