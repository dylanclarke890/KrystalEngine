#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"

namespace krys::boo::css
{
  bool ComputedStyleDependencies::CanResolveDependenciesWithConversionData(const ToLengthConversionData &conversionData) const noexcept
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

    if (ViewportDimensions && !conversionData.RenderView())
    {
      return false;
    }

    return true;
  }
}