#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::svg
{
  class SVGElement;
}

namespace krys::boo
{
  class Path;

  /// @brief The path of a shape element (`<circle>`, `<ellipse>`, `<line>`, `<path>`, `<polygon>`,
  /// `<polyline>` or `<rect>`), empty for other elements.
  KRYS_NODISCARD Path PathFromGraphicsElement(const svg::SVGElement &) noexcept;
}
