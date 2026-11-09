/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief The size of an observed box, in its writing mode's axes.
  /// @see https://drafts.csswg.org/resize-observer/#resizeobserversize
  class ResizeObserverSize : public RefCounted<ResizeObserverSize>
  {
  private:
    double _inlineSize {0};
    double _blockSize {0};

    ResizeObserverSize(double inlineSize, double blockSize) noexcept
        : _inlineSize(inlineSize), _blockSize(blockSize)
    {
    }

  public:
    KRYS_NODISCARD static Ref<ResizeObserverSize> Create(double inlineSize, double blockSize) noexcept
    {
      return AdoptRef(*new ResizeObserverSize(inlineSize, blockSize));
    }

    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobserversize-inlinesize
    KRYS_NODISCARD double InlineSize() const noexcept
    {
      return _inlineSize;
    }

    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobserversize-blocksize
    KRYS_NODISCARD double BlockSize() const noexcept
    {
      return _blockSize;
    }
  };
}
