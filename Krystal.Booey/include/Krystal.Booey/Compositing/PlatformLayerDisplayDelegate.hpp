/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Compositing/TextureMapperPlatformLayer.hpp"
#include "Krystal.Web/Graphics/Layers/GraphicsLayerContentsDisplayDelegate.hpp"

namespace krys::boo
{
  /// @brief A display delegate that shows a platform layer as a layer's contents.
  /// @note Every delegate Krystal.Booey's layers are given is one of these: Krystal.Booey makes them all.
  class PlatformLayerDisplayDelegate : public web::GraphicsLayerContentsDisplayDelegate
  {
  private:
    TextureMapperPlatformLayer *_platformLayer;

  public:
    KRYS_NODISCARD static Ref<PlatformLayerDisplayDelegate>
      Create(TextureMapperPlatformLayer *platformLayer) noexcept
    {
      return AdoptRef(*new PlatformLayerDisplayDelegate(platformLayer));
    }

    KRYS_NODISCARD TextureMapperPlatformLayer *PlatformLayer() const noexcept
    {
      return _platformLayer;
    }

  protected:
    explicit PlatformLayerDisplayDelegate(TextureMapperPlatformLayer *platformLayer) noexcept
        : _platformLayer(platformLayer)
    {
    }
  };
}
