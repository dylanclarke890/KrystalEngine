/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class DestinationColorSpace;
  class FloatSize;
  class ImageBuffer;
  enum class RenderingMode : uint8;
  enum class RenderingPurpose : uint8;

  /// @brief The host's graphics: the image buffers it makes with its own backends.
  // FIXME: The display's identifier and serialised image buffers, once there are any; image buffers' pixel
  // formats come with the formats (sRGB, 8 bits a channel).
  class GraphicsClient
  {
    friend class ImageBuffer;
    KRYS_NON_COPY_MOVABLE(GraphicsClient);

  public:
    GraphicsClient() noexcept = default;
    virtual ~GraphicsClient() noexcept = default;

  private:
    // Called by passing GraphicsClient into ImageBuffer functions.
    KRYS_NODISCARD virtual RefPtr<ImageBuffer>
      CreateImageBuffer(const FloatSize &, RenderingMode, RenderingPurpose, float resolutionScale,
                        const DestinationColorSpace &) const noexcept = 0;
  };
}
