#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class RenderElement;
  class RenderStyle;

  namespace dom
  {
    class Document;
  }
}

namespace krys::boo::style::interpolation
{
  /// @brief What interpolation needs from the animation it's for: the document, the renderer and the style
  /// being animated.
  class Client
  {
  public:
    virtual ~Client() noexcept = default;

    KRYS_NODISCARD virtual dom::Document *Document() const noexcept = 0;
    KRYS_NODISCARD virtual RenderElement *Renderer() const noexcept = 0;
    KRYS_NODISCARD virtual const RenderStyle &CurrentStyle() const noexcept = 0;
    KRYS_NODISCARD virtual Maybe<uint32> TransformFunctionListPrefix() const noexcept = 0;
  };
}
