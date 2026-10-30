#pragma once

#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResources.hpp"

namespace krys::boo
{
  /// @brief Finds and breaks the cycles between a renderer's resources and what they in turn use.
  class SVGResourcesCycleSolver
  {
    KRYS_NON_COPY_MOVABLE(SVGResourcesCycleSolver);

  public:
    static void ResolveCycles(RenderElement &, SVGResources &) noexcept;

  private:
    SVGResourcesCycleSolver() noexcept = default;

    KRYS_NODISCARD static bool ResourceContainsCycles(LegacyRenderSVGResourceContainer &,
                                                      SVGResources::ResourceSet &activeResources,
                                                      SVGResources::ResourceSet &acyclicResources) noexcept;
    static void BreakCycle(LegacyRenderSVGResourceContainer &, SVGResources &) noexcept;
  };
}
