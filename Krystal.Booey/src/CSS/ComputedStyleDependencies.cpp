#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"

namespace krys::boo::css
{
  bool ComputedStyleDependencies::CanResolveWith(const ToLengthConversionData &conversionData) const noexcept
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