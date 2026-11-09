/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/ImageBufferBackend.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  /// @brief An image buffer backend that records what's drawn into a display list, for a drawing backend to
  /// render when it draws the buffer.
  /// @note Krystal.UI renders the recorded drawing into a texture each time it draws the buffer.
  class ImageBufferDisplayListBackend final : public ImageBufferBackend
  {
  private:
    display_list::RecorderImpl _drawingContext;

  public:
    static constexpr RenderingMode BackendRenderingMode = RenderingMode::DisplayList;

    KRYS_NODISCARD static UniquePtr<ImageBufferDisplayListBackend> Create(const Parameters &) noexcept;

    KRYS_NODISCARD GraphicsContext &Context() noexcept override;

    /// @brief What's been drawn into the buffer so far.
    KRYS_NODISCARD Ref<const display_list::DisplayList> CopyDisplayList() noexcept;

    explicit ImageBufferDisplayListBackend(const Parameters &) noexcept;
  };
}
