#include "Krystal.Booey/CSS/Values/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/ToLengthConversionData.hpp"

namespace krys::boo::css
{
  bool ComputedStyleDependencies::CanResolveDependenciesWithConversionData(
    const ToLengthConversionData &conversionData) const noexcept
  {
    if (!RootProperties.empty() && !conversionData.RootStyle())
    {
      return false;
    }

    if (!Properties.empty() && !conversionData.Style())
    {
      return false;
    }

    if (ContainerDimensions && !conversionData.ElementForContainerUnitResolution())
    {
      return false;
    }

    if (ViewportDimensions && !conversionData.GetRenderView())
    {
      return false;
    }

    return true;
  }
}