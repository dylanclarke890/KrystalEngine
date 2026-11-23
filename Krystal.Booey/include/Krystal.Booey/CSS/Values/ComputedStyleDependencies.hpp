#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::css
{
  class ToLengthConversionData;

  enum class PropertyId : uint16;

  /// @brief What a value needs, beyond itself, to be computed: the element's or the root's properties (such
  /// as `font-size` for `em` or `rem`), and the sizes of its container, the viewport or its anchors.
  /// @see https://drafts.css-houdini.org/css-properties-values-api/#dependency-cycles
  struct ComputedStyleDependencies
  {
    /// @brief The element's own properties, each once.
    List<PropertyId> Properties;
    /// @brief The root element's properties, each once.
    List<PropertyId> RootProperties;
    /// @brief Whether it uses container query units.
    bool ContainerDimensions {false};
    /// @brief Whether it uses viewport units.
    bool ViewportDimensions {false};
    /// @brief Whether it uses `anchor()` or `anchor-size()`.
    bool Anchors {false};

    /// @brief Whether the value computes the same on every element. The viewport's size doesn't count
    /// against this, since no style can change it.
    /// @see https://drafts.css-houdini.org/css-properties-values-api/#computationally-independent
    KRYS_NODISCARD bool IsComputationallyIndependent() const noexcept
    {
      return Properties.empty() && RootProperties.empty() && !ContainerDimensions && !Anchors;
    }

    /// @brief Whether @p conversionData has what these need: the style, the root style, the container
    /// element and the render view. Anchors are resolved separately and aren't checked.
    KRYS_NODISCARD bool
      CanResolveDependenciesWithConversionData(const ToLengthConversionData &conversionData) const noexcept;
  };
}
