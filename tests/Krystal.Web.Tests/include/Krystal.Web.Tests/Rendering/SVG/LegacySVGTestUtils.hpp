#pragma once

#include "Krystal.Web.Tests/Dom/DOMTestUtils.hpp"
#include "Krystal.Web/Rendering/RenderObject.hpp"
#include "Krystal.Web/Rendering/SVG/Legacy/SVGResources.hpp"
#include "Krystal.Web/Rendering/SVG/Legacy/SVGResourcesCache.hpp"

namespace krys::web::tests
{
  /// @brief @p renderer as a plain render object, to reach the members its subclasses hide.
  KRYS_NODISCARD inline const RenderObject &AsObject(const RenderObject &renderer) noexcept
  {
    return renderer;
  }

  /// @brief The renderer of the element of @p document with the id @p id, which must have one and be a @p T.
  template <typename T>
  KRYS_NODISCARD T &RendererById(Document &document, DOMStringView id) noexcept
  {
    auto *renderer = ById(document, id).Renderer();
    krys_assert(renderer);
    return Downcast<T>(*renderer);
  }

  /// @brief The SVG resources cached for the renderer of the element of @p document with the id @p id.
  KRYS_NODISCARD inline SVGResources *ResourcesById(Document &document, DOMStringView id) noexcept
  {
    auto *renderer = ById(document, id).Renderer();
    krys_assert(renderer);
    return SVGResourcesCache::CachedResourcesForRenderer(*renderer);
  }
}
