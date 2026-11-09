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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class FloatRect;
}

namespace krys::boo::html
{
  class CanvasRenderingContext2DBase;

  /// @brief Applies a 2D context's filter to what's drawn while it's in place: the context's state is saved,
  /// and its drawing goes into a filtered layer until the state is restored.
  class CanvasFilterContextSwitcher
  {
    KRYS_NON_COPY_MOVABLE(CanvasFilterContextSwitcher);

  private:
    CanvasRenderingContext2DBase &_context;

  public:
    /// @brief A switcher for drawing within @p bounds; null if the context has no filter, or it can't be
    /// applied.
    KRYS_NODISCARD static UniquePtr<CanvasFilterContextSwitcher> Create(CanvasRenderingContext2DBase &,
                                                                        const FloatRect &bounds) noexcept;

    explicit CanvasFilterContextSwitcher(CanvasRenderingContext2DBase &) noexcept;
    ~CanvasFilterContextSwitcher() noexcept;

    /// @brief The bounds drawn within, with the filter's outsets.
    KRYS_NODISCARD FloatRect ExpandedBounds() const noexcept;
  };
}
