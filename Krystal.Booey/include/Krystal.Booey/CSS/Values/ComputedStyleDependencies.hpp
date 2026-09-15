#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  class ToLengthConversionData;

  enum class PropertyId : uint16;

  struct ComputedStyleDependencies
  {
    SmallList<PropertyId> Properties;
    SmallList<PropertyId> RootProperties;
    bool ContainerDimensions {false};
    bool ViewportDimensions {false};
    bool Anchors {false};

    KRYS_NODISCARD bool isComputationallyIndependent() const noexcept
    {
      return Properties.empty() && RootProperties.empty() && !ContainerDimensions && !Anchors;
    }

    /// @brief Checks to see if the provided conversion data is sufficient to resolve the provided
    /// dependencies.
    KRYS_NODISCARD bool
      CanResolveDependenciesWithConversionData(const ToLengthConversionData &) const noexcept;
  };
}