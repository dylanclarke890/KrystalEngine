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

#include "Krystal.Booey/Graphics/Layers/ContentsFormat.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief How a pixel's channels are laid out in memory, 8 bits each.
  // FIXME: RGB10, RGB10A8 and RGBA16F, with wide gamut and HDR backings.
  enum class PixelFormat : uint8
  {
    RGBA8,
    BGRX8,
    BGRA8,
  };

  enum class UseLosslessCompression : bool
  {
    No,
    Yes
  };

  KRYS_NODISCARD constexpr ContentsFormat ConvertToContentsFormat(PixelFormat format) noexcept
  {
    switch (format)
    {
      case PixelFormat::RGBA8:
      case PixelFormat::BGRX8:
      case PixelFormat::BGRA8: return ContentsFormat::RGBA8;
    }
    return ContentsFormat::RGBA8;
  }

  KRYS_NODISCARD constexpr bool PixelFormatIsOpaque(PixelFormat format) noexcept
  {
    switch (format)
    {
      case PixelFormat::BGRX8: return true;
      case PixelFormat::RGBA8:
      case PixelFormat::BGRA8: return false;
    }
    return false;
  }

  TextStream &operator<<(TextStream &, PixelFormat) noexcept;
}
