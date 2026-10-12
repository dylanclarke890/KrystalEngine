#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::css
{
  class ToLengthConversionData;

  enum class PropertyId : uint16;

  struct ComputedStyleDependencies
  {
    List<PropertyId> Properties;
    List<PropertyId> RootProperties;
    bool ContainerDimensions {false};
    bool ViewportDimensions {false};
    bool Anchors {false};

    KRYS_NODISCARD bool IsComputationallyIndependent() const noexcept
    {
      return Properties.empty() && RootProperties.empty() && !ContainerDimensions && !Anchors;
    }

    /// @brief Checks to see if the provided conversion data is sufficient to resolve the provided dependencies.
    KRYS_NODISCARD bool CanResolveDependenciesWithConversionData(const ToLengthConversionData &conversionData) const noexcept;
  };
}
