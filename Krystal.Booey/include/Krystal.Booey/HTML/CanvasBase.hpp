#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::html
{
  class CanvasRenderingContext;

  /// @brief What a canvas is to its rendering context: its size, and where its drawing is reported.
  // FIXME: The image buffer, readbacks and the origin-clean flag come with the 2D context.
  class CanvasBase : public AbstractRefCounted
  {
  private:
    IntSize _size;

  public:
    ~CanvasBase() noexcept override = default;

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

    /// @brief Called when the context has drawn into @p rect of the canvas.
    /// @param rect The area drawn, in the canvas's pixels, or nothing if the caller knows the whole canvas
    /// was invalidated already.
    virtual void DidDraw(const Maybe<FloatRect> &rect) noexcept;

  protected:
    explicit CanvasBase(IntSize size) noexcept;

    void SetSize(const IntSize &size) noexcept;
  };
}
