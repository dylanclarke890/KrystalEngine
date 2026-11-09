/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class GraphicsContextStateSaver;
  class Image;
  class ImageBuffer;
}

namespace krys::boo::css
{
  struct ParserContext;
}

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::html
{
  class CanvasRenderingContext;

  /// @brief What a canvas is to its rendering context: its size, the image buffer a 2D context draws into,
  /// and where its drawing is reported.
  class CanvasBase : public AbstractRefCounted
  {
  private:
    mutable IntSize _size;
    mutable RefPtr<ImageBuffer> _imageBuffer;
    mutable UniquePtr<GraphicsContextStateSaver> _contextStateSaver;
    // Whether the buffer was asked for, not whether it was made.
    mutable bool _hasCreatedImageBuffer {false};

  public:
    ~CanvasBase() noexcept override;

    KRYS_NODISCARD virtual bool IsHTMLCanvasElement() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual uint32 Width() const noexcept
    {
      return static_cast<uint32>(_size.Width());
    }
    KRYS_NODISCARD virtual uint32 Height() const noexcept
    {
      return static_cast<uint32>(_size.Height());
    }
    KRYS_NODISCARD const IntSize &Size() const noexcept
    {
      return _size;
    }

    KRYS_NODISCARD virtual CanvasRenderingContext *RenderingContext() const noexcept = 0;

    /// @brief The image buffer a 2D context draws into, made on the first call; null if the canvas is empty
    /// or too large.
    KRYS_NODISCARD ImageBuffer *Buffer() const noexcept;
    KRYS_NODISCARD bool HasCreatedImageBuffer() const noexcept
    {
      return _hasCreatedImageBuffer;
    }

    /// @brief The buffer holding what the context has drawn, ready to be read or drawn from.
    KRYS_NODISCARD RefPtr<ImageBuffer> MakeRenderingResultsAvailable() noexcept;

    /// @brief An image of what's drawn, copied when it's first asked for after a draw.
    KRYS_NODISCARD virtual Image *CopiedImage() const noexcept = 0;
    virtual void ClearCopiedImage() const noexcept = 0;

    /// @brief The parser context colours given to the context are parsed in.
    KRYS_NODISCARD virtual const css::ParserContext &CssParserContext() const noexcept = 0;
    /// @brief The document the canvas belongs to (its script execution context), if any.
    KRYS_NODISCARD virtual dom::Document *CanvasDocument() const noexcept = 0;

    /// @brief Returns the buffer's graphics context to the state it was made with.
    void ResetGraphicsContextState() const noexcept;

    /// @brief Called when the context has drawn into @p rect of the canvas.
    /// @param rect The area drawn, in the canvas's pixels, or nothing if the caller knows the whole canvas
    /// was invalidated already.
    virtual void DidDraw(const Maybe<FloatRect> &rect) noexcept;

  protected:
    explicit CanvasBase(IntSize size) noexcept;

    void SetSize(const IntSize &size) noexcept;

    /// @brief Whether a buffer of @p area pixels is accelerated.
    KRYS_NODISCARD bool ShouldAccelerate(uint64 area) const noexcept;

    void AddCanvasNeedingPreparationForDisplayOrFlush() noexcept;
    void RemoveCanvasNeedingPreparationForDisplayOrFlush() noexcept;

    /// @brief A new buffer of the canvas's size for its context, or null if the canvas is empty or too large.
    KRYS_NODISCARD RefPtr<ImageBuffer> AllocateImageBuffer() const noexcept;

    void SetHasCreatedImageBuffer(bool hasCreatedImageBuffer) const noexcept
    {
      _hasCreatedImageBuffer = hasCreatedImageBuffer;
    }

    /// @brief Replaces the buffer (the canvas takes its size) and returns the old one.
    RefPtr<ImageBuffer> SetImageBuffer(RefPtr<ImageBuffer> &&) const noexcept;

  private:
    virtual void CreateImageBuffer() const noexcept
    {
    }
  };
}
