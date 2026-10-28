#pragma once

#include "Krystal.Booey/HTML/CanvasBase.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::html
{
  /// @brief A canvas's rendering context, which draws its pixels. Owned by its canvas; references to it keep
  /// the canvas alive.
  // FIXME: The layer contents delegate comes with compositing canvases.
  class CanvasRenderingContext
  {
    KRYS_NON_COPY_MOVABLE(CanvasRenderingContext);

  protected:
    enum class Type : uint8
    {
      CanvasElement2D,
      EngineTexture,
    };

  private:
    CanvasBase &_canvas;
    const Type _type;

  public:
    virtual ~CanvasRenderingContext() noexcept = default;

    void AddRef() const noexcept
    {
      _canvas.AddRef();
    }
    void SubRef() const noexcept
    {
      _canvas.SubRef();
    }

    KRYS_NODISCARD CanvasBase &GetCanvasBase() const noexcept
    {
      return _canvas;
    }

    KRYS_NODISCARD bool Is2d() const noexcept
    {
      return _type == Type::CanvasElement2D;
    }
    KRYS_NODISCARD bool IsEngineTexture() const noexcept
    {
      return _type == Type::EngineTexture;
    }
    /// @brief Whether the context draws on the GPU, into a surface the compositor shows as is.
    KRYS_NODISCARD bool IsGPUBased() const noexcept
    {
      return IsEngineTexture();
    }

    /// @brief Whether the context's pixels are shown as its canvas's layer contents rather than painted.
    KRYS_NODISCARD bool DelegatesDisplay() const noexcept;

  protected:
    CanvasRenderingContext(CanvasBase &canvas, Type type) noexcept;
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_CANVAS_RENDERING_CONTEXT(ToValueTypeName, predicate)                     \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(ToValueTypeName)                                                    \
    KRYS_NODISCARD static bool IsType(const krys::boo::html::CanvasRenderingContext &context) noexcept       \
    {                                                                                                        \
      return context.predicate;                                                                              \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
