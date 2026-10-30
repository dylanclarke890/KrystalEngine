#pragma once

#include "Krystal.Booey/CSS/DOMMatrix2DInit.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasPath.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  /// @brief A path that can be built once and filled, stroked or clipped to by 2D contexts.
  /// @see https://html.spec.whatwg.org/#path2d
  class Path2D final : public RefCounted<Path2D>, public CanvasPath
  {
  public:
    KRYS_NODISCARD static Ref<Path2D> Create() noexcept
    {
      return AdoptRef(*new Path2D);
    }
    KRYS_NODISCARD static Ref<Path2D> Create(const Path &path) noexcept
    {
      return AdoptRef(*new Path2D(path));
    }
    KRYS_NODISCARD static Ref<Path2D> Create(const Path2D &path) noexcept
    {
      return Create(path.GetPath());
    }
    /// @brief A path from SVG path data (`<path d>`'s syntax).
    KRYS_NODISCARD static Ref<Path2D> Create(dom::DOMStringView pathData) noexcept;

    ~Path2D() noexcept override = default;

    /// @see https://html.spec.whatwg.org/#dom-path2d-addpath
    dom::ExceptionOr<void> AddPath(const Path2D &path, css::DOMMatrix2DInit &&transform = {}) noexcept;

    KRYS_NODISCARD const Path &GetPath() const noexcept
    {
      return _path;
    }

  private:
    Path2D() noexcept = default;
    explicit Path2D(const Path &path) noexcept : CanvasPath(path)
    {
    }
  };
}
