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

#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasBase.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2DSettings.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
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
    mutable RefPtr<Image> _copiedImage;

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

    /// @brief The canvas's pixels as a `data:` URL of an image of @p mimeType (PNG, JPEG or WebP; PNG for any
    /// other), at @p quality (0 to 1) for JPEG and WebP; `data:,` if the canvas has no pixels.
    /// @see https://html.spec.whatwg.org/#dom-canvas-todataurl
    KRYS_NODISCARD dom::DOMString ToDataURL(dom::DOMStringView mimeType = {},
                                            Maybe<double> quality = {}) noexcept;
    // transferControlToOffscreen() isn't planned.

#pragma endregion

    void DidDraw(const Maybe<FloatRect> &rect) noexcept final;

    KRYS_NODISCARD const css::ParserContext &CssParserContext() const noexcept final;
    KRYS_NODISCARD dom::Document *CanvasDocument() const noexcept final;

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
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;
    void OnAdopt(dom::Document &oldDocument) noexcept override;
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

    KRYS_NODISCARD Image *CopiedImage() const noexcept final;
    void ClearCopiedImage() const noexcept final;

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
