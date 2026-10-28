#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class GraphicsContextStateSaver;
  class ImageBuffer;
}

namespace krys::boo::css
{
  struct ParserContext;
}

namespace krys::boo::html
{
  class CanvasRenderingContext;

  /// @brief What a canvas is to its rendering context: its size, the image buffer a 2D context draws into,
  /// and where its drawing is reported.
  // FIXME: Readbacks come with the GPU bitmap (B14b) and pixel buffers (B14c).
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

    /// @brief The parser context colours given to the context are parsed in.
    KRYS_NODISCARD virtual const css::ParserContext &CssParserContext() const noexcept = 0;

    /// @brief Returns the buffer's graphics context to the state it was made with.
    void ResetGraphicsContextState() const noexcept;

    /// @brief Called when the context has drawn into @p rect of the canvas.
    /// @param rect The area drawn, in the canvas's pixels, or nothing if the caller knows the whole canvas
    /// was invalidated already.
    virtual void DidDraw(const Maybe<FloatRect> &rect) noexcept;

  protected:
    explicit CanvasBase(IntSize size) noexcept;

    void SetSize(const IntSize &size) noexcept;

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
