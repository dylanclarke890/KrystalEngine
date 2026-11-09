/*
 * Copyright (C) 2009-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext.hpp"
#include "Krystal.Booey/Graphics/Color/DestinationColorSpace.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/Images/NativeImage.hpp"
#include "Krystal.Booey/Graphics/Layers/GraphicsLayer.hpp"
#include "Krystal.Booey/Graphics/Layers/GraphicsLayerContentsDisplayDelegate.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2DBase.hpp"

namespace krys::boo::html
{
  CanvasRenderingContext::CanvasRenderingContext(CanvasBase &canvas, Type type) noexcept
      : _canvas(canvas), _type(type)
  {
  }

  bool CanvasRenderingContext::DelegatesDisplay() const noexcept
  {
    if (auto *context2D = DynamicDowncast<CanvasRenderingContext2DBase>(*this))
    {
      return context2D->IsAccelerated();
    }
    return IsGPUBased();
  }

  RefPtr<ImageBuffer> CanvasRenderingContext::SurfaceBufferToImageBuffer(SurfaceBuffer) noexcept
  {
    // This will be removed once all contexts store their own buffers.
    return ShareRefPtr(_canvas.Buffer());
  }

  RefPtr<NativeImage> CanvasRenderingContext::SurfaceBufferToNativeImage(SurfaceBuffer) noexcept
  {
    return nullptr;
  }

  bool CanvasRenderingContext::IsSurfaceBufferTransparentBlack(SurfaceBuffer) const noexcept
  {
    return false;
  }

  RefPtr<GraphicsLayerContentsDisplayDelegate> CanvasRenderingContext::LayerContentsDisplayDelegate() noexcept
  {
    return nullptr;
  }

  void CanvasRenderingContext::SetContentsToLayer(GraphicsLayer &layer) noexcept
  {
    layer.SetContentsDisplayDelegate(LayerContentsDisplayDelegate(),
                                     GraphicsLayer::ContentsLayerPurpose::Canvas);
  }

  DestinationColorSpace CanvasRenderingContext::ColorSpace() const noexcept
  {
    return DestinationColorSpace::SRGB();
  }

  bool CanvasRenderingContext::WillReadFrequently() const noexcept
  {
    return false;
  }
}
