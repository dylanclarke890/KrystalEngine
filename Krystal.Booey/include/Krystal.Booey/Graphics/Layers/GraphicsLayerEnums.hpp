/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  enum class GraphicsLayerType : uint8
  {
    Normal,
    Structural, // Supports position and transform only, and doesn't flatten (i.e. behaves like preserves3D is
                // true).
    PageTiledBacking,
    TiledBacking,
    ScrollContainer,
    ScrolledContents,
    Shape
  };

  enum class GraphicsLayerMode : uint8
  {
    PlatformLayer,
    LayerHostingContextId
  };

  enum class GraphicsLayerContentsLayerPurpose : uint8
  {
    None = 0,
    Image,
    Media,
    Canvas,
    BackgroundColor,
    Plugin,
    Model,
    HostedModel,
    Host,
  };

  enum class GraphicsLayerShouldSetNeedsDisplay : bool
  {
    DoNotSet,
    Set
  };

  enum class GraphicsLayerShouldClipToLayer : bool
  {
    DoNotClip,
    Clip
  };

  enum class GraphicsLayerCompositingCoordinatesOrientation : uint8
  {
    TopDown,
    BottomUp
  };

  enum class GraphicsLayerScalingFilter : uint8
  {
    Linear,
    Nearest,
    Trilinear
  };

  enum class GraphicsLayerCustomAppearance : bool
  {
    None,
    ScrollingShadow
  };
}
