#pragma once

#include "Krystal.Core/Attributes.hpp"
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

    KRYS_NODISCARD bool IsComputationallyIndependent() const noexcept
    {
      return Properties.empty() && RootProperties.empty() && !ContainerDimensions && !Anchors;
    }

    KRYS_NODISCARD bool CanResolveWith(const ToLengthConversionData &conversionData) const noexcept;
  };
}
